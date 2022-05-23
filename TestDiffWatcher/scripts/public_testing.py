import traceback
import os
import sys
import random
import shutil
import time

script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
if script_loc_dir not in sys.path:  sys.path.append(script_loc_dir)
from confluence_and_html_functions import *
from test_report_generator import generate_report_set, gen_random_results_report
from post_to_confluence import add_results_to_confluence, get_report_commit
from populate_git_info_on_confluence import populate_git_info_on_confluence
from mail import EMAIL_KEYRING,get_email_creds,set_email_creds

def console_select_options(options, descriptions=None, prompt=None, select_multiple=False):

    if not descriptions:
        descriptions = options

    options_str = f'1: {descriptions[0]}\n'

    for i in range(1, len(descriptions)):
        options_str += f'{i+1}: {descriptions[i]}\n'

    print('')
    if prompt:
        print(prompt)

    print(options_str)

    desc = 'Type the number of your selected choice'
    if select_multiple: desc += ' (multiple choices are space separated)'
    desc += ':'


    while True:
        try:
            chosen_indx = input(f'{desc} ')

            chosen_options = ()

            for j in chosen_indx.split():
                chosen_options = chosen_options + (options[int(j)-1],)

            break
        except ValueError:
            print('Inappropriate values entered, please try again')

    if select_multiple:
        return chosen_options
    else:
        return chosen_options[0]



def view_confluence_pages():
    selected_nodes = console_select_options(list(TESTNODE_CONFLUENCE_PAGES.keys()), prompt='Select the node to view', select_multiple=True)

    print('')
    for node in selected_nodes:
        print('Node: {}\nConfluence: {}'.format(node, get_confl_page_link(node)))

def gen_sample_results_dialog():
    # select the node
    node = console_select_options(list(TESTNODE_CONFLUENCE_PAGES.keys()), prompt='Make results for node')

    # select how many
    print('')
    count = int(input('How many samples should be generated: '))

    # select the type
    clear_skies_opt = 0
    cloudy_opt = 1
    std_opt = 2
    blood_bath_opt = 3
    randomized_opt = 4

    clear_skies_desc = 'Clear Skies: Generated report(s) have no failures'
    cloudy_desc      = 'Cloudy:      Generated report(s) have little to no failures'
    std_desc         = 'Standard:    Generated report(s) have random selection of failures'
    blood_bath_desc  = 'Blood Bath:  Generated report(s) have several failures'
    randomized_desc  = 'Randomized:  Generated reports are random assortment of the above types'

    opts = ( clear_skies_opt, cloudy_opt, std_opt, blood_bath_opt, randomized_opt )
    descs = ( clear_skies_desc, cloudy_desc, std_desc, blood_bath_desc, randomized_desc )

    selected_opt = console_select_options(opts, descriptions=descs, prompt='Select type of geenrated reports' )


    # save to file
    def_dir = os.path.join ( os.getcwd(), 'sample_results' )

    i = 0
    while os.path.exists( '{}{}'.format(def_dir, f'_{i}' if i!=0 else '') ):
        i += 1

    def_dir =  '{}{}'.format(def_dir, f'_{i}' if i!=0 else '')

    def_file = os.path.join(os.getcwd(), 'Sample_Report')
    i = 0
    while os.path.exists('{}{}.html'.format(def_file, f'_{i}' if i!=0 else '')):
        i += 1

    def_file = '{}{}.html'.format(def_file, f'_{i}' if i!=0 else '')

    dir_desc = f'Input directory (will be EMPTIED) to save generated reports [{def_dir}]'
    file_desc = f'Input file name to save the generated report [{def_file}]'
    default_desc = dir_desc if count>1 else file_desc
    default = def_dir if count>1 else def_file
    
    print('')
    save_to = input(f'{default_desc}: ')

    if save_to == '':
        save_to = default


    # randomized is not available for one report, so just pick one of the report types at random
    if count == 1 and selected_opt == randomized_opt:
        selected_opt = random.randrange(0, 4)

    clear_skies = ( selected_opt == clear_skies_opt )
    cloudy = ( selected_opt == cloudy_opt )
    std = ( selected_opt == std_opt )
    blood_bath = ( selected_opt == blood_bath_opt )


    print('')

    if count > 1:
        if os.path.exists(save_to):
            print(f'Emptying {save_to}\n....') 
            shutil.rmtree(save_to)

        os.makedirs(save_to)

        # generate the report set
        print(f'Generating report set and saving to {save_to}...')

        generate_report_set(count, save_to, all_clear_skies=clear_skies, all_cloudy=cloudy, all_blood_bath=blood_bath)

        print('Done')

    else:
        gen_random_results_report(save_to, clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath, verbose=True)

    return node, count, save_to

