import traceback
import os
import sys
from datetime import datetime as ourdt

script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
if script_loc_dir not in sys.path:  sys.path.append(script_loc_dir)
from confluence_and_html_functions import *


#-----------------------------------------------------------------------------
# Globals

MAX_ENTRIES = 100


def get_result_files(results_folder, changelist_number):
    html_file = None
    all_report = None

    #go through all the report directories and look at their CL number file
    #CL_<commit number>.txt, if we find a match then we already know the html file name since it is the same as the report directory

    print('Finding HTML file for CL: '+changelist_number+'...')

    cl_file = "CL_"+changelist_number.replace("\\","-").replace("/","-")+".txt"
    
    for filename in os.listdir(results_folder):
        cur_results_dir = os.path.join (results_folder, filename)
        if os.path.isdir( cur_results_dir ) and (cl_file in os.listdir(cur_results_dir)):
            #if the current item is a directory and contains the appropriate cl_file, then we know that this the right html
            html_file = os.path.join(results_folder, str(filename)+".html")
            all_report = os.path.join(cur_results_dir, 'all_report.xml' )
            break

    return html_file, all_report

def get_driver_version_from_report(all_report_addr):
    #gets the driver version from the all_report.xml file
    #we assume there is only one adapter

    if (all_report_addr is None) or (not os.path.exists(all_report_addr)):
        return 'Not Available'

    with open(all_report_addr, 'r') as file:
        all_report = BeautifulSoup(file.read(), "xml")
    #get adapter at index 0
    adapter = all_report.find_all('adapter', attrs={'name': lambda x: x is not None and x == '0'})[0]
    #get the info and parse to get the driver version
    info = adapter.find_all('adapter')[0]
    try:
        driver_version = info.find_all('value', attrs={'name': lambda x: x == 'driver_version'})[0].string
        return driver_version
    except IndexError:
        return 'Non-AMD Graphics Card'

def getTestInfoForRow(parsedrow):
    testinfo = {
        'name': parsedrow[0],
        'total': int(parsedrow[1]),
        'passed': int(parsedrow[2]),
        'failed': int(parsedrow[3]),
        'skipped': int(parsedrow[4]),
        'time_taken': parsedrow[5]
    }

    return testinfo

def get_table_info(results_table_in):
    """
    will parse the table and return a dictionary in the format:
    {
        'order': <test names in the order they appear on the html>
        'info' : {
            '<test_name>': {
                <name>,<total>,<passed>,<failed>,<skipped>,<time>,<regression>
            }
        }
    }
    """

    table_info = {
        'tests_list': [],
        'tests_info': {}
    }

    #first parse the results table into a python table i.e. row of rows
    table = parseHTMLTable(results_table_in)

    #first pop the header
    table.pop(0)

    #for every other table parse the test information
    for a_row in table:
        test_info = getTestInfoForRow(a_row)

        if len(a_row) >= 7:
            #this table has a regression column
            test_info['regression'] = a_row[6]

        test_name = test_info['name']

        table_info['tests_list'].append(test_name)
        table_info['tests_info'][test_name] = test_info

    return table_info

# def get_err_on_total(test, table_soup):
#     #checks the passed in table soup to see if the total value for the passed in test is marked as red
#     #i.e. has an error
#     test_row = get_matching_rows_from_table_soup(table_soup, keys=[test])[0]

#     #check the total cell for that row
#     total_cell = test_row.find_all('td')[1]
#     if total_cell['bgcolor'] == 'red':
#         #if the total value has a red background color, then we know it was err on total when it was being posted
#         return True
#     return False


def generate_new_cl_data(new_cl, new_results_table, last_cl, last_results_table):
    #first generate the data info for both the new_cl and the last_cl
    new_cl_info = get_table_info(new_results_table)

    prev_test_err_on_total = False


    last_cl_info = None
    if last_cl != None and last_results_table != None:
        #case where this node has never been posted on
        last_cl_info = get_table_info(last_results_table)



    for test_name in new_cl_info['tests_list']:
        new_test = new_cl_info['tests_info'][test_name]
        new_test['err_on_total'] = False
        new_test['regression'] = '--'

        prev_test = None
        if (last_cl_info != None) and (test_name in last_cl_info['tests_info']):
            prev_test = last_cl_info['tests_info'][test_name]

        if prev_test != None:
            #first do a total comparison
            #a reduction in total test suite should be marked as error
            if new_test['total'] < prev_test['total']:
                new_test['err_on_total'] = True

            #if there are failures or skipped cases,
            if (new_test['failed'] > 0) or (new_test['skipped'] > 0):
                if (prev_test['failed'] > 0) or (prev_test['skipped'] > 0):
                    #failures on last cl as well, use the last cl regression
                    new_test['regression'] = prev_test['regression']
                else:
                    #regression is the current cl
                    new_test['regression'] = new_cl

        else:
            #this is either the first commit to the confluence or this test was not run on previous cl
            #cannot do a total diff, and all regressions will be the current cl
            if (new_test['failed'] > 0) or (new_test['skipped'] > 0):
                #if there are failures or skips, then set the regression to be this cl
                #since its the only one
                new_test['regression'] = new_cl

    return new_cl_info


