'''
This script is used to keep track of commit history to the amd/stg/amf repository
It can be executed only on the master node
It takes commits and their info (author, date and subject) and writes it to COMMIT_HISTORY
The new commits are gotten from updating the local repository on the master node C:\AMF-Jenkins-Git
UPDATE_BRANCH refers to our branch of interest, i.e. amd/stg/amf
'''


import os
import platform
import sys
import ast
import re
import argparse
import subprocess
import copy
from shutil import copyfile
import time

COMMIT_LOG='C:\\AMF-Jenkins-Git\\commit_log.txt'
COMMIT_HISTORY='C:\\AMF-Jenkins-Git\\COMMIT_HISTORY.txt'
MAX_COMMITS=100
MAX_COMMITS_TO_PULL=100
LOCAL_REPO='C:\\AMF-Jenkins-Git\\drivers'
UPDATE_BRANCH='amd/stg/amf'
PLATFORM = platform.system()
RETRIES = 3


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
		if self.logfile:
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
	#net use command in windows, used for accessing other machine
	cmd_args = ['net', 'use' , remoteaddr]

	if user != None:
		cmd_args.append('/user:{}'.format(user))
	if pswd != None:
		cmd_args.append(pswd)

	if run_process(cmd_args, wait=True) != 0:
		raise Exception('net use command failed: {}'.format(cmd_args))

def readDixFromFile(fileAddrIn):
	try:
		file=open(fileAddrIn, "r")
		dix=ast.literal_eval(file.read())
		file.close()
		return dix
	except Exception as e:
		raise Exception('readDixFromFile():\n'+str(e))

def writeDixToFile(dixIn, fileAddrIn):
	try:
		with open(fileAddrIn,'w') as data: 
			data.write(str(dixIn))
	except Exception as e:
		raise Exception('writeDixToFile():\n'+str(e))

def remote_copy_file(source, dest, host=None, host_user=None, host_pswd=None, remotesrc=False, remotedest=False):
	#first do a netuse on the source
	
	if remotesrc and remotedest:
		raise Exception('Source and destination cannot both be remote on linux')

	if PLATFORM == 'Windows':
		#assuming we have already netused
		#just use copyfile
		copyfile(source, dest)

	elif PLATFORM == 'Linux':
		if host_user is None or host_pswd is None or host is None:
			raise Exception('Cannot send file, no host info')

		if (not remotesrc) and (not remotedest):
			raise Exception('No remote source or destination')

		sftp = SFTPConnection(host, 22, host_user, host_pswd, None)
		sftp.openConnection()

		if remotesrc:
			#we are getting from remote so pull
			sftp.pull(source, dest)
		elif remotedest:
			#sending to remote
			sftp.push(source, dest)

def update_repository(local_repo=LOCAL_REPO, branch=UPDATE_BRANCH):
	prev_dir = os.getcwd()
	print('Updating repository====>')
	print(local_repo)
	fetch_re = RETRIES
	while(fetch_re > 0):
		try:
			update_cmd = [ 'git', 'fetch', 'origin', branch, '--tags', '--force', '--prune']
			#pull from the repo
			rc = run_process( update_cmd, cwd=local_repo, communicate=True )
			if rc == 0:
				#if its successful, we break the while loop
				break
		except:
			#catch the exception and subtract from the retries
			time.sleep(120)
			fetch_re -= 1

	if fetch_re == 0:
		#if we have 0 retries that means it tried and failed so raise an exception
		raise Exception('REPO UPDATE FAILED AFTER {r} RETRIES'.format(r=RETRIES))

	print("Update Complete===>")
	
def get_latest_commits(local_repo=LOCAL_REPO, branch=UPDATE_BRANCH, commit_count=MAX_COMMITS_TO_PULL, commit_log_file=COMMIT_LOG):
	update_repository(local_repo=local_repo, branch=branch)
	prev_dir = os.getcwd()
	os.chdir(local_repo)

	output = subprocess.getoutput('git log origin/{} --pretty=format:%H++++%an++++%ae++++%ad++++%s -{}'.format(branch, commit_count))
	commit_log_lines = output.split('\n')

	os.chdir(prev_dir)
	hashes = []
	hash_info = {}
	for rawline in commit_log_lines:
		#commits are in order of newest to oldest
		info = rawline.strip().split('++++')
		chash = info[0]
		author = info[1]
		email = info[2]
		date = info[3]
		subject = info[4].replace('"','*').replace('”','*')

		#append it to hashes list newest to oldest
		hashes.append(chash)

		#add its info to the file
		hash_info[chash] = {
			'author_name' : author,
			'author_email' : email,
			'author_date' : date,
			'subject' : subject
		}

	return hashes, hash_info