def gen_sample_results():
    return gen_sample_results_dialog()

def post_to_confluence(node, report_file):
    auth = get_confl_auth()
    page_title = get_confl_pagetitle(node)
    page_id = get_confl_pageid(node)
    commit = get_report_commit(report_file)

    with open(report_file, 'r') as f:
        new_html = str(f.read())

    add_results_to_confluence( new_html, 'Not Available', commit, page_id, page_title, auth )

def post_sample_results_to_confluence():

    print('')
    print('First generate the sample reports...')

    node, amount, save_location = gen_sample_results_dialog()

    report_files = []

    if amount > 1:
        for i in range(amount):
            report_files.append(os.path.join( save_location, 'Report_{}.html'.format(i+1)))

    else:
        report_files.append(save_location)

    print('')
    ans = input(f'Post to {node} confluence [/y/n]: ').strip()

    if ans != '' and ans != 'y':
        node = console_select_options(list(TESTNODE_CONFLUENCE_PAGES.keys()), prompt='Post results to confluence for which node')
            
    print('')
    for report_file in report_files:
        print('Posting Report {} to confluence'.format(report_file))
        post_to_confluence(node, report_file)
        if amount>1: time.sleep(5) #need buffer time when posting multiple reports

    print('')
    print('Populating other git information...')

    populate_git_info_on_confluence(get_confl_auth(), node, all_commits=True)

    print('')
    print('Done! Check results at {}'.format(get_confl_page_link(node)))

def set_confl_credentials():
    print('')
    overwrite_creds = True

    try:
        _, passwd =  get_confl_auth()
        ans = input(f'Credentials have already been detected on your system ({CONFLUENCE_KEYRING}), overwrite them? [y/n]: ').lower().strip()
        overwrite_creds = ( ans == 'y' )
    except Exception:
        print('You will need to store the API Key provided to you for confluence credentials')

    if overwrite_creds:
        key = input('Enter Key/Password: ')

        if set_confl_auth(key) == 0:
            print(f'Credentials saved to {CONFLUENCE_KEYRING}')
        else:
            print(f'ERROR: Credentials could not be saved to: {CONFLUENCE_KEYRING}')

    else:
        print('Credentials were not overwritten')
        return 0

def set_email_credentials():
    print('')
    overwrite_creds = True

    try:
        _, passwd =  get_email_creds()
        ans = input(f'Credentials have already been detected on your system ({EMAIL_KEYRING}), overwrite them? [y/n]: ').lower().strip()
        overwrite_creds = ( ans == 'y' )
    except Exception:
        print(f'Please enter the provided key to be saved to {EMAIL_KEYRING} credential')

    if overwrite_creds:
        key = input('Enter Key/Password: ')

        if set_email_creds(key) == 0:
            print(f'Credentials saved to {EMAIL_KEYRING}')
            return 0
        else:
            print(f'ERROR: Credentials could not be saved to: {EMAIL_KEYRING}')
            return 1

    else:
        print('Credentials were not overwritten')
        return 0

def main():
    set_confl_creds_opt = 'Set Confluence Credentials'
    set_email_creds_opt = 'Set Email Credentials'
    view_confluence_pages_opt = 'View node confluence pages'
    gen_sample_results_opt = 'Generate sample test results'
    post_sample_results_to_confluence_opt = 'Post sample test results to confluence'


    main_menu_opt = console_select_options([set_confl_creds_opt, set_email_creds_opt, view_confluence_pages_opt, gen_sample_results_opt, post_sample_results_to_confluence_opt], prompt='Select an action')

    if main_menu_opt == set_confl_creds_opt:
        return set_confl_credentials()

    elif main_menu_opt == set_email_creds_opt:
        return set_email_credentials()

    elif main_menu_opt == view_confluence_pages_opt:
        view_confluence_pages()
        return 0

    elif main_menu_opt == gen_sample_results_opt:
        gen_sample_results()
        return 0

    elif main_menu_opt == post_sample_results_to_confluence_opt:
        post_sample_results_to_confluence()
        return 0

if __name__ == "__main__":
    sys.exit(main())

