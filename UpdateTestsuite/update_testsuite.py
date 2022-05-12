import os
import platform
import sys
import ast
import re
import hashlib
import argparse
import subprocess
import traceback
import time
from shutil import copyfile

# Script is used for jenkins syncing 
# See https://confluence.amd.com/display/IMMER/Jenkins+AMF+Machine+Management+Jobs#JenkinsAMFMachineManagementJobs-TestsuiteUpdate


"""
These are the directories on the repository to sync on the test node
Directories are listed out in their parsed version so they can be concatenated on each OS appropriately
"""
SYNC_DIRS = [
	['4test','tests-amf'],
	['python']
]


"""
Mention files, directories and extensions that should be ignored in the sync
Paths to files and directories are written in the parsed path format
For directories to ignore, all files under that directory will be ignored.
"""
IGNORE = {
	#for extensions ommit the dot, so pyc refers to all .pyc files
	'extensions': [ 'pyc' ],
	'files': [],
	'directories': [
		['4test', 'always-on-testnodes'],
		['4test','tests-amf','_results'],
		['4test','tests-amf','test_suites','source'],
		['.*', '__pycache__']
	]
}

"""
AMF_JENKINS_DIR is the name of directory on the test nodes for syncing
JENKINS_DIR is the name of the Jenkins directory on the test nodes
AMF_INTERNAL_DIR is the name of the repository to sync from
"""
AMF_JENKINS_DIR='AMF-Jenkins'
JENKINS_DIR='Jenkins'
AMF_INTERNAL_DIR='AMFInternal'
NET_USED_REMOTE_ADDRS = []

# OS PLATFORM, retruns 'Windows', 'Linux' etc.
PLATFORM=platform.system()

# roots in the different OS
SYSTEM_ROOTS = {
	'Windows':'C:\\',
	'Linux':'/home/amd/'
}

# The parent directory of the local repo on the Windows and Linux servers
SERVER_ROOTS = {
	'Windows':'D:\\sharedWrite\\TestSuiteSync',
	'Linux':'/home/amd/SHARED/TestSuiteSync'
}


# The remote address to the parent directory of the local repo on the Windows and Linux servers
REMOTE_SERVER_ROOTS = {
	'Windows':'\\\\amf-fileshare-w\\sharedWrite\\TestSuiteSync',
	'Linux':'amd@amf-ubu2004-navi10:/home/amd/SHARED/TestSuiteSync'
}

# Name of log files 
SERVER_LOG_NAME = 'serverFileLog.txt'
LOCAL_LOG_NAME = 'localFileLog.txt'


"""
Roles are used to determine what the running machine should do, there are four roles
SERVER: This computer has a local copy of AMFInternal
In the script, servers update their local git repository via a git pull and generate the server file log
NODE: These nodes generate a local file log, compare it to the server file log and get/delete the relevant files
SERVERADJ: This is a machine that generates the server file log. It is necessary because the windows server does not have a processor powerful enough to generate log in due time.
SERVERNODE: This is a machine that is both a server and node. It is necessary so that files are copied locally rather than remotely.
"""
class MachineRoles():
	def __init__(self):
		self.isNode=False
		self.isServer=False
		self.isNodeAndServer=False
		self.isServerLogger=False

	def setRole(self, rolestr):
		if rolestr == 'node':
			self.isNode=True
		elif rolestr == 'server':
			self.isServer=True
		elif (rolestr == 'serveradj') or (rolestr == 'serverlogger'):
			self.isServerLogger = True
		elif rolestr == 'servernode':
			self.isNodeAndServer=True
		else:
			raise Exception('MachineRoles.setRole(): String argument \''+rolestr+'\' is not recognized')
			return 1

	def getRoleStr(self):
		if self.isServerLogger:
			return 'serverlogger'
		elif self.isNodeAndServer:
			return 'servernode'
		elif self.isServer:
			return 'server'
		elif self.isNode:
			return 'node'
		else:
			return 'unknown'


