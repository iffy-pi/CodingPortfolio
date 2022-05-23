import traceback
import os
import sys
import subprocess
import time
import names
import random
from datetime import datetime as ourdt

try:
    prev_dir = os.getcwd()
    script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
    os.chdir(script_loc_dir)
    from confluence_and_html_functions import *
    from mail import send_email_from_bot
    from test_report_generator import generate_report_set
    from post_to_confluence import add_results_to_confluence
    from populate_git_info_on_confluence import populate_git_info_on_confluence
    from repository_git_functions import add_to_commit_history
    from test_diff import diff_test_results
    os.chdir(prev_dir)
except:
    print('Could not import necessary confluence and HTML functions from confluence_and_html_functions.py')
    traceback.print_exc()
    sys.exit(2)

def get_personal_confluence_auth():
    username = 'iffysbot@gmail.com'
    passwd = keyring.get_password('iffysconfluence.atlassian.net', username)

    return username, passwd

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

def post_to_confluence(node, report_file):
    auth = get_personal_confluence_auth()
    page_title = get_confl_pagetitle(node)
    page_id = get_confl_pageid(node)
    commit = get_report_commit(report_file)

    with open(report_file, 'r') as f:
        new_html = str(f.read())

    add_results_to_confluence( new_html, 'Not Available', commit, page_id, page_title, auth )

def get_report_commit(report_file):
    report_soup = parse_html_file(report_file)
    system_info_table = report_soup.find_all(attrs={'id':'small'})[1]

    for t_row in system_info_table.find_all('tr'):
        header = t_row.find_all('th')[0]
        if header.string.lower() == 'build':
            return t_row.find_all('td')[0].string


def post_sample_results_to_conflunce(count, node, source_dir, all_clear_skies=False, all_cloudy=False, all_blood_bath=False):
    #generate the sample results at the source dir,
    print("Generating reports...")
    subprocess.getoutput('rd /q /s "{}"'.format(source_dir))
    os.makedirs(source_dir, exist_ok=True)
    generate_report_set(count, source_dir, all_clear_skies=all_clear_skies, all_cloudy=all_cloudy, all_blood_bath=all_blood_bath)

    for i in range(count):
        report_file = os.path.join( source_dir, 'Report_{}.html'.format(i+1))
        print(f'Posting Report {i+1} to confluence==================>')
        post_to_confluence(node, report_file)
        time.sleep(3)


def testing():
    #read all hashes in used hashes
    #print(add_to_commit_history(commit_count=1))
 
    #populate_git_info_on_confluence(get_personal_confluence_auth(), 'Win10_Navi21', all_commits=True)

    #diff_test_results(get_personal_confluence_auth(), 'Win10_Navi21', testing=True, save_html='Win10_Navi21_Tdiff.html')

    text = 'hello world'
    fro = 'farm@company.com'

    send_email_from_bot( text, 'Hello World', 'iffysbot@gmail.com', [], verbose=True )
    
def main():
    testing()

    #post_sample_results_to_conflunce(65, 'Win10_Navi21', 'C:\\Users\\omnic\\local\\GitRepos\\AMDScripts\\personal_confluence_and_email_scripts\\samples' )

    #report = 'C:\\Users\\omnic\\local\\GitRepos\\AMDScripts\\personal_confluence_and_email_scripts\\samples\\Report_1_de00317b62b5d991e0201ac895206e77.html'
    #post_to_confluence('Win10_Navi21', report)
    #post_to_confluence()

if __name__ == "__main__":
    sys.exit(main())