def make_info_table(cl, link, driver_version):
    author = ''
    date = ourdt.today().strftime('%H:%M, %d %b %Y')
    commit_link_html = make_hyperlink(make_commit_link(cl))

    return make_html_table([
            ['Author',      author],
            ['Date',        date],
            ['Drivers',     driver_version],
            ['Commit Link', commit_link_html],
            ['Build Link',  str(link)]
        ], side_header=True)

    return html


def make_results_table(cl_info):
    #cl_info is in format of output of get_table_info

    html = '<table>'
    html += '<tbody>'
    #add the header
    html += '<tr>'
    html += '<th>Test name</th>'
    html += '<th>Total</th>'
    html += '<th>Passed</th>'
    html += '<th>Failed</th>'
    html += '<th>Skipped</th>'
    html += '<th>Time taken</th>'
    html += '<th>Possible CL# Regression</th>'
    html += '</tr>'


    table = [
        ['Test name', 'Total', 'Passed', 'Failed', 'Skipped', 'Time taken', 'Possible CL# Regression']
    ]

    for test_name in cl_info['tests_list']:
        #for each test put their info in the table
        test = cl_info['tests_info'][test_name]
        
        html += '<tr>'

        html += '<td>{name}</td>'.format(name=test_name)

        html += make_html_color_cell(test['total'], color='red', condition=test['err_on_total'])
        html += make_html_color_cell(test['passed'], color='green', condition=True)
        html += make_html_color_cell(test['failed'], color='red', condition=(test['failed'] > 0))
        html += make_html_color_cell(test['skipped'], color='orange', condition=(test['skipped'] > 0))

        html += '<td>{value}</td>'.format(value=test['time_taken'])
        html += '<td>{value}</td>'.format(value=test['regression'])

        html += '</tr>'

    html += '</tbody>'
    html += '</table>'

    return html

def add_cl_to_soup(new_cl, new_link, cl_info, driver_version, confluence_soup):
    #confluence soup has been preprocessed

    #first lets make the header html for the new_cl
    new_header = confluence_soup.new_tag('h3')
    new_header['id'] = new_cl+'_cl_header'
    new_header.string = 'CL#'+new_cl


    #next we need the info table
    new_info_table = parse_html(make_info_table(new_cl, new_link, driver_version)).contents[0]
    new_info_table['id'] = new_cl+'_info_table'
    new_info_table['border'] = '1'

    #next we need the results table
    new_results_table = parse_html(make_results_table(cl_info)).contents[0]
    new_results_table['id'] = new_cl+'_results_table'
    new_results_table['border'] = '1'

    #finally add the new html objects to the soup
    confluence_soup.insert(0, new_header)
    confluence_soup.insert(1, new_info_table)
    confluence_soup.insert(2, new_results_table)


def write_to_file(content):
    with open('localoutput.txt','w') as file:
        file.write(str(content))

def test_node_connection(auth, pageid, title):
    try:
        soup = get_test_results_confluence_soup(auth, pageid)
        print('Machine Can Connect!')
        print('Page Info:')
        pprint({
            'title': title,
            'pageid': pageid
            })
        print('SUCCESS')
        return 0
    except Exception as e:
        print('Machine cannot connect! Traceback:')
        traceback.print_exc()
        print('FAILURE')
        return 1


def add_results_to_confluence(new_html, driver_version, new_cl, pageid, title, auth):
    #get the data for the new_cl: results_table, and link
    #done using beautful soup
    print('Getting New HTML Data...')
    new_soup = parse_html(new_html)

    new_link = new_soup.find_all('a')[0]
    new_results_table = new_soup.find_all('table')[-1]

    #get the current information on the confluence page and parse to confluence soup
    print('Getting HTML Data from Confluence...')
    confluence_soup = get_test_results_confluence_soup(auth, pageid)

    print('Processing results for CL: {cl}...'.format(cl=new_cl))
    #get CL list from confluence (in order of newest to oldest)
    #preprocess the cl_list to remove duplicates and keep max size
    cls_on_confluence = get_cls_from_soup(confluence_soup, ommit=[new_cl], limit=MAX_ENTRIES-1)

    last_cl_posted = None
    last_results_table = None

    if len(cls_on_confluence) > 0:
        last_cl_posted = cls_on_confluence[0]
        last_results_table = get_results_table(last_cl_posted, confluence_soup)

    #generate data for the new cl using last uploaded cl and confluence soup
    new_cl_info = generate_new_cl_data(new_cl, new_results_table, last_cl_posted, last_results_table)

    #add the cl to the confluence soup
    add_cl_to_soup(new_cl, new_link, new_cl_info, driver_version, confluence_soup)

    #write the new soup to the confluence page
    print('Writing to Confluence...')

    # with open('confluence_soup.html', 'w') as file:
    #     file.write(str(confluence_soup))
    # return

    write_confluence_soup(confluence_soup, auth, pageid, title)

