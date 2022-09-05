import traceback
import os
import sys

import time

from random import seed
from random import randint
import random
from tabulate import tabulate #pip install tabulate

script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
if script_loc_dir not in sys.path:  sys.path.append(script_loc_dir)
from confluence_and_html_functions import *
from repository_git_functions import gen_new_commits

tests_info = [
    ['CropTool Test', 432],
    ['Capability Test', 2],
    ['Internal Engine Test', 3],
    ['Converter Test', 150],
    ['Decoder AV1 Test', 12],
    ['Decoder H264AVC Test', 20],
    ['Encoder Render Test', 28],
    ['Encoder Latency Test', 24],
    ['Transcoder Test', 50],
    ['Encoder AVC Test', 180],
    ['Encoder HEVC Test', 192],
    ['Encoder Vs FFMPEG Test', 30]
]

def get_base_soup():
    html = '<html><head><meta content="text/html; charset=utf-8" http-equiv="Content-Type"/><style> a:link {color: "black"} a:visited {color: "black"} a:active {color: "black"} table, th, td { border: 1px solid black; padding: 2px; } table { border-collapse: collapse; } h3 { display: block; font-size: 1.17em; margin-top: 1em; margin-bottom: 1em; margin-left: 0; margin-right: 0; font-weight: bold; } #heap { font-size: 16px; font-family: Arial,"Bitstream Vera Sans",Helvetica,Verdana,sans-serif; } #heap td { text-align: center; padding: 4px 7px 2px; vertical-align: top; } #heap th { text-shadow: rgba(255, 255, 255, 0.796875) 0px 1px 0px; font-weight: normal; line-height: 1.3em; background-color: "#d0e0e0" } #small { font-size: 12px; font-family: Arial,"Bitstream Vera Sans",Helvetica,Verdana,sans-serif; } #small th { text-shadow: rgba(255, 255, 255, 0.796875) 0px 1px 0px; font-weight: normal; line-height: 1.3em; background-color: "#d0e0e0" } #small td { text-align: center; padding: 4px 7px 2px; vertical-align: top; } </style></head><body><h3>Summary: [41 FAIL(s)] </h3> Project Link: <a href="http://my-jenkins.com/jobs/TestingPipeline/builds/1234">http://my-jenkins.com/jobs/TestingPipeline/builds/1234</a><br/><br/> Jenkins Link: <a href="http://my-jenkins.com/jobs/TestingPipeline/builds/1234">http://my-jenkins.com/jobs/TestingPipeline/builds/1234</a><br/><hr/><br/><table border="1" id="small"><tr><th>Total</th><th>Passed</th><th>Failed</th><th>Skipped</th></tr><tr><td>1123</td><td bgcolor=" lightgreen ">1054</td><td bgcolor=" red ">41</td><td bgcolor=" SandyBrown ">28</td></tr></table><hr/><h3>System</h3><table border="1" id="small"><tr align="left"><th>CPUMHz</th><td>3998</td></tr><tr align="left"><th>CPUName</th><td>Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz</td></tr><tr align="left"><th>Host</th><td>WIN10-NAVI21</td></tr><tr align="left"><th>Memory</th><td>16320</td></tr><tr align="left"><th>OS</th><td>10.0.19041X64(Build:1023)</td></tr><tr align="left"><th>build</th><td>15b1247675adc2fedbb2e01f2c0eaa50</td></tr><tr align="left"><th>info</th><td>1.4.23.0</td></tr></table><hr/><h2>Details</h2><h3>Adapter: Navi21 [#0] </h3><table border="1" id="small"><tr><td colspan="8"><b>#</b>0 </td></tr><tr><th>Description</th><th>Driver Version</th><th>Device ID</th><th>Revision</th><th>Family</th><th>Target Version</th><th>Header</th><th>Runtime</th></tr><tr><td>AMD Radeon RX 6900 XT</td><td>20.40-200101n-230356E-ATI</td><td>AAAA</td><td>01</td><td>Navi21</td><td>20.40</td><td>1.4.22</td><td>1.4.22</td></tr><tr><td colspan="8"><b>Components</b></td></tr><tr><td colspan="8">Converter;CropTool;Encoder;Decoder;</td></tr></table><a name="0_details"></a><br/><table border="1" id="heap"><tr><th>Test name</th><th>Total</th><th>Passed</th><th>Failed</th><th>Skipped</th><th>Time taken</th></tr></table><hr/></body></html>'

    return parse_html(html)