# SFTPConnection class is used to open and close sftp connections to a host
# Also handles transferring files to and from the host when the connection is opened
class SFTPConnection():
	"""
	Init Function
	Initializes the class properties
	Parameters:
		host - str - name of host
		port - int - port to connect to the host on, usually 22
		host_username - str - username of the host to connect to
		host_password - str - password of the host to connect to
		logfile - str - address of file for paramiko to log to
	"""
	def __init__(self, host, port, host_username, host_password, logfile):
		self.logfile=logfile
		self.host=host
		self.port=port
		self.username=host_username
		self.password=host_password
		self.sftp = None
		self.transport = None

	# Opens a connection to self.host, initializes the self.scp and self.transport objects
	def openConnection(self):
		import paramiko
		paramiko.util.log_to_file(self.logfile)

		# Open a transport
		self.transport = paramiko.Transport((self.host, self.port))

		# Auth    
		self.transport.connect(None, self.username, self.password)
		
		self.sftp = paramiko.SFTPClient.from_transport(self.transport)

	
	# Closes the connection (if open) to self.host
	def closeConnection( self ):
		if self.sftp: self.sftp.close()
		if self.transport: self.transport.close()

	# Get file from server, remotepath is the address of the file on the server, localpath is the address on the running machine
	def pull( self, remotepath, localpath):
		self.sftp.get(remotepath, localpath)

	# Send file to server, remotepath is file address on the host, localpath is the file address on the running machine
	def push (self, localpath, remotepath):
		self.sftp.get(localpath, remotepath)

def get_fileshare_to_linux_server():
	sftp = SFTPConnection("amf-ubu2004-navi10", 22, SERVER_USER, SERVER_PSWD, os.path.join(WORKING_DIR, 'paramikoutil.log'))
	sftp.openConnection()
	return sftp

def on_windows():
	return (PLATFORM.lower() == 'windows')

def on_linux():
	return (PLATFORM.lower() == 'linux')

def is_platform_supported():
	return (on_windows() or on_linux())

def get_platform_key():
	#returns the appropriate key for the above dictionaries
	if on_windows():
		return 'Windows'
	elif on_linux():
		return 'Linux'

def readDixFromFile(fileAddrIn):
	file=open(fileAddrIn, "r")
	dix=ast.literal_eval(file.read())
	file.close()
	return dix

def writeDixToFile(dixIn, fileAddrIn):
	with open(fileAddrIn,'w') as data: 
		data.write(str(dixIn))

def concatenate_commands(list_of_cmds):
	if platform.system().lower() == 'windows':
		concat = '&'
	elif platform.system().lower() == 'linux':
		concat = '&&'
	else:
		raise Exception('Unknown OS please add concatenation symbol')

	full_cmd_args = list_of_cmds.pop(0)

	for cur_cmd_args in list_of_cmds:
		full_cmd_args = full_cmd_args + [ concat ] + cur_cmd_args

	return full_cmd_args

def run_process(cmd_args, stdin=None, stdout=None, cwd=None, shell=True, wait=False, communicate=False, timeout=None, wait_period=5, error_level=None):

	the_cmd = None
	if shell:
		#if shell=True, command is just a string and it is passed to shell
		#if shell=False, command is list, where first argument is the program and the remaining are its arguments
		if isinstance(cmd_args, list):
			#if shell is true, we must convert list of command values to string
			cmd_str = cmd_args.pop(0)
			for c in cmd_args:
				#shells require space formatting
				if ' ' in c: c = "{}".format(c)
				cmd_str = '{} {}'.format(cmd_str, c)
			the_cmd = cmd_str
		else:
			the_cmd =  cmd_args

	if wait and (stdout is None):
		stdout = subprocess.PIPE
		#subprocess.PIPE is used if you want to read the stdout as an object
		#keep in mind you will have to decode it to a readable format
		#to do it do child.stdout.decode("UTF-8")

	child = subprocess.Popen( the_cmd, stdout=stdout, cwd=cwd, stdin=stdin, env=os.environ, shell=shell)
	return_code = None
	if communicate:
		#communcating with process, ie use stdin and stdout direct to process
		child.communicate(timeout=timeout)

	elif wait:
		#waiting for the process to for the specified amount of time
		return_code = child.wait(timeout=timeout)

	else:
		#this process is meant to run in the background so just return the object
		return child

	#terminate the process
	rc = child.returncode
	child.kill()

	if error_level:
		#check the error level of the process
		if (not rc) or ( error_level==0 and rc!=0 ) or ( error_level>0 and rc>=error_level):
			#for 0 error level, we are checking non zero exit code
			#in all other cases, raise exception if return code is greater than error level
			raise Exception('Process: "{}" failed! Return code "{}" greater than accepted: "{}"'.format(the_cmd , rc, error_level))

	#process has finished, get return code and return it
	return rc

