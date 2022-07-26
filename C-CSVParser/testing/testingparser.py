import sys
import os
import subprocess


def run_process(cmd_args, stdin=None, stdout=None, cwd=None, shell=True, wait=False, communicate=False, timeout=None, error_level=None):

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


def run_c_parser( input_file_addr, output_file_addr ):
	with open(output_file_addr, 'w') as output_file:
		# we will write stdout to the file
		run_process(['testparser.out', input_file_addr], stdout=output_file, wait=True)



def main():
	# 




if __name__ == "__main__":
	sys.exit(main())