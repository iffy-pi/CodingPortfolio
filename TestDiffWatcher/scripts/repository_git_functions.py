'''
This script is used to keep track of commit history to sample repository
Based on sample commits and data
Run on my own personal system
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

import secrets
import names # pip install names
import random
from datetime import datetime as ourdt


script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
COMMIT_HISTORY=os.path.join(script_loc_dir, 'sample-repository', 'COMMIT_HISTORY.txt')
MAX_COMMITS=100
MAX_COMMITS_TO_PULL=20
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


def gen_random_commit_hash(used_hashes):
	#generate the commit hash 
	h = secrets.token_hex(nbytes=16)
	while h in used_hashes:
		h = secrets.token_hex(nbytes=16)
	return h

def gen_random_date():

	minute = random.randrange(0,60)
	hour = random.randrange(0,24)
	cur_year = int(ourdt.today().strftime('%Y'))

	year = random.randrange(2020, cur_year+1)
	month =  random.randrange(1, 13)

	day_lim = 31

	# february case
	if month == 2:
		if year%4 == 0:
			day_lim = 29
		else:
			day_lim = 28

	# 30 days in september, april, june and november
	elif month in [ 9, 4, 6, 11]:
		day_lim = 30

	day = random.randrange(0, day_lim+1)    

	date =  [ minute, hour, day, month, year ]
	for i, _ in enumerate(date):
		if date[i] < 10:
			date[i] = f'0{date[i]}'

	return f'{date[1]}:{date[0]} {date[2]}/{date[3]}/{date[4]}'


def gen_random_author():
	name = names.get_full_name()
	email = '{}.{}@company.com'.format(name.split()[0], name.split()[1])
	return name, email

def gen_random_comment():
	action = [ ("Added", "to" ), ("Removed", 'from'), ("Downgraded",'for'), ("Modified",'in'), ("Updated",'for'), ("Implemented",'in') ]
	feature = [ "Color", "Parsing", "Light Detection", "Surface Mapping", "Resizing", 'DX11', 'DX12', 'FreeSync' ]
	desc = [ "in", "for", 'to', "from" ]
	app = [ 'Converter', 'CropTool', 'Encoder', 'Decoder', 'FFMPEG']

	action_choice = random.randrange(0, len(action))
	feat_choice = random.randrange( 0, len(feature))
	app_choice = random.randrange(0, len(app))

	return f'{action[action_choice][0]} {feature[feat_choice]} {action[action_choice][1]} {app[app_choice]}'
	
def get_latest_commits(commit_count=MAX_COMMITS_TO_PULL, history_file=COMMIT_HISTORY):
	
	#generates sample commits from sample repository, made randomly
	hashes_in_history = getCommitHistory(history_file=history_file)['hashes']

	generated_hashes = []

	#generate the required amount of commit hashes
	for _ in range(commit_count):
		#each hash should not be in the set of used hashes
		# ie hashes that are already in commit history, or hashes we just generated to be added to commit history
		new_hash = gen_random_commit_hash( generated_hashes + hashes_in_history)
		# hashes are ordered from newest to oldest
		generated_hashes.insert(0, new_hash)


	#generate the hash info for each new hash
	hash_info = {}
	for h in generated_hashes:
		#add its info to the file
		author, email = gen_random_author()
		hash_info[h] = {
			'author_name' : author,
			'author_email' : email,
			'author_date' : gen_random_date(),
			'subject' : gen_random_comment()
		}

	return generated_hashes, hash_info


def set_commit_history(history_file=COMMIT_HISTORY, commit_count=None):
	print('Getting Commits...')
	if commit_count is None:
		commit_count = MAX_COMMITS_TO_PULL

	latest_hashes , latest_hash_info = get_latest_commits(commit_count=commit_count, history_file=history_file)
	print('Saving to file...')
	commitdix = {
		'hashes': latest_hashes, #list of hashes newest to oldest
		'hashinfo': latest_hash_info
	}
	writeDixToFile(commitdix, history_file)
	return 0

def add_info_to_commit_history(hashes, hash_info, history_file=COMMIT_HISTORY, verbose=False):
	history = getCommitHistory(history_file=history_file)

	#hashes to be added should not already be present in the dictionary
	if any( a_hash in history['hashes'] for a_hash in hashes):
		raise Exception('Duplicate hash detected!')

	history['hashes'] = hashes + history['hashes']

	for h in hashes:
		if verbose: print(f'New commit: {h}')
		history['hashinfo'][h] = hash_info[h]

	writeDixToFile(history, history_file)

def add_to_commit_history( history_file=COMMIT_HISTORY, commit_count=MAX_COMMITS_TO_PULL, verbose=False):
	if verbose: print('Pulling new commits...')
	latest_hashes , latest_hash_info = get_latest_commits(commit_count=commit_count, history_file=history_file)

	# since sample repository, it is designed to not have duplicate hashes
	# so all latest commits are new

	add_info_to_commit_history(latest_hashes, latest_hash_info)

	#returns the new commits and hash info used if we care about
	return latest_hashes, latest_hash_info

def gen_new_commits(count=1, padding=0, random_padding=True, history_file=COMMIT_HISTORY):
	commits = []

	hashes_to_save = []
	hash_info_to_save = []

	# padding is amount of commits in between each commit in the set

	max_random_padding = 10

	if random_padding and padding!=0:
		# if random padding, padding number is used to determine max padding
		max_random_padding = padding

	for _ in range(count):
		if random_padding:
			padding = random.randrange(0,max_random_padding+1)

		#generate the (padding+1) commits
		hashes, hash_info = get_latest_commits(commit_count=padding+1)

		#take first commit from list since that is the newest commit
		# now this commit will be padding commits newer than last logged commit
		# commits in list sorted newest to oldest
		commits.insert(0, hashes[0])

		# add the list of hashes to the hashes to save
		# list is ordered from oldest to newest set of hashes
		hashes_to_save.append(hashes)
		hash_info_to_save.append(hash_info)


	# save the generated hashes and stuff

	history = getCommitHistory(history_file=history_file)
	for hash_list, a_hash_info in zip(hashes_to_save, hash_info_to_save):
		history['hashes'] = hash_list + history['hashes']

		for h in hash_list:
			history['hashinfo'][h] = a_hash_info[h]

	writeDixToFile(history, history_file)

	return commits


def getCommitHistory(history_file=COMMIT_HISTORY):
	# we check if the file exists first before pulling
	if not os.path.exists(history_file):
		return { 'hashes': [], 'hashinfo': {}}

	return readDixFromFile(history_file)


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
	hist = getCommitHistory()
	hashlist = hist['hashes']
	hashinfo = hist['hashinfo']
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