def net_use(remoteaddr, user=None, pswd=None):
	if remoteaddr in NET_USED_REMOTE_ADDRS:
		return

	#net use command in windows, used for accessing other machines
	if not on_windows():
		raise Exception('net use being used on a non-Windows machine')

	cmd_args = ['net', 'use' , remoteaddr]

	if user != None:
		cmd_args.append('/user:{}'.format(user))
	if pswd != None:
		cmd_args.append(pswd)

	if run_process(cmd_args, wait=True) != 0:
		raise Exception('net use command failed: {}'.format(cmd_args))

	NET_USED_REMOTE_ADDRS.append(remoteaddr)

def clear_net_use():
	for remote_addr in NET_USED_REMOTE_ADDRS:
		print('Unmounting: {}'.format(remote_addr))
		if run_process(['net', 'use', remote_addr, '/delete'], wait=True) != 0:
			raise Exception('net use command failed: {}'.format(['net', 'use', remote_addr, '/delete']))

def make_full_path(parsed_path):
    #takes an array of parsed directories and combines them to with appropriate OS separators
    full_path = None
    for p in parsed_path: full_path = os.path.join( full_path, p) if full_path else p
    return full_path



def md5(fname):
	#hash function, splits the file into sections of 4096 bytes to generate hash for it
	#returns hash as a string
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def ignore_this_file(repo_dir, file_addr):
	#function returns true/false if this file should be ignored when generating the file log
	#checks file against the list of ignored extensions, files and directories

	#if file has extension
	for ext in IGNORE['extensions']:
		if re.match('.*\\.{}'.format(ext), file_addr): return True

	if os.path.splitext(file_addr) in IGNORE['extensions']: return True

	#if it is in the list of files to ignore
	for parsed_file_path in IGNORE['files']:
		if file_addr == make_full_path( [repo_dir] + parsed_file_path ): return True

	#if file is under list of ignored directories then leave it
	for parsed_dir_path in IGNORE['directories']:
		full_dir_path = make_full_path( [repo_dir] +  parsed_dir_path )
		if re.match('{}.*'.format(full_dir_path.replace('\\','\\\\')), file_addr): return True

	return False


def os_path_to_git_path( os_path ):
    upper_path , lower_path = os.path.split(os_path)
    git_path = lower_path
    while ( upper_path != '' ):
        #while there is still an upper directory
        upper_path, lower_path = os.path.split(upper_path)
        git_path = '{}/{}'.format( lower_path, git_path )

        #special windows case for drive
        if re.match( r'[A-Z]:\\', upper_path ):
            raise Exception('Cannot convert paths with drive letters!')
    return git_path

def git_path_to_os_path(git_path):
    parsed = git_path.split('/')
    full_path = parsed.pop(0)
    for p in parsed: full_path = os.path.join(full_path, p)
    return full_path

def file_addr_to_log_key(faddr, repo_dir):
    #file key is relative to repo and uses git paths
    return os_path_to_git_path ( faddr.replace( os.path.join(repo_dir, '') , '') )

def log_key_to_file_addr(key, repo_dir):
    #convert to os path from git path first and then add repo address
    return os.path.join( repo_dir, git_path_to_os_path( key ))



def generate_file_log(dir_paths, log_file_addr, use_file_hashes, verbose=False):
	#get the repo dir from one of the sync paths
	#for the passed in directory paths, each sync directory should have a matching directory path in order for us to parse repo
	# if no match is found raise exception
	for parsed_path in SYNC_DIRS:
	    f_path = make_full_path(parsed_path)
	    if all( f_path not in d for d in dir_paths):
	        #if fpath is not in any of the dir_paths then we will not be able to parse the repo directory
	        raise Exception('Cannot Parse Repo Directory!')

	#the repo directory will be the remaining path when the sync directory is removed
	#using first sync directory to get it, get the matching local path for that sync directory
	#and remove sync directory
	repo_dir = list(filter(
	            lambda path: make_full_path(SYNC_DIRS[0]) in path,
	            dir_paths))[0].replace( os.path.join ( '', make_full_path (SYNC_DIRS[0]) ), '')

	#delet old log file if it exists
	if os.path.exists(log_file_addr): os.remove(log_file_addr)

	file_dix = {}
	for path in dir_paths:
		if os.path.exists(path):
			for root, dirs, files in os.walk(path):
				for file in files:
					file_addr = os.path.join(root, file)

					if not ignore_this_file( repo_dir, file_addr):
						file_key = file_addr_to_log_key ( file_addr, repo_dir)
						if verbose: print('Processing file: {}'.format(file_key))
						#for the file key in the log
						file_dix[file_key] = md5(file_addr) if use_file_hashes else '0'

	writeDixToFile( file_dix, log_file_addr )