def get_report_commit(report_file):
    report_soup = parse_html_file(report_file)
    system_info_table = report_soup.find_all(attrs={'id':'small'})[1]

    for t_row in system_info_table.find_all('tr'):
        header = t_row.find_all('th')[0]
        if header.string.lower() == 'build':
            return t_row.find_all('td')[0].string

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
        "--CHANGELIST_NUMBER",
        required=False,
        type = str,
        metavar='<CL#>',
        help = "Specify which CL# this test belongs to"
    )

    parser.add_argument(
        "--passwd",
        required=False,
        type=str,
        metavar='<password>',
        help='Password used to post to confluence'
    )

    parser.add_argument(
        "--file",
        required=False,
        type=str,
        metavar='<filename>',
        help='To upload a specific html file to confluence'
    )
    parser.add_argument(
        "--pageid",
        required=False,
        type=str,
        metavar='<page ID>',
        help='Page ID of the confluence page to post stuff to, used for putting backlogged info up'
    )
    parser.add_argument(
        "--pagetitle",
        required=False,
        type=str,
        metavar='<page ID>',
        help='Page ID of the confluence page to post stuff to, used for putting backlogged info up'
    )
    parser.add_argument(
        "--drivers",
        required=False,
        type=str,
        metavar='<driver version>',
        help='The version of drivers currently on the node, will override any values pulled from all_report.xml'
    )
    parser.add_argument('--sample', '--sample', action='store_true', help='Used when we are posting a samle generated report')
    parser.add_argument('--skip_html', '--skip_html', action='store_true', help='Testing the script, disables email sending')
    parser.add_argument('--test_connection', '--test_connection', action='store_true', help='Testing the script, disables email sending')
    parser.add_argument(
        "-auth",
        required=False,
        type=str,
        metavar='<username,password>',
        help='Override authentication with passed in authentication'
    )



    options = parser.parse_args()
    try:
        NODE_PAGE_ID = None
        NODE_PAGE_TITLE = None

        print('Getting Log In Credentials...')
        if options.auth is not None:
            username = str(options.auth).split(',')[0]
            passw = str(options.auth).split(',')[1]
            auth = (username, passw)
        else:
            try:
                auth = get_personal_confluence_auth()
            except Exception as e:
                raise Exception('ERROR: Could not get login credentials from node for confluence:'+str(e))

        # Get node information (page title, confluence pageid)
        if options.pageid is not None:
            NODE_PAGE_ID = options.pageid
        else:
            NODE_PAGE_ID = get_confl_pageid(options.node)

        if options.pagetitle is not None:
            NODE_PAGE_TITLE = options.pagetitle
        else:
            NODE_PAGE_TITLE = get_confl_pagetitle(options.node)
        
        if (NODE_PAGE_ID is None) or (NODE_PAGE_TITLE is None):
            raise Exception('No page id or title provided')

        if options.test_connection:
            return test_node_connection(auth, NODE_PAGE_ID, NODE_PAGE_TITLE)

        if options.skip_html:
            html = '<html></html>'
        else:
            html_file = None
            all_report = None

            if options.file is not None:
                #Use a specified file instead of doing a search based on the cl number
                html_file = options.file

                # if we are using a sample commit and no commit hash is specified, we can parse it from the report
                if (options.sample) and (not options.CHANGELIST_NUMBER): options.CHANGELIST_NUMBER = get_report_commit(html_file)
            else:
                # Find the html file name in the _results folder
                html_file, all_report = get_result_files( os.path.join(os.getcwd(), "_results"), options.CHANGELIST_NUMBER)

            #check if the html_file exists
            if (html_file is None) or (not os.path.exists(html_file)):
                raise Exception('Cannot find HTML file: {file}'.format(file=html_file))
            print('FILE: {file}'.format(file=html_file))

            #get the driver version
            if options.drivers is not None:
                driver_version = str(options.drivers)
            else:
                driver_version = get_driver_version_from_report(all_report)
            print('DRIVERS: {version}'.format(version=driver_version))

            #get the html data in the file
            html = ""
            with open(html_file, 'r') as fd:
                html = fd.read()

        add_results_to_confluence(html, driver_version, options.CHANGELIST_NUMBER, NODE_PAGE_ID, NODE_PAGE_TITLE, auth)
        return 0
    except:
        print("ERROR: Something went wrong while posting to confluence:")
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