def gen_random_matrix(total, weighting=None, clear_skies=False, cloudy=False, blood_bath=False):
    all_passes = 0
    all_fails = 1
    all_skips = 2
    random_distr = 3

    #weight key ( P[all passes], P[all failures], P[all skips], P[random matrix])

    standard_weights = (50,13,13,24) # used in regular behaviour
    clear_skies_weights = (100,0,0,0) # no failures at  alll
    cloudy_weights = (75,5,5,15) # some failures, better than standard worse than clear skies
    blood_bath_weights = (10,10,10,70) # terrible failures every where

    if not weighting:
        #if no weighting was specified check if an option was set
        if clear_skies: weighting = clear_skies_weights
        elif cloudy: weighting = cloudy_weights
        elif blood_bath: weighting = blood_bath_weights
        else: weighting = standard_weights

    # select a choice based on the provided weighting
    choice = random.choices([all_passes, all_fails, all_skips, random_distr], weights=weighting, k=1)[0]

    passed = failed = skipped = 0
    # apply choice behaviour to matrix
    if choice == all_passes: passed = total

    elif choice == all_fails: failed = total

    elif choice == all_skips: skipped = total

    iterations = 0
    while ( passed + failed + skipped != total):
        #randomly generate a matching matrix
        passed = random.randrange(0, total+1)
        failed = random.randrange(0, total+1)
        skipped = random.randrange(0, total+1)
        iterations += 1

    return passed, failed, skipped, iterations

def gen_random_execution_time():
    return random.randrange(1, 501)

def gen_random_results(weighting=None, clear_skies=False, cloudy=False, blood_bath=False):
    #generates python table of random results
    result_rows = []
    for pair in tests_info:
        test_name = pair[0]
        tests_total = pair[1]

        #first generate the passed, failed, skipped
        p, f, s, _ = gen_random_matrix(tests_total, weighting=weighting, clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath)

        exec_time = gen_random_execution_time()

        result_rows.append([test_name, tests_total, p, f, s, exec_time])

    return result_rows

def generate_random_commit_hash():
    return gen_new_commits(count=1, random_padding=True, padding=10)[0]


def gen_html_test_results_row(row):
    #row is in format
    #[ Test, total, passed, failed, skipped, execution time]
    html = '<tr>'

    #test name is always left aligned
    html += f'<td style="text-align: left;">{row[0]}</td>'

    #total
    html += make_html_color_cell(row[1])

    #passed
    html += make_html_color_cell(row[2], color='lightgreen', condition=True)

    #failed
    html += make_html_color_cell(row[3], color='red', otherwise_color='lightgreen', condition=row[3]>0)

    #skipped
    html += make_html_color_cell( row[4], color='SandyBrown', condition=row[4]>0)

    #execution time
    html += f'<td>{row[5]} s</td>'


    html += '</tr>'
    return html

def update_results_table(table, reference_soup):
    results_table = reference_soup.find_all(attrs={'id':'heap'})[0]

    for row in table:
        new_row = parse_html( gen_html_test_results_row(row) ).contents[0]
        results_table.append(new_row)

def udpate_commit_hash(commit_hash, reference_soup):
    '''
    elements with id small in order:
    summary table, system info table, adapter info table
    '''
    system_info_table = reference_soup.find_all(attrs={'id':'small'})[1]

    for t_row in system_info_table.find_all('tr'):
        header = t_row.find_all('th')[0]
        if header.string.lower() == 'build':
            t_row.find_all('td')[0].string = commit_hash
            break

def update_node_name( node, reference_soup):
    system_info_table = reference_soup.find_all(attrs={'id':'small'})[1]

    for t_row in system_info_table.find_all('tr'):
        header = t_row.find_all('th')[0]
        if header.string.lower() == 'host':
            t_row.find_all('td')[0].string = node
            breaks

def update_summary_table(total, passed, failed, skipped, reference_soup):
    summary_table = reference_soup.find_all(attrs={'id':'small'})[0]

    summary_row = summary_table.find_all('tr')[1]

    for td, matching_val in zip(summary_row.find_all('td'), [total, passed, failed, skipped]):
        td.string = str(matching_val)