def get_server_log_file_addr():
	return os.path.join( SERVER_ROOT, JENKINS_DIR, SERVER_LOG_NAME )


#The job run on the machines with role as serveradj, i.e. just generating the file log
def server_logger_job():
	if not on_windows(): raise Exception('Server Logger is only designed for Windows machines!')

	local_log_for_server_addr = os.path.join( WORKING_DIR, 'local_log_for_server.txt' )
	#remote address for the log file on the server
	remote_server_log_file_addr = get_server_log_file_addr().replace(SERVER_ROOT, REMOTE_SERVER_ROOT)

	#get the remote server sync paths
	remote_server_sync_paths = []
	for path in SERVER_SYNC_PATHS:
		remote_server_sync_paths.append( path.replace(SERVER_ROOT, REMOTE_SERVER_ROOT) )

	print('Generating Log for Windows Server')
	net_use(REMOTE_SERVER_ROOT, user=SERVER_USER, pswd=SERVER_PSWD)
	generate_file_log( remote_server_sync_paths, local_log_for_server_addr, True, verbose=True)

	print('Transferring log file to server....')
	copyfile( local_log_for_server_addr, remote_server_log_file_addr  )
	print('Complete')
	return 0



#Update the local repository on the server nodes
def update_server_repo():
	try:
		import git
	except:
		raise Exception('No git module available, please install using python pip!')

	repo = git.Repo(SERVER_REPO_DIR)
	prev_latest_commit = repo.head.commit

	update_commands = concatenate_commands([ ['git', 'reset', '--hard', 'origin/amd/main'], ['git', 'pull'] ])

	#print('Updating Repository...')
	if (run_process(update_commands, cwd=SERVER_REPO_DIR, communicate=True)  != 0): raise Exception('An error occured updating the repository!')

	if prev_latest_commit != repo.head.commit:
		print('Files were changed')
	else:
		print('No changes')

#Job executed by machines with server role, i.e. pull from the repo and generate the log file
def server_job():
	print('Updating Repo: {}...'.format(SERVER_REPO_DIR))
	update_server_repo()
	
	if not UPDATE_REPO_ONLY:
		print('Generating server log...')
		generate_file_log(SERVER_SYNC_PATHS, get_server_log_file_addr(), True, verbose=True)
		print('Server Log Generated Succcessfully')
	
	return 0


def sync_dirs_with_robocopy(source, dest, verbose=True):
	if not on_windows():
		raise Exception('Command only runs for Windows systems')

	robocopy_cmd = [ 'robocopy', source, dest, '/mt:32', '/mir', '/FFT']

	print('Waiting for command to finish: {}'.format(robocopy_cmd))
	if verbose:
		return_code = run_process(robocopy_cmd, communicate=True)
	else:
		return_code = run_process(robocopy_cmd, communicate=True, stdout=subprocess.PIPE)

	#check the return code
	if (not return_code) or (return_code >= 4):
		raise Exception('robocopy sync failed! Source: {}, Dest: {}'.format(source,dest))

def sync_dirs_with_rsync(source, dest, sshpass=None, verbose=True):
	if not on_linux():
		raise Exception('Only for Linux Machines!')

	rsync_cmd = ['rsync', '-av', '{}/'.format(source), dest ]

	if sshpass:
		#this is a remote copy need sshpass
		rsync_cmd = ['sshpass', '-p', '"{}"'.format(sshpass) ] + rsync_cmd

	if verbose:
		return_code = run_process(rsync_cmd, communicate=True)
	else:
		return_code = run_process(rsync_cmd, wait=True)

	#check the return code
	if (return_code != 0):
		raise Exception('rsync failed! Source: {}, Dest: {}'.format(source,dest))


