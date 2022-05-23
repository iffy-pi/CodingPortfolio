#-----------------------------------------------------------------------------
'''
Popualtes the git information on the confluence page
Called by Test Results Git Populator
'''
#-----------------------------------------------------------------------------

import traceback
import os
import sys

script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
if script_loc_dir not in sys.path:  sys.path.append(script_loc_dir)
from confluence_and_html_functions import *
from repository_git_functions import getCommitHistory, findCommitsBetween

MAX_INBETWEEN_CLS_DISPLAYED = 10

def tprint(text):
    if TESTING:
        print(text)

def find_value_in_table(name, table):
    #finds the specified value for 'name' in table with 2 columns
    for row in table:
        if row[0] == name:
            return str(row[1])
    return None

def make_commit_hyperlink(cl):
    return make_hyperlink(make_commit_link(cl), text=cl[:7])
  
def set_author_in_soup(author, info_table_soup):
    #find the author row and then set the value
    #first find the appropriate row
    all_rows = info_table_soup.find_all('tr')
    for row_soup in all_rows:
        #check if the td information matches author and set if it does
        all_tds = row_soup.find_all(['th','td'])
        if all_tds[0].string == 'Author':
            all_tds[1].string = author
            return

    raise Exception('Author field was never found in info table for setting')

def populate_author_for_info_table(info_table_soup):
    #fills in the empty author in the info table soup
    # get the info table for the commit

    changes_made = False

    parsed_table = parseHTMLTable(info_table_soup)
    author = find_value_in_table('Author', parsed_table)
    commit = info_table_soup['id'].replace('_info_table', '')

    if author is None:
        raise Exception('Error parsing info table for filling in empty author, author is None')

    if author == '':
        if commit in COMMIT_HISTORY_DIX['hashinfo']:
            author = COMMIT_HISTORY_DIX['hashinfo'][commit]['author_name']

            print(f'Setting Author {author} in CL#{commit}')
            #write the value to the info soup
            set_author_in_soup(author, info_table_soup)
            changes_made = True
        else:
            print(f'{commit} is not in commit history')

    return changes_made


def populate_empty_author(commit, confluence_soup):
    #fills in the empty author and writes the soup to the confluence.
    # get the info table for the commit
    return populate_author_for_info_table(get_info_table(commit, confluence_soup))

def populate_all_empty_authors(confluence_soup):
    #fills in all empty authors in the test results confluence soup
    info_table_soups = get_tables_from_soup(confluence_soup, info_tables=True)

    changes_made = False

    for info_soup in info_table_soups:
        if populate_author_for_info_table(info_soup):
            changes_made = True

    if not changes_made:
        print('All authors set')

    return changes_made

def populate_commits_inbetween_for_info_table(cls_inbetween, info_table_soup):
    if cls_inbetween is None:
        table_text = 'Unavailable'
    elif cls_inbetween == []:
        table_text = 'None (They are consecutive)'
    else:
        #we want to have a limit of cls
        cl_count = len(cls_inbetween)

        table_text = make_commit_hyperlink(cls_inbetween[0])
        for i in range(1, min(cl_count, MAX_INBETWEEN_CLS_DISPLAYED) ):
            table_text = table_text + ', ' + make_commit_hyperlink(cls_inbetween[i])

        if cl_count > MAX_INBETWEEN_CLS_DISPLAYED:
            table_text += ' and {} more.'.format(cl_count - MAX_INBETWEEN_CLS_DISPLAYED)

    #print('\nTABLE TEXT: {}\n'.format(table_text))
    #make new row and append to soup
    new_info_row = parse_html('<tr><th>Commits Since Previous</th><td>{content}</td></tr>'.format(content=table_text)).contents[0]
    info_table_soup.tbody.append(new_info_row)

    return True



def populate_commits_inbetween(current_commit, previous_commit, confluence_soup, overwrite=False):

    info_table_soup = get_info_table(current_commit, confluence_soup)

    if not overwrite:
        # first check if there is already a commits in be
        parsed_table = parseHTMLTable(info_table_soup)
        for rows in parsed_table:
            if rows[0] == 'Commits Since Previous':
                #if there is already a commits in between then we dont need to append it.
                #print(f'Commits In-Between are already set for {current_commit}')
                return False

    cls_inbetween = None

    if previous_commit:
        #if there is a previous commit get the commits in between
        cls_inbetween = findCommitsBetween(COMMIT_HISTORY_DIX['hashes'], later_than=previous_commit, earlier_than=current_commit)

    return populate_commits_inbetween_for_info_table(cls_inbetween, info_table_soup)