def set_commit_history(history_file=COMMIT_HISTORY, local_repo=LOCAL_REPO, branch=UPDATE_BRANCH, commit_count=None):
	print('Getting Commits...')
	if commit_count is None:
		commit_count = MAX_COMMITS_TO_PULL

	latest_hashes , latest_hash_info = get_latest_commits(local_repo=local_repo, branch=branch, commit_count=commit_count)
	print('Saving to file...')
	commitdix = {
		'hashes': latest_hashes, #list of hashes newest to oldest
		'hashinfo': latest_hash_info
	}
	writeDixToFile(commitdix, history_file)
	return 0

def add_to_commit_history(local_repo=LOCAL_REPO, branch=UPDATE_BRANCH, history_file=COMMIT_HISTORY):
	print('Searching latest commits...')
	#first get the current history file
	commitdix = readDixFromFile(history_file)
	#get the MAX_COMMITS number latests commits
	latest_hashes , latest_hash_info = get_latest_commits(local_repo=local_repo, branch=branch)

	newcommits=[]
	newcommitsinfo = {}
	newCommitFound = False
	reachedHistory = False
	i=0
	while (not reachedHistory) and (i < len(latest_hashes)):
		chash = latest_hashes[i]
		author = latest_hash_info[chash]['author_name']
		email = latest_hash_info[chash]['author_email']
		date = latest_hash_info[chash]['author_date']
		subject = latest_hash_info[chash]['subject']
		#commits are ordered newest to oldest
		#so check if this commit is already in dictionary
		if chash not in commitdix['hashes']:
			print('New Commit: '+chash)
			#new commit not in the history so we add it
			#commitdix['hashes'].insert(0, chash)
			#new commit list has the new commits in the order of newest to oldest
			newcommits.append(chash)

			newcommitsinfo[chash] = {
				'author_name' : author,
				'author_email' : email,
				'author_date' : date,
				'subject' : subject
			}

			newCommitFound=True
		else:
			#we already have this hash, meaning we have all the hashes after since ordered newest to oldest
			#stop processing
			reachedHistory=True
		#increment to next hash
		i = i+1

	if newCommitFound:
		#reverse the commits so we can add them appropriately in the commit dix
		newcommits.reverse()
		for a_hash in newcommits:
			#now list is sorted from oldest to newest, which means we can just insert them at the beginning of commit dix
			#and preserver order
			commitdix['hashes'].insert(0, a_hash)
			commitdix['hashinfo'][a_hash] = newcommitsinfo[a_hash]

		print('Saving to file...')
		writeDixToFile(commitdix, history_file)
	else:
		print('No new commits found')


def getCommitHistory():
	return readDixFromFile(COMMIT_HISTORY)


def get_commit_link(cl):
	return 'https://github.amd.com/AMD-Radeon-Driver/drivers/commit/{cl}'.format(cl=cl)

def findCommitsBetween(hashlist, earlier_than=None, later_than=None):
	#hash list is ordered from newest to oldest
	#print the commit info for each of the commits between these two
	if earlier_than is None and later_than is None:
		#there is no limit here, so search entire hash list
		return hashlist

	commitsInBetween = []
	start_lim = None
	end_lim = None

	if earlier_than is None: start_lim = -1 #no start limit, we start from the beginning of the list
	else: start_lim = hashlist.index(earlier_than)

	if later_than is None: end_lim = len(hashlist) # no end limit so search till end of list
	else: end_lim = hashlist.index(later_than)

	
	if start_lim is None or end_lim is None:
		raise Exception('No limits have been defined')

	if end_lim < start_lim:
		raise Exception('End Limit is before Start Limit, are your commits in the correct order?')

	# range search is not inclusive
	for i in range(start_lim+1, end_lim):
		commitsInBetween.append(hashlist[i])
	
	return commitsInBetween