def clone_on_windows():
	if CUR_MACHINE_ROLE.isNode:
		#only nodes need netuse, server nodes do not need it
		net_use(REMOTE_SERVER_ROOT, user=SERVER_USER, pswd=SERVER_PSWD)

	elif not CUR_MACHINE_ROLE.isNodeAndServer:
		#only nodes and server nodes should be running this
		raise Exception('Inappropriate role for clone: '+CUR_MACHINE_ROLE.getRoleStr())

	for i in range(0, len(SERVER_SYNC_PATHS)):
		dest_path = LOCAL_SYNC_PATHS[i]
		source_path = SERVER_SYNC_PATHS[i]

		if not CUR_MACHINE_ROLE.isNodeAndServer:
			# if not a server node, need remote accesss
			source_path = source_path.replace(SERVER_ROOT, REMOTE_SERVER_ROOT)

		sync_dirs_with_robocopy(source_path, dest_path)

def clone_on_linux():
	if not (CUR_MACHINE_ROLE.isNode or CUR_MACHINE_ROLE.isNodeAndServer):
		raise Exception('Inappropriate role for clone: '+CUR_MACHINE_ROLE.getRoleStr())

	for i in range(0, len(SERVER_SYNC_PATHS)):
		dest_path = LOCAL_SYNC_PATHS[i]
		source_path = SERVER_SYNC_PATHS[i]

		server_pass = None
		if not CUR_MACHINE_ROLE.isNodeAndServer:
			source_path = source_path.replace(SERVER_ROOT, REMOTE_SERVER_ROOT)
			server_pass = SERVER_PSWD

		#run the sync command
		sync_dirs_with_rsync(source_path, dest_path, sshpass=server_pass)

def clone_repo():
	#first we have to make all the relevant directories
	for path in LOCAL_SYNC_PATHS:
		os.makedirs(os.path.split(path)[0], exist_ok=True)

	if on_windows():
		return clone_on_windows()
	elif on_linux():
		return clone_on_linux()


def copy_files_windows(copy_list):
	net_use(REMOTE_SERVER_ROOT, user=SERVER_USER, pswd=SERVER_PSWD)

	for pair in copy_list:
		server_addr = pair[0]
		local_addr = pair[1]
		os.makedirs(os.path.split(local_addr)[0], exist_ok=True)

		if not CUR_MACHINE_ROLE.isNodeAndServer:
			server_addr = server_addr.replace(SERVER_ROOT, REMOTE_SERVER_ROOT)

		print('Copying {}...'.format( server_addr.replace( os.path.join(REMOTE_SERVER_ROOT, AMF_INTERNAL_DIR, ''), '' ) ))

		copyfile(server_addr, local_addr)


#Same as copyFilesWindows, just minor changes for Linux
def copy_files_linux(copy_list):
	#transfers files using sftp
	sftp = None
	if not CUR_MACHINE_ROLE.isNodeAndServer:
		#this is not a node and server so we cannot copy files locally
		# we need sftp
		sftp = get_fileshare_to_linux_server()

	for pair in copy_list:
		server_addr = pair[0]
		local_addr = pair[1]
		os.makedirs(os.path.split(local_addr)[0], exist_ok=True)
		print('Copying {}...'.format( server_addr.replace( os.path.join(REMOTE_SERVER_ROOT, AMF_INTERNAL_DIR, ''), '' ) ))
		if CUR_MACHINE_ROLE.isNodeAndServer:
			#use copy file
			copyfile(server_addr, local_addr)
		else:
			#use sftp
			sftp.pull(server_addr, local_addr)

	#close the sftp connection when all files are done
	if sftp: sftp.closeConnection()


#Parent function for copying files, chooses appropriate OS function based on platform
def copy_files(copy_list):
	if on_windows():
		copy_files_windows(copy_list)
	elif on_linux():
		copy_files_linux(copy_list)