def update_build_links(build, reference_soup):
    new_link = 'http://my-jenkins.com/jobs/TestingPipeline/builds/{}'.format(build)
    for i, tag in enumerate(reference_soup.find_all('a')):
        if i > 1: break
        tag['href'] = new_link
        tag.string = new_link


def gen_random_results_report_soup(verbose=False, return_commit_hash=False, clear_skies=False, cloudy=False, blood_bath=False):
    # generate the random commit hash
    commit_hash = generate_random_commit_hash()

    # generate the random test results
    test_results = gen_random_results(clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath)

    # get totals
    sum_total = sum_passed = sum_failed = sum_skipped = sum_exec = 0
    for row in test_results:
        sum_total += row[1]
        sum_passed += row[2]
        sum_failed += row[3]
        sum_skipped += row[4]
        sum_exec += row[5]

    # generate random jenkins build
    jenkins_build = random.randrange(0, 10000)

    # get reference soup
    base_soup = get_base_soup()

    # modify project link and jenkins link
    update_build_links(jenkins_build, base_soup)

    # modify build value to put in commit hash
    udpate_commit_hash(commit_hash, base_soup)

    # modify test results table with random results
    update_results_table(test_results, base_soup)

    # modify summary table with totals
    update_summary_table(sum_total, sum_passed, sum_failed, sum_skipped, base_soup)

    #modify the summary header
    summary_header = base_soup.find_all('h3')[0]

    fail_info = None
    
    if sum_failed > 0: fail_info = '{} FAIL(s)'.format(sum_failed)

    if sum_skipped > 0: fail_info = '{}{} SKIP(s)'.format( fail_info+' and ' if fail_info else '', sum_skipped )

    summary_header.string = 'Summary: [{}]'.format('PASS' if not fail_info else fail_info)

    if verbose:
        print('Basic Information')
        print( tabulate([
                    [ 'Commit', commit_hash],
                    [ 'Build', jenkins_build]
                ]))
        print('Results')
        results = list(test_results)
        results.append( ['', '', '', '', '', ''])
        results.append(['Cumulative', sum_total, sum_passed, sum_failed, sum_skipped, sum_exec] )
        print(tabulate( results , headers=['Test name', 'Total', 'Passed', 'Failed', 'Skipped', 'Exec Time']))

    if return_commit_hash:
        return base_soup, commit_hash
    else:
        return base_soup


def gen_result_set_in_one_report(n, faddr, all_clear_skies=False, all_cloudy=False, all_blood_bath=False):
    #run the results the specified number of times
    base_soup = get_base_soup()

    std_opt = 0
    clear_skies_opt = 1
    cloudy_opt = 2
    blood_bath_opt = 3

    clear_skies = False
    blood_bath = False
    cloudy = False

    opt = 0
    if all_clear_skies:
        clear_skies = True
        opt = 1
    elif all_cloudy:
        cloudy = True
        opt = 2
    elif all_blood_bath:
        blood_bath = True
        opt = 3

    for _ in range(n):

        #choose a random case for the current result
        if not(all_clear_skies or all_cloudy or all_blood_bath):
            opt = random.randrange(0,4)
            #while opt == 2: opt = randint(0,3)
            clear_skies = ( opt == clear_skies_opt )
            blood_bath = ( opt == blood_bath_opt )
            cloudy = ( opt == cloudy_opt )

        test_results = [[ '', 0, 0, 0, 0, 0],[ '', 0, 0, 0, 0, 0],[ 'Opt: {}'.format(opt), 0, 0, 0, 0, 0]] + gen_random_results(clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath)

        #add each result to the html table
        update_results_table(test_results, base_soup)

    #save the file
    with open(faddr, 'w') as file:
        file.write( str(base_soup))



def testing():
    print("Generating...")

    gen_result_set_in_one_report(50, 'C:\\Users\\omnic\\local\\GitRepos\\AMDScripts\\personal_confluence_and_email_scripts\\Testing.html')

    # pos = mouse.position()
    # mouse.moveTo(108, 78)
    # # time.sleep(0.5)
    # # print('Click!')
    # mouse.click(button='left')
    # mouse.moveTo(pos)
    print('Done')