def populate_all_commits_inbetween(confluence_cls, confluence_soup, overwrite=False):
    previous_commit = None

    changes_made = False

    confluence_cls_oldest_to_newest = list(confluence_cls)
    confluence_cls_oldest_to_newest.reverse()


    for current_commit in confluence_cls_oldest_to_newest:

        if populate_commits_inbetween(current_commit, previous_commit, confluence_soup, overwrite=overwrite):
            print(f'Populated commits in-between for {current_commit}')
            changes_made = True

        # make new previous commit
        previous_commit = current_commit

    return changes_made

def populate_git_info_on_confluence(auth, node, commit=None, all_commits=False, testing=False, testing_post_to_confluence=False ):
    init_global_vars(testing=testing, testing_post_to_confluence=testing_post_to_confluence)

    global COMMIT_HISTORY_DIX
    COMMIT_HISTORY_DIX = getCommitHistory()

    new_data_for_confluence = False

    #get the confluence soup and populate the information
    print('Getting Confluence Soup...')
    confluence_soup = get_test_results_confluence_soup(auth, get_confl_pageid(node))

    confluence_cls = get_cls_from_soup(confluence_soup)

    if all_commits:
        # we are doing it for all commits
        print('Populating commit author for all commits...')
        if populate_all_empty_authors(confluence_soup):
            new_data_for_confluence = True

        print('Populating commits in-between for all commits...')
        if populate_all_commits_inbetween(confluence_cls, confluence_soup):
            new_data_for_confluence = True


    else:
        #doing a specific commit
        #if no commit is specified, do the latest one
        if not commit: commit = confluence_cls[0]

        print(f'Populating commit author for {commit}')
        if populate_empty_author(commit, confluence_soup):
            new_data_for_confluence = True

        try:
            previous_commit = confluence_cls [ confluence_cls.index(commit)+1 ]
        except IndexError:
            previous_commit = None

        print(f'Populating commits in-between for {commit}')
        if populate_commits_inbetween(commit, previous_commit, confluence_soup):
            new_data_for_confluence = True

    
    # with open('confluence_soup.html', 'w') as file:
    #     file.write(str(confluence_soup))
    # return 0

    #write to confluence if we have new data avaialable
    if new_data_for_confluence:

        if (not TESTING) or (TESTING and ENABLE_CONFLUENCE_TESTING):
            print('Writing to Confluence...')
            write_confluence_soup(confluence_soup, auth, get_confl_pageid(node), get_confl_pagetitle(node))
        else:
            print('POSTING TO CONFLUENCE IS DISABLED')

    return 0

def init_global_vars(testing=False, testing_post_to_confluence=False):
    if any( v not in globals() for v in ['TESTING', 'ENABLE_CONFLUENCE_TESTING']):
        global TESTING
        global ENABLE_CONFLUENCE_TESTING
        TESTING = testing
        ENABLE_CONFLUENCE_TESTING = testing_post_to_confluence


def main():


    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-node",
        required=True,
        type = str,
        metavar='<NodeName>',
        help = "Specify which Node to update on Confluence"
    )

    parser.add_argument(
        "-commit",
        required=False,
        type = str,
        metavar='<NodeName>',
        help = "Commit to populate information for, by default is latest commit"
    )

    parser.add_argument('--all_commits', '--all_commits', action='store_true', help='Populates git information for all commits on confluence page')



    #arguments used for testing purposes
    parser.add_argument('--testing', '--testing', action='store_true', help='Testing the script, disables email sending')
    parser.add_argument('--post_to_confluence', '--post_to_confluence', action='store_true', help='Used with --testing, enables writing to confluence')
    options = parser.parse_args()


    global TESTING
    global ENABLE_CONFLUENCE_TESTING #the actual commit file that is local

    global NODE #the node name
    global WRITE_TO_CONFLUENCE #determines if we need to write the page to confluence


    TESTING = options.testing
    ENABLE_CONFLUENCE_TESTING = options.post_to_confluence

    if TESTING:
        print('TESTING MODE')
        if ENABLE_CONFLUENCE_TESTING:
            print('POST TO CONFLUENCE IS ENABLED')
        else:
            print('POST TO CONFLUENCE IS DISABLED')
        print('\n')

    try:
        auth = get_login(USER_NAME, None)
    except Exception as e:
        print('main(): Could not get login credentials from node for confluence:')
        print('get_login(): '+str(e))
        return 2

    NODE = str(options.node)

    #Find the html file name in the _results folder
    try:
        return populate_git_info_on_confluence(auth, NODE, commit=options.commit, all_commits=options.all_commits)
    except:
        print("main(): Error Occured:")
        traceback.print_exc()
        return 2

if __name__ == "__main__":
    sys.exit(main())