#Gets the server file log from the server node and copies it to fileAddrToWriteTo
def get_file_log_from_server(save_addr):
	#used by the nodes to retrieve the file log from server
	if os.path.exists(save_addr):
		os.remove(save_addr)

	#get address of the server log file
	remote_server_log_file = get_server_log_file_addr().replace(SERVER_ROOT, REMOTE_SERVER_ROOT)

	if on_windows():
		net_use(REMOTE_SERVER_ROOT, user=SERVER_USER, pswd=SERVER_PSWD)
		copyfile( remote_server_log_file, save_addr)

	elif on_linux():
		sftp = get_fileshare_to_linux_server()
		sftp.pull( get_server_log_file_addr(), save_addr )
		sftp.closeConnection()

"""
Job run by machines with node role,
Get the server file log, generate local file log and diff against server file log
Files in local but not in server are deleted
Files with different hashes are copied over.
"""
def node_job():
	print('Syncing: {}'.format(LOCAL_REPO_DIR))
	print('Working Directory: {}'.format(WORKING_DIR))

	server_log_file_addr = os.path.join( WORKING_DIR, SERVER_LOG_NAME )
	local_log_file_addr = os.path.join( WORKING_DIR, LOCAL_LOG_NAME )

	print('Getting Server File Log....')
	get_file_log_from_server(server_log_file_addr)

	# generate the local log and read it
	print('Generating Local File Log....')
	generate_file_log( LOCAL_SYNC_PATHS, local_log_file_addr, True)

	local_files_info = readDixFromFile( local_log_file_addr )
	server_files_info = readDixFromFile( server_log_file_addr )

	print('Performing Diff...')
	# present in local but not in server must be deleted
	files_to_delete = set(local_files_info.keys()) - set(server_files_info.keys())

	# present in server but not in local or with different hash than in local must be copied
	files_to_copy = set(server_files_info.items()) - set(local_files_info.items())

	copy_list = []
	if len(local_files_info) == 0:
		print('No files found. Cloning Repository...')
		clone_repo()

	else:
		for key in files_to_delete:
			file = log_key_to_file_addr( key , LOCAL_REPO_DIR )
			print('Deleting: '+file)
			if os.path.exists(file): os.remove(file)

		for pair in files_to_copy:
			local_addr = log_key_to_file_addr( pair[0], LOCAL_REPO_DIR)
			server_addr = local_addr.replace(LOCAL_REPO_DIR, SERVER_REPO_DIR)
			copy_list.append( [server_addr, local_addr])

		if len(copy_list) > 0:
			print('Copying Files...')
			copy_files( copy_list)

	print('\nSync Verification...')
	failed_sync = False

	for file in files_to_delete:
		#verify that the files actually deleted
		file = log_key_to_file_addr( file, LOCAL_REPO_DIR )
		if os.path.exists(file):
			print('Failed to delete: '+str(file))
			failed_sync = True

	for pair in files_to_copy:
		local_file_addr = log_key_to_file_addr( pair[0], LOCAL_REPO_DIR)
		hash_of_file_on_server = pair[1]
		disp_addr = local_file_addr.replace(LOCAL_REPO_DIR, SERVER_REPO_DIR).replace(SERVER_ROOT, REMOTE_SERVER_ROOT)
		#verify that we got the new file
		#and the new file matches whats on the server
		if not os.path.exists(local_file_addr):
			print('Failed To Copy: '+disp_addr )
			failed_sync=True
		else:
			localfilehex = md5(local_file_addr)
			if localfilehex != hash_of_file_on_server:
				print('Failed To Copy: '+disp_addr )
				failed_sync = True

	if failed_sync:
		print('Sync Failed')
		return 1
	else:
		print('Sync Successful')
		return 0