def get_print_format(cl, hashinfo):
	return '{cl} || {user} || {sub}'.format(cl=cl, user=hashinfo[cl]['author_name'], sub=hashinfo[cl]['subject'])


def showCommitsBetween(earlier_than, later_than):
	add_to_commit_history()
	hist = getCommitHistory()
	hashlist = hist['hashes']
	hashinfo = hist['hashinfo']

	commitsbtwn = findCommitsBetween(hashlist, earlier_than=earlier_than, later_than=later_than)

	if commitsbtwn is not None:
		print('{l} commits'.format(l=len(commitsbtwn)))
		print('Commits in between {new} and {old}, sorted from latest to earliest:'.format(new=earlier_than, old=later_than))

		print('EARLIER THAN: {info}'.format(info=get_print_format(earlier_than, hashinfo)))

		print('====================================================================================================>')

		for c in commitsbtwn:
			print('Commit {info}'.format(info=get_print_format(c, hashinfo)))

		print('=====================================================================================================>')

		print('LATER THAN: {info}'.format(info=get_print_format(later_than, hashinfo)))


		print('Super Link: {link}'.format( link=get_commit_link('') ))
	else:
		print('Not Available, one or both commits could not be found in commit history')

def searchCommit(search_commit, earlier_than=None, later_than=None):
	hashlist = getCommitHistory()['hashes']
	hashinfo = getCommitHistory()['hashinfo']
	commitsbtwn = findCommitsBetween(hashlist, earlier_than=earlier_than, later_than=later_than)

	if commitsbtwn is None:
		print('Search range is inapplicable')
		return

	#will be here is commitsbtwn has value
	if search_commit in commitsbtwn:
		sindx = commitsbtwn.index(search_commit)
		print('Search commit {c} found in specified range'.format(c=search_commit))
		print(commitInfoToStr(search_commit, hashinfo))

		other_info = ''

		if earlier_than is not None:
			print('{n} commits earlier than "{e}"'.format(n=sindx+1, e=earlier_than))

		if later_than is not None:
			 print('{n} commits later than "{l}"'.format(n=sindx-len(commitsbtwn)+2, l=later_than))


	else:
		print('Search commit {c} was not found in range'.format(c=search_commit))
		

def commitInfoToStr(commithash, hash_history):
	fullstr = None
	if commithash in hash_history.keys():
		info = hash_history[commithash]
		return 'Commit: {}\nAuthor: {}\nEmail: {}\nComment: "{}"'.format(commithash, info['author_name'], info['author_email'], info['subject'])
	else:
		return None

def main():
	parser = argparse.ArgumentParser()

	parser.add_argument('--set', '--set', action='store_true', help='Reset the commit history')
	parser.add_argument('--update', '--update', action='store_true', help='Add latest commit to history')
	parser.add_argument('--between', '--between', action='store_true', help='Show commits in between <earlier_than> and <later_than>')
	parser.add_argument(
		"-earlier_than",
		required=False,
		type = str,
		metavar='<commit hash>',
		help = "Search range is commits earlier than this commit"
	)
	parser.add_argument(
		"-later_than",
		required=False,
		type = str,
		metavar='<commit hash>',
		help = "Search range is commits than than this commit"
	)
	parser.add_argument(
		"-commit_count",
		required=False,
		type = int,
		metavar='<number>',
		help = "The number of commits to set the commit history to"
	)
	parser.add_argument(
		"-find",
		required=False,
		type = str,
		metavar='<commit hash>',
		help = "Search for this commit a given range"
	)
	options = parser.parse_args()

	if options.set:
		set_commit_history(commit_count=options.commit_count)

	elif options.update:
		add_to_commit_history()

	elif options.between:
		if (options.earlier_than is None) and (options.later_than is None):
			print('Invalid range specified')
			return 1

		showCommitsBetween(options.earlier_than.strip(), options.later_than.strip())

	elif options.find is not None:
		searchCommit(options.find, earlier_than=options.earlier_than, later_than=options.later_than)


	return 0

if __name__ == "__main__":
	sys.exit(main())

"""
Can be wrtitten to history file in the post to confluence step, 
this scrpt will just find the earliest commit and do diff
for now we are in testing
"""