def generate_report_set(amount, dest_dir, all_clear_skies=False, all_cloudy=False, all_blood_bath=False):
    if not os.path.exists(dest_dir): raise Exception('Could not find dest_dir {}'.format(dest_dir))

    # all_clear_skies means all generated reports have clear skies resukts
    # all_cloudy means all generated reports have cloudy results
    # all_blood_bath means all generated reports have blood bath results
    # no specification means that it is randomized


    # generate the required amount of commits outside here!
    # and then pass them on to the next report function
    # need to update personal_jenkins_git_function to add padding random number of commits between generated commits

    std_opt = 0
    clear_skies_opt = 1
    cloudy_opt = 2
    blood_bath_opt = 3

    clear_skies = False
    blood_bath = False
    cloudy = False

    opt = 0
    if all_clear_skies:
        clear_skies = True
        opt = 1
    elif all_cloudy:
        cloudy = True
        opt = 2
    elif all_blood_bath:
        blood_bath = True
        opt = 3

    for i in range(amount):

        #chooses random case for the current result
        if not(all_clear_skies or all_cloudy or all_blood_bath):
            opt = random.randrange(0,4)
            clear_skies = ( opt == clear_skies_opt )
            blood_bath = ( opt == blood_bath_opt )
            cloudy = ( opt == cloudy_opt )

        made_soup , commit_hash = gen_random_results_report_soup(return_commit_hash=True, clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath)
        file_name = os.path.join(dest_dir, 'Report_{}.html'.format(i+1))
        with open(file_name, 'w') as file:
            file.write(str(made_soup))

def gen_random_results_report(file_addr, verbose=False, clear_skies=False, cloudy=False, blood_bath=False):
    with open(file_addr, 'w') as file:
        file.write( str(gen_random_results_report_soup(verbose=verbose, clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath)) )
    if verbose: print(f'Report saved to {file_addr}')

def report_type_help_str():
    file = '''
Tyype of report to generate:
Brackets specify the required argument
Clear Skies: (clearskies) Generated report(s) have no failures
Cloudy:      (cloudy)      Generated report(s) have little to no failures
Standard:    (standard)    Generated report(s) have few to several failures (random distribution)
Blood Bath:  (bloodbath)  Generated report(s) have several failures

When generating multiple reports, above options are used to specify the type of all reports generated
Also have access to random type when generating multiple reports:

Random:      (random)      Each generated report in set will be one of the above types selected at random

If not specified, (standard) is used for one report, (random) for report set
    '''
    return file

def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-count",
        required=True,
        type = int,
        metavar='<CL#>',
        help = "Amount of reports to generate"
    )

    parser.add_argument(
        "-save_to",
        required=True,
        type=str,
        metavar='<address>',
        help='Location to save generated reports, is file address for one report '
    )

    parser.add_argument(
        "-type",
        required=False,
        type=str,
        metavar='<string>',
        help=report_type_help_str()
    )

    parser.add_argument('--verbose', '--verbose', action='store_true', help='Testing the script, disables email sending')
    options = parser.parse_args()


    if not options.type:
        if options.count > 1:
            options.type = 'random'
        else:
            options.type = 'standard'

    options.type = options.type.strip().lower()

    clear_skies = ( options.type == 'clearskies')
    cloudy = ( options.type == 'cloudy' )
    blood_bath = ( options.type == 'bloodbath' )

    if options.count > 1:
        if not os.path.exists( options.save_to ):
            print(f"ERROR: '{options.save_to}' does not exist!")
            return 1

        if not os.path.isdir (options.save_to):
            print(f"ERROR: '{options.save_to}' is not a directory!")
            return 1

        print('Generating Reports...')
        generate_report_set(options.count, options.save_to, all_clear_skies=clear_skies, all_cloudy=cloudy, all_blood_bath=blood_bath)
        print(f'Reports saved at {options.save_to}')
    else:
        if not re.match('.*\\.html', options.save_to):
            print(f"ERROR: '{options.save_to}' is not a HTML file address")
            return 1

        gen_random_results_report(options.save_to, clear_skies=clear_skies, cloudy=cloudy, blood_bath=blood_bath, verbose=True)

    return 0

if __name__ == "__main__":
    sys.exit(main())