def main():
	if not is_platform_supported():
		print('Unsupported Platform!: {}'.format(PLATFORM))
		return 1

	global TESTING
	global UPDATE_REPO_ONLY
	global CUR_MACHINE_ROLE
	global SERVER_USER
	global SERVER_PSWD
	global SYSTEM_ROOT
	global SERVER_ROOT
	global REMOTE_SERVER_ROOT
	global WORKING_DIR
	global LOCAL_REPO_DIR
	global SERVER_REPO_DIR
	global SERVER_SYNC_PATHS
	global LOCAL_SYNC_PATHS

	#parse the arguments
	parser = argparse.ArgumentParser()
	parser.add_argument(
		"-role",
		required=True,
	    type = str,
	    metavar='<server, node, serveradj or servernode>',
	    help = "The role of the computer for the update"
	)
	parser.add_argument(
		"-sync_dir",
		required=False,
	    type = str,
	    metavar='<Directory Address>',
	    help = "The parent directory of 4test/tests-amf and python, which will be synced to the latest version"
	)
	parser.add_argument(
		"-working_dir",
		required=False,
	    type = str,
	    metavar='<Directory Address>',
	    help = "The working directory for the script (store temporary files at), should not be sync_dir"
	)

	parser.add_argument(
		"-servercreds",
		required=False,
	    type = str,
	    help = 'The "username password" pair used to access the servers for the nodes'
	)
	parser.add_argument('--update_repo_only', '--update_repo_only', action='store_true', help='Testing the script, disables email sending')
	parser.add_argument('--testing', '--testing', action='store_true', help='Testing the script, disables email sending')
	options = parser.parse_args()

	try:
		TESTING = options.testing
		UPDATE_REPO_ONLY = options.update_repo_only

		CUR_MACHINE_ROLE = MachineRoles()
		# get the role for this running machine and call the appropriate job
		CUR_MACHINE_ROLE.setRole(options.role)

		plaform_key = get_platform_key()

		SYSTEM_ROOT 		= SYSTEM_ROOTS[plaform_key] # the system root, C:\ or /home/amd
		SERVER_USER 		= None # root user for accessing server, populated below
		SERVER_PSWD 		= None # root password for accessing server , populated below
		SERVER_ROOT 		= SERVER_ROOTS[plaform_key] # the parent directory of the repo on the server
		REMOTE_SERVER_ROOT 	= REMOTE_SERVER_ROOTS[plaform_key] # the remote path to the parent directory of the repo on the server
		WORKING_DIR 		= WORKING_DIR = os.path.join(SERVER_ROOT, JENKINS_DIR) if CUR_MACHINE_ROLE.isServer else os.path.join(SYSTEM_ROOT, JENKINS_DIR) # the directory used to store log files
		LOCAL_REPO_DIR 		= os.path.join(SYSTEM_ROOT, AMF_JENKINS_DIR) # the local AMF-Jenkins directory on the system
		SERVER_REPO_DIR		= os.path.join(SERVER_ROOT, AMF_INTERNAL_DIR) # The full path to the repo directory (AMFInternal) on the server nodes

		SERVER_SYNC_PATHS = []
		LOCAL_SYNC_PATHS = []

		if options.sync_dir is not None:
			if not os.path.exists(options.sync_dir):
				print('Sync Directory \'{dir}\' cannot be found on machine'.format(dir=options.sync_dir))
				return 1
			LOCAL_REPO_DIR = str(options.sync_dir)

		if options.working_dir is not None:
			if not os.path.exists(options.working_dir):
				print('Working Directory: \'{dir}\' cannot be found on machine'.format(dir=options.working_dir))
				return 1
			WORKING_DIR = str(options.working_dir)

		if options.servercreds:
			SERVER_USER, SERVER_PSWD = options.servercreds.split(' ')
			SERVER_USER = SERVER_USER.strip()
			SERVER_PSWD = SERVER_PSWD.strip()

		if SERVER_USER is None or SERVER_PSWD is None:
			if not (CUR_MACHINE_ROLE.isServer or CUR_MACHINE_ROLE.isNodeAndServer):
				raise Exception('Non Server machines must provide server credentials!')

		for split_path in SYNC_DIRS:
			SERVER_SYNC_PATHS.append( make_full_path( [SERVER_REPO_DIR] + split_path ))
			LOCAL_SYNC_PATHS.append( make_full_path( [LOCAL_REPO_DIR] + split_path ))

		if CUR_MACHINE_ROLE.isNode or CUR_MACHINE_ROLE.isNodeAndServer:
			ret = node_job()

		elif CUR_MACHINE_ROLE.isServer:
			ret = server_job()

		elif CUR_MACHINE_ROLE.isServerLogger:
			ret = server_logger_job()
		else:
			raise Exception('No job found for this machine')

		clear_net_use()
		return ret
	except:
			print('main(): An exception occured')
			traceback.print_exc()
			return 1

	return 0

if __name__ == "__main__":
	sys.exit(main())