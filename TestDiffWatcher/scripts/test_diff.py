#-----------------------------------------------------------------------------
'''
Generates and sends the TestDiffWatcher emails
Called by TestDiffWatcher job in Jenkins
'''
#-----------------------------------------------------------------------------
import traceback
import os
import sys

try:
    prev_dir = os.getcwd()
    script_loc_dir = os.path.split(os.path.realpath(__file__))[0]
    os.chdir(script_loc_dir)
    from confluence_and_html_functions import *
    from mail import *
    from amf_jenkins_git_functions import getCommitHistory
    os.chdir(prev_dir)
except:
    print('Could not import necessary confluence and HTML functions from confluence_and_html_functions.py')
    traceback.print_exc()
    sys.exit(2)

#for graphing
import matplotlib.pyplot as plt #external install

#for image exporting
import io
import base64
import time
#from PIL import Image
#-----------------------------------------------------------------------------
# Globals

CONFLUENCE_PARENT_LINK = "https://confluence.com/pages/viewpage.action?pageId="

# OS PLATFORM, retruns 'Windows', 'Linux' etc.
PLATFORM = platform.system()
MAXIMATOR_EMAIL='user@mail.com'
COMMIT_HISTORY_DIX = getCommitHistory()

def print_if_true(bool, text=None, texts=None):
    if bool:
        if texts == None:
            if text != None:
                print(str(text))
            else:
                raise Exception('text and texts parameters are both none')
        else:
            fulltext = str(texts.pop(0))
            for remtext in texts:
                fulltext += ' '+str(remtext)
            print(fulltext)


def getShortHash(commit_hash, length=None):
    if length is None:
        length = 5
    return str(commit_hash)[0:length]


def find_value_in_table(name, table):
    #finds the specified value for 'name' in table with 2 columns
    for row in table:
        if row[0] == name:
            return str(row[1])
    return None

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

    return changes_made

def getHTMLHeader():
    header = '<head><title></title>'
    header += '<meta name="viewport" content="width=device-width, initial-scale=1">'
    header += '<style>'
    header += 'body {background-color:#ffffff;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}'
    header += 'h1{font-family:Tahoma, sans-serif;font-size:40px;font-style:normal;font-weight:300;color:#000000;background-color:#ffffff;margin:0px;padding:0px;}'
    header += 'h2{font-family:Tahoma, sans-serif;font-size:25px;font-style:normal;font-weight:600;color:#000000;background-color:#ffffff;margin:22px 0px 0px 9px;padding:0px;}'
    header += 'h3{font-family:Consolas ;font-size:22px;font-style:normal;font-weight:600;color:#000000;background-color:#ffffff;margin:22px 0px 0px 9px;padding:0px;}'
    header += 'p {font-family:\'Trebuchet MS\', serif;font-size:20px;font-style:normal;font-weight:normal;color:#000000;background-color:#ffffff;}'
    header += 'p2 {font-family:Consolas;font-size:20px;font-style:normal;font-weight:normal;color:#000000;background-color:#ffffff;margin:0px;padding:0px;}'
    header += 'td{text-align: left;padding: 5px;font-family:\'Trebuchet MS\', serif;font-size:20px;font-style:normal;font-weight:normal;color:#000000;background-color:#ffffff;}'
    header += 'th{text-align: left;padding: 5px;font-family:\'Trebuchet MS\', serif;font-size:20px;font-style:normal;font-weight:600;color:#000000;background-color:#ffffff;}'
    header += 'c1 {color: #349a34}'
    header += 'c2 {color: #087EC9}'
    header += 'c3 {color: #ff0a0f}'
    header += 'c4 {color: #D06D04}'
    header += '.center {display:block; margin-left:auto; margin-right:auto; width:50%;}'
    header += '</style></head>'
    return header

def genHTMLTable(rows, headers=False, data_style='td', header_style='th', autofitting=True):
    #returns the ccode for the 
    htmloutput=''
    th_open = '<'+header_style+'>'
    th_close= '</'+header_style+'>'
    td_open = '<'+data_style+'>'
    td_close= '</'+data_style+'>'

    if autofitting:
        tabletype='<table style="width: 100%">'
    else:
        tabletype='<table class="GeneratedTable">'
    #first do intiial processing
    htmlheader=''
    #now handle headers
    if headers:
        htmlheader += '<thead><tr>'
        header_row = rows[0]
        for header_cell in header_row:
            #each header cell will be surrounded by style
            htmlheader += th_open+header_cell+th_close

        #now close the table header
        htmlheader += '</tr></thead>'

    htmlrows=''
    if (headers and (len(rows)-1 != 0)) or (len(rows) != 0):
        #generate rows
        htmlrows += '<tbody>'
        if headers:
            rows.pop(0)

        for row in rows:
            htmlrows += '<tr>'
            for cell in row:
                htmlrows += td_open+cell+td_close
            htmlrows += '</tr>'
        htmlrows += '</tbody>'

    htmloutput += tabletype+htmlheader+htmlrows+'</table>'
    return htmloutput

def doublequote(astr):
    return '"'+astr+'"'

def bold(astr):
    return styleTextIn(astr, 'b')

def get_project_link(cl, confluence_soup):
    #add the previous job link since theres a previous available
    info_table = parseHTMLTable(get_info_table(cl, confluence_soup))

    project_link = find_value_in_table('Build Link', info_table)
    if project_link is None:
        raise Exception('Could not find project link for '+cl)

    return project_link

def make_commit_hyperlink(cl):
    return make_hyperlink(make_commit_link(cl), text=cl[:7])


def getRowForTest(testname, rows):
    for row in rows:
        if str(row[0]) == testname:
            return row
    return None

def getTestInfoForRow(parsedrow):
    testinfo = {
        'name': parsedrow[0],
        'total': int(parsedrow[1]),
        'passed': int(parsedrow[2]),
        'failed': int(parsedrow[3]),
        'skipped': int(parsedrow[4]),
        'time': int(str(parsedrow[5].split()[0])) #getting the value of seconds
    }

    return testinfo

def getTestInfoForHTMLRow(rowIn):
    #input is in html row
    return getTestInfoForRow(parseHTMLRow(rowIn))

def areTestResultsSame(test1_info, test2_info):

    # The assumption here is that both rows are for the same test
    # so all we need to do is extract the CL# and count how many failures were present
    if test2_info is None:
        raise Exception('isTestResultSame(): test2_info is None')    

    if test1_info['name'] != test2_info['name']:
        raise Exception('isTestResultSame(): Comparing different tests')

    if test1_info['total'] == test2_info['total'] and\
     test1_info['passed'] == test2_info['passed'] and \
     test1_info['failed'] == test2_info['failed'] and \
     test1_info['skipped'] == test2_info['skipped']:
        #the tests are the same result so everything is fine
        #print('SAME')
        return True
    else:
        #the tests are not the same result
        #print('Not the same')
        return False

def areThereFailures(test_info):
    #returns true if there are failures for the current row
    if test_info['total'] != test_info['passed']:
        return True
    else:
        return False

def getReason(test1_info, test2_info):
    #we know there's a difference we want to return if its an improvement or regression
    test2_failures=areThereFailures(test2_info)
    test1_failures=areThereFailures(test1_info)

    cur_total = test1_info['total']
    cur_passed = test1_info['passed']
    prev_total = test2_info['total']
    prev_passed = test2_info['passed']

    if cur_passed > prev_passed:
        if cur_total == cur_passed:
            if prev_total == prev_passed:
                #no failures for current test and previous test
                #we just added new tests
                return 'Tests Addition'
            else:
                #failures for previous cl but not for this one
                #we fixed all the failures
                return 'Fix'
        else:
            return 'Improvement'

    elif cur_passed < prev_passed:
        if cur_total < prev_total:
            if cur_passed == cur_total:
                #less tests in current cl but all the tests passed
                return 'Tests Reduction'
            else:
                #less tests in current cl but there are still failures
                return 'Regression'
        else:
            #regression for every other case
            return 'Regression'
    else:
        return 'Equal'


def compareTableInfo(current_cl, current_table_html, previous_cl, previous_table_html):
    compare_info = {
        'different': False,
        'current_cl': current_cl,
        'previous_cl': previous_cl if previous_cl else None,
        'new_tests': [],
        'missing_tests': [],
        'differing_tests': [],
        'failing_tests': []
    }

    """
    compare_info contains the comparison information between this cl and the last one (if there is a last one)
    Description of keys:
        different:      (boolean)                   if there is any difference between the current results and the last results
        current_cl:     (str)                       the current cl
        previous_cl:    (str)                       the previous cl if it does exist
        new_tests:      (test_info dictionary list) tests that were run on current cl but not on last cl
        missing_tests:  (test name list)            tests that were run on previous cl but not on current cl
        differing_tests (diff_element list)         tests that were run on both cls but have different results
        failing_tests:  (test_info dictionary list) tests on the current cl that had any sort of failures.
    
        diff_element has the following format
        {
            test_name: (str) the name of the test
            diff_reason: (str) the reason for failure, applicable for differing testst (e.g 'Regression', 'Improvement', 'Fix')
            current_results: (test_info dictionary) test info for the current results
            previous_results: (test_info dictionary) test info for the previous results if available
        }

        test_info dictionary is the dictionary returned by getTestInfoForRow()
    """

    #generate the test info
    current_table = parseHTMLTable(str(current_table_html))
    current_table.pop(0) #remove the header
    current_tests = []
    for row in current_table:
        test_info = getTestInfoForRow(row)
        current_tests.append(test_info)

        #check for failing tests if there are present
        if areThereFailures(test_info):
            compare_info['failing_tests'].append(test_info)


    if previous_cl is None or previous_table_html is None:
        #There is no previous stuff to check, so all the tests on the current cl are new
        compare_info['different'] = True
        compare_info['new_tests'] = current_tests
        return compare_info


    #parse the previous table if it does exist
    previous_table = parseHTMLTable(str(previous_table_html))
    previous_table.pop(0) #remove the header
    previous_tests = []
    for row in previous_table:
        p_test = getTestInfoForRow(row)
        previous_tests.append(p_test)

        #also check for missing tests for every test we parse
        #missing tests are tests that were not run on current cl
        if not any( c_test['name'] == p_test['name'] for c_test in current_tests ):
            compare_info['missing_tests'].append(p_test['name'])

    #now go through to get new tests and differing tests
    for c_test in current_tests:
        p_test = None

        #find the previous results for the current test
        for t in previous_tests:
            if t['name'] == c_test['name']:
                p_test = t
                break

        #if no previous was found it means that this is a new test
        if p_test is None:
            compare_info['new_tests'].append( c_test )
            continue
        
        #it could be a differing test
        theSame = areTestResultsSame(c_test, p_test)

        if not theSame:
            compare_info['differing_tests'].append({
                'test_name': c_test['name'],
                'diff_reason': getReason(c_test, p_test),
                'current_results': c_test,
                'previous_results': p_test
                })

    compare_info['different'] = ( len(compare_info['new_tests'])>0  or len(compare_info['missing_tests'])>0 or len(compare_info['differing_tests'])>0 )
    return compare_info

def preProcess(confluence_cls, confluence_soup, limit=None, limit_to_commit=None):

    new_cls = list(confluence_cls)
     #now reverse them so we get them from oldest to newest
    """
    confluence_cls are ordered by latest commit first, i.e.
    [a, b, c, d, e, f, g, h, i, j]
    where a is the latest, b second latest ... j the earliest/oldest
    """
    """
    Let's handle them in the way they will appear on the graph, so reverse it
     ==> [j, i, h, g, f, e, d, c, b, a]
    """
    new_cls.reverse()

    start = 0

    """
    To handle limits, we want to capture latest information
    therefore limit should cover the later end of the list
    so limit=5: [e,d,c,b,a], limit=3: [c,b,a]
    this is done by modifying start of substr
    """
    if limit_to_commit != None:
        limit_commit = str(limit_to_commit)

        matchedcommits = list(filter(
            lambda commit: re.match(limit_commit+'.*', commit),
            new_cls))

        if len(matchedcommits) != 1:
            raise Exception('Limiting commit "'+limit_commit+'" is not in cl list')

        #get the index of the item
        #and calculate limit
        #indx = len - limit
        #limit = len - ind

        limit = len(new_cls) - new_cls.index(matchedcommits[0])

    if limit != None:
        if limit < len(new_cls):
            start = len(new_cls) - limit

    new_cls = new_cls[start:]
    new_tables = []

    #get the parsed results table for each cl
    for cl in new_cls:
        cur_table = parseHTMLTable( get_results_table(cl, confluence_soup) )
        cur_table.pop(0) #remove table header
        new_tables.append(cur_table)

    return new_cls, new_tables

def gen_graph_info(confluence_cls, confluence_soup, verbose=False, limit=None, limit_to_commit=None):
    # diff to print graph based on diffs, not on values
    commits=[]
    test_list = {}

    """
    graph_info = {
        full_commits: []
        tests: {}
    }
    commits are the order of the commits as they would appear on the graph
    tests = {
            <testname>: {
            'commits': [commits that had this test, in order of oldest to newest]
            'total': [<value of total tests for each commit, in order of commits list>]
            'passed': [<value of passed tests>]
            'failed': ['<value of failed tests>']
            'skipped': ['value of skipped tests']
            'time': [<execution time for that commit in seconds>]
            },

            <other test>: {...}

    that is commit at index 0 is aaaba
    then passed, failed, skipped eytc for commit aaaba is passed[0], failed[0], skipped[0] etc
   
    }
    """
    #perform preprocessing to limit and reverse list
    cl_lists, tables = preProcess(confluence_cls, confluence_soup, limit=limit, limit_to_commit=limit_to_commit)
    

    print_if_true(verbose, text='Going through CLs....')
    #these are the keys in our test info dictionary
    value_keys = ['total', 'passed', 'failed', 'skipped', 'time']
    for i in range(0, len(cl_lists)):
        cur_cl = cl_lists[i]
        cur_table = tables[i]

        print_if_true(verbose, texts=['At CL:', str(cur_cl)])
        
        if cur_cl in commits:
            #duplicates should not happen
            raise Exception('This cl "'+cur_cl+'" has been processed')

        #include this cl into the commit list
        commits.append(cur_cl)

        #go through every row in the current table and get the test information
        for cur_row in cur_table:
            cur_info = getTestInfoForRow(cur_row)
            testname = cur_info['name']

            print_if_true(verbose, texts=['\tHandling Test:', str(testname)])
            
            if testname not in test_list.keys():
                print_if_true(verbose, texts=['\tAdding test', testname, 'to dictionary'])
                #this is a new test that hasnt been done before
                #so initialize its entry in the dictionary
                test_list[testname] = {
                    'name': testname,
                    'commits': []
                }
                for key in value_keys:
                    test_list[testname][key] = []

            print_if_true(verbose, texts=['\tAdding info for test', testname, 'to dictionary for', cur_cl])

            #append the found info, the index of the values in their corresponding lists match to each other
            test_list[testname]['commits'].append(cur_cl)

            for key in value_keys:
                test_list[testname][key].append(cur_info[key])

    
    if verbose:
        for test in test_list.keys():
            print_if_true(verbose, texts=['Number of points for', test,'is', str(len(test_list[test]['commits']))])

    #completed going through every cl in the list
    #construct our graph info
    graph_info = {
        'commits': commits,
        'tests' : test_list
    }

    return graph_info


def graph_test(test_info,
                style='regular', limit=None, limit_to_commit=None,
                short_hash=True, relative=False,
                save_graph_to=None, show_graph=False, get_graph_html=False, size=[852,480],
                node=None):
    
    labels = {
        't': "Total Tests",
        'p': "Passed Tests",
        'f': "Failed Tests",
        's': "Skipped Tests"
    }
    colors = {
        't': 'black',
        'p': 'green',
        'f': 'red',
        's': 'orange'
    }
    plot_line = {
        't': True,
        'p': True,
        'f': True,
        's': True
    }

    can_graph=True
    diff = False
    diff_from_total = False
    exec_time = False

    if style != 'regular':
        if style == 'diff_from_total':
            diff_from_total=True
        elif style == 'execution_time':
            exec_time = True
        elif style == 'diff':
            diff = True

    #DATA POINT PREPROCESSING =========================================================================

    testname = test_info['name']
    commits = list(test_info['commits'])
    total = list(test_info['total'])
    passed = list(test_info['passed'])
    failed = list(test_info['failed'])
    skipped = list(test_info['skipped'])
    exec_times = list(test_info['time'])
    data_init = None

    if limit_to_commit != None:
        #print(commits[0])
        limit_commit = str(limit_to_commit)
        matchedcommits = list(filter(
            lambda commit: re.match(limit_commit+'.*', commit),
            commits))
        if len(matchedcommits) != 1:
            raise Exception('Limiting commit "'+limit_commit+'" is not in commit list')
        limit = len(commits) - commits.index(matchedcommits[0])

    if short_hash:
        #process to get short hash
        for i in range(0, len(commits)):
            commits[i] = commits[i][0:5]

    if limit != None:
        if limit < len(commits):
            #commits are ordered oldest to newest
            #we just have to cut lower end of list not in limit
            start = len(commits) - limit
            commits= commits[start:]
            total= total[start:]
            passed= passed[start:]
            failed= failed[start:]
            skipped= skipped[start:]
            exec_times= exec_times[start:]

    if diff:
        """
        For diff, we getting info from how test differ from baseline of oldest commit in graph
        """
        data_init = {
            'total': total[0],
            'passed': passed[0],
            'failed': failed[0],
            'skipped': skipped[0]
        }
        #for each data set, get its diff against the first in the list.
        for i in range(0, len(commits)):
            #recalculation
            total[i] = total[i] - data_init['total']
            passed[i] = passed[i] - data_init['passed']
            failed[i] = failed[i] - data_init['failed']
            skipped[i] = skipped[i] - data_init['skipped']

    #print(str(total))
    if diff_from_total:
        """
        Used for cases when we just want to show how passed tests are differing from total;
        e.g. if total = [10, 10, 10, 10] and passed = [8, 8, 9, 7]
        the graph will plot total as 0
        and passed as [-2, -2, -1, -3]
        People can see if they introduced an error, because passed will go down
        """
        #can only be done if total value is the same for full range
        ref_total = total[len(total)-1]
        not_total = list(filter(
            lambda total_point : total_point != ref_total,
            total))
        if len(not_total) != 0:
            raise Exception('Total Values Differ in Specified Range')
            can_graph=False

        #otherwise, just set the values
        for i in range(0, len(total)):
            passed[i] = passed[i] - ref_total
            total[i] = 0

    #GRAPH ATTRIBUTES PREPROCESSING ======================================================================

    xlabel='Commits (Oldest to Newest)'
    ylabel= 'Number of Tests'
    graph_title= node+': '+testname
    legend_outside=True
    lastcommindx= len(commits) -1

    if relative:
        #we want to show commits by current and closest instead of hash
        commits[lastcommindx] = 'Current'
        commits[lastcommindx-1] = 'Closest'

    #handle the different graph types
    if exec_time:
        output_sets = [ exec_times ]
        output_colors = [colors['t']]
        output_labels = []
        graph_title += ' (Execution Time)'
        ylabel = 'Execution Time (seconds)'

    elif (diff or diff_from_total):
        output_sets = [total, passed]
        output_colors = [colors['t'], colors['p']]
        legend_outside = False

        if diff:
            commits.append('')
            data_sets = [total, passed]
            for data_set in data_sets:
                lastvalue= data_set[len(data_set)-1]
                data_set.append(lastvalue)

            output_labels=[
                'Total (Initially {num})'.format(num=data_init['total']),
                'Passed (Initially {num})'.format(num=data_init['passed'])
            ]
            graph_title += ' (Diff)'

        else:
            output_labels = [
                'Total Tests ({num} Normalized to 0)'.format(num=ref_total),
                'Passed Tests - Total Tests'
            ]
            graph_title += ' (Diff From Total)'

    else:
        output_sets = [total, passed, failed, skipped]
        output_colors = [colors['t'], colors['p'], colors['f'], colors['s']]
        output_labels = [labels['t'],labels['p'],labels['f'],labels['s']]

    #GRAPHING ======================================================================

    if can_graph:
        return graph_data( commits,
                    output_data_sets=output_sets,
                    output_colors=output_colors,
                    output_labels=output_labels,
                    x_axis_label=xlabel, y_axis_label=ylabel,
                    title=graph_title,
                    legend_outside=legend_outside,
                    save_graph_to=save_graph_to, get_graph_html=get_graph_html, show_graph=show_graph,
                    size=size
                )
    else:
        return -1


def graph_data(input_data_set, 
        output_data_sets=[[]], output_labels=[], output_colors=[],
        x_axis_label=None, y_axis_label=None, title=None,
        legend_outside=True, size=[852,480],
        save_graph_to=None, get_graph_html=False, show_graph=False):

    #set the ticks appropriately
    y_ticks = []
    maxvals = []
    minvals = []
    for data_set in output_data_sets:
        maxvals.append(max(data_set))
        minvals.append(min(data_set))

    maxval = max(maxvals)
    minval = min(minvals)

    y_range = abs(maxval-minval)
    if y_range > 10 and y_range <= 30:
        y_ticks = range(minval, maxval, 2)
    elif y_range <= 10:
        y_ticks = range(minval, maxval, 1)

    #plot the data sets
    ax = plt.subplot(111)
    ax.clear()

    for i in range(0, len(output_data_sets)):
        try:
            cur_color = output_colors[i]
        except IndexError:
            cur_color = None

        try:
            cur_label = output_labels[i]
        except IndexError:
            cur_label = None

        if (cur_color is not None) and (cur_label is not None):
            ax.plot(input_data_set, output_data_sets[i], color=cur_color, label=cur_label)
        elif cur_color is not None:
            ax.plot(input_data_set, output_data_sets[i], color=cur_color)
        elif cur_label is not None:
            ax.plot(input_data_set, output_data_sets[i], label=cur_label)
        else:
            ax.plot(input_data_set, output_data_sets[i])

    # naming the x axis
    if x_axis_label is not None:
        plt.xlabel(x_axis_label)
    # naming the y axis
    if y_axis_label is not None:
        plt.ylabel(y_axis_label)


    # giving a title to my graph
    if title is not None:
        plt.title(title)
     
    # turns x ticks 45 degress clockwise
    plt.xticks(rotation = 45)

    #use our own y ticks if they are available
    if y_ticks != []:
        plt.yticks(y_ticks)

    #set grid and 0 margins on x axis
    plt.grid()
    plt.margins(x=0)
    
    #puts legend outside or inside of graph
    if legend_outside:
        ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    else:
        ax.legend()

    #settubg the size of the plot
    px = 1/plt.rcParams['figure.dpi']  # pixel in inches
    figure = plt.gcf()

    figure.set_size_inches(size[0]*px, size[1]*px)

    if save_graph_to is not None:
        plt.savefig(str(save_graph_to), bbox_inches='tight')
        print('Saved Graph to '+str(save_graph_to))

    if show_graph:
        plt.show()
    
    if get_graph_html:
        #print("Ecoding graph to html...")
        start_time=time.time()
        encoded = fig_to_base64(figure)
        graph_html = '<img src="data:image/png;base64, {}" class="center">'.format(encoded.decode('utf-8'))
        #print("Encoded in %s seconds" % (time.time() - start_time))
        return graph_html

    return None

def fig_to_base64(fig):
    img = io.BytesIO()
    fig.savefig(img, format='png',
                bbox_inches='tight')
    img.seek(0)

    return base64.b64encode(img.getvalue())

def image_to_b64_html(filename):
    ext = filename.split('.')[-1]
    prefix = f'data:image/{ext};base64,'
    with open(filename, 'rb') as f:
        img = f.read()
    data = str(prefix + base64.b64encode(img).decode('utf-8'))
    return '<img src="{}">'.format(data)

def resizeImage(file, saveto=None, factor=None, width=None, height=None):

    img = Image.open(file)
    if factor==None:
        if width != None:
            factor = (int(width) / float(img.size[0]))
        elif height != None:
            factor = (int(height) / float(img.size[1]))
        else:
            raise Exception('Must have at least one argument: factor, width, height')

    new_width = int((float(img.size[0]) * float(factor)))
    new_height = int((float(img.size[1]) * float(factor)))
    img = img.resize((new_width, new_height), Image.ANTIALIAS)

    if saveto != None:
        img.save(str(saveto))

    return img

def getGraphHTML(test_info):
    #tries to do diff from total
    try:
        graph_html = graph_test(test_info, style='diff_from_total', relative=True, node=NODE, get_graph_html=True)
    except Exception as e:
        if str(e) == 'Total Values Differ in Specified Range':
            print('Could not graph {test} in diff_from_total, the total tests change in the commit range'.format(test=test_info['name']))
            graph_html = None
        else:
            raise Exception(e)

    if graph_html is None:
        #if diff from total is not available, just do it regularly
        #just do a regular test
        graph_html = graph_test(test_info, style='regular', relative=True, node=NODE, get_graph_html=True)
        if graph_html is None:
            #if thats not available either then we just return not available
            return styleTextIn('Graph Not Available','p')
        else:
            return graph_html
    else:
        return graph_html

def gen_general_info_html(compareInfo, hash_info, previousAvail):
    html = ''
    htmlnewl='<p> </p>'
    #add title
    html += styleTextIn('Initial Info', 'h2')

    current_cl = compareInfo['current_cl']
    #if the commit is not in the hash info table then no information will be available
    try:
        curAuthName =  hash_info[current_cl]['author_name']
        comment =      hash_info[current_cl]['subject']
        curAuthEmail = hash_info[current_cl]['author_email']
    except KeyError:
        curAuthName =  'Not Available'
        comment =      'Not Available'
        curAuthEmail = 'Not Available'
    
    #add the current table  to the html
    html += genHTMLTable([
            [ bold('Current Commit:'), bold(make_hyperlink(make_commit_link(current_cl), text=current_cl))],
            [ 'Author:',               curAuthName],
            [ 'Email:',                curAuthEmail],
            [ 'Jenkins Job:',          make_hyperlink(compareInfo['current_job_link'])]
    ])
    #add the current comment
    html += styleTextIn("Comment: "+doublequote(comment), 'p2')
    html += htmlnewl

    if previousAvail:
        #if there is a previous build, then add the information for that as well
        previous_cl = compareInfo['previous_cl']
        try:
            prevAuthName =   hash_info[previous_cl]['author_name']
            prevAuthEmail =  hash_info[previous_cl]['author_email']
            prevcomment =    hash_info[previous_cl]['subject']
        except KeyError:
            prevAuthName =  'Not Available'
            prevcomment =   'Not Available'
            prevAuthEmail = 'Not Available'

        #add the table to the html
        html += genHTMLTable([
            [ bold('Previous Commit:'), bold(make_hyperlink(make_commit_link(previous_cl), text=previous_cl))],
            [ 'Author:',                prevAuthName],
            [ 'Email:',                 prevAuthEmail],
            [ 'Jenkins Job:',           make_hyperlink(compareInfo['previous_job_link'])]
        ])
        #add previous comment
        html += styleTextIn("Comment: "+doublequote(prevcomment), 'p2')
        html += htmlnewl

    #also add a link to its confluence page
    results_link = make_hyperlink('{}{}'.format(CONFLUENCE_PARENT_LINK, get_confl_pageid(NODE)), text=get_confl_pagetitle(NODE))
    html += styleTextIn('Test History: '+results_link, 'h3')

    return html

def gen_commit_history_html(compareInfo, previousAvail):
    history_text=''
    if previousAvail and compareInfo['history_searched']:
        max_commits_shown = 15
        cls_inbtwn=compareInfo['cls_inbetween']
        clcount=len(cls_inbtwn)
        if len(cls_inbtwn) > 0:
            #print at most 10 and then say how many more
            history_text = make_commit_hyperlink(cls_inbtwn[0])
            for i in range(1, min(clcount, max_commits_shown)):
                history_text = history_text+', '+make_commit_hyperlink(cls_inbtwn[i])

            if clcount > max_commits_shown:
                history_text += ' and {num} more.'.format(num=(clcount - max_commits_shown))

        else:
            history_text = 'None (They are consecutive)'
    else:
        history_text = 'Unavailable'


    return styleTextIn('Commits In-Between: {h}'.format(h=history_text), 'h3')

def gen_new_tests_html(new_tests):
    if len(new_tests) == 0:
        return ''

    html =  styleTextIn('New Tests', 'h2')

    for test in new_tests:
        #just write the name and the results
        html += styleTextIn(test['name'], 'h3')
        html += genHTMLTable([
                                ['Total', 'Passed', 'Failed', 'Skipped'],
                                [str(test['total']), str(test['passed']), str(test['failed']), str(test['skipped'])]
                            ], headers=True)
        html += genHTMLTable([ [' ', ' ']])

    return html

def genDiffResultsTableForTest(diff_element):
    test_results_header=['Commit Type', 'Total', 'Passed', 'Failed', 'Skipped']

    diff_reason_html_styles = {
        'Fixed':'c1',
        'Improvement': 'c2',
        'Regression' : 'c3'
    }

    diff_reason = diff_element['diff_reason']
    current = diff_element['current_results']
    previous = diff_element['previous_results']

    #first geenrate the test_info_table
    #generate raw text first
    reasonhtml='Differing Results ('+diff_reason+')'

    #use diff reason to determine what style it should be
    if diff_reason in diff_reason_html_styles.keys():
        #if the reason is in the keys then style with appropriate colour
        reasonhtml= styleTextIn(reasonhtml, diff_reason_html_styles[diff_reason])

    test_info_table = [
        ['Diff Type:', bold(reasonhtml)]
    ]


    test_results_table = [
        ['Commit', 'Total', 'Passed', 'Failed', 'Skipped']
    ]

    current_html = ['current',str(current['total']), str(current['passed']), str(current['failed']), str(current['skipped'])]
    previous_html = ['previous',str(previous['total']), str(previous['passed']), str(previous['failed']), str(previous['skipped'])]

    #compare values to determine if they should be highlighted
    for i in range(1, len(current_html)):
        if current_html[i] != previous_html[i]:
            #if they are not the same value then highlight it
            current_html[i] = styleTextIn(current_html[i], 'c4')

    #now append new rows
    test_results_table.append(current_html)
    test_results_table.append(previous_html)

    return test_info_table, test_results_table

def gen_differing_tests_html(differing_tests, graphed_tests, graphInfo):
    if len(differing_tests) == 0:
        return ''

    html = styleTextIn('Differing Tests', 'h2')

    for diff_element in differing_tests:
        test_name = diff_element['test_name']

        test_info_table, test_results_table = genDiffResultsTableForTest(diff_element)

        #graph the current test if not already done
        if test_name not in graphed_tests:
            graph_html = getGraphHTML(graphInfo['tests'][test_name])
            graph_table = [
                [bold('Graph'), graph_html]
            ]
            graphed_tests.append(test_name)

        #add the html for the test 
        html += styleTextIn('Test: '+str(test_name), 'h3')
        html += genHTMLTable(test_info_table)
        html += genHTMLTable(graph_table)
        html += genHTMLTable(test_results_table, headers=True)
        html += genHTMLTable([ [' ', ' ']])
    
    return html

def gen_missing_tests_html( missing_tests ):
    if len(missing_tests) == 0:
        return ''

    html = styleTextIn('Missing Tests', 'h2')
    for test in missing_tests:
        html += styleTextIn(test, 'h3')

    return html


def gen_failing_tests_html(failing_tests, new_tests, differing_tests, graphed_tests , graphInfo):
    if len(failing_tests) < 1:
        #there are no failing tests so no html
        return ''

    failing_tests_html = ''
    failed_tests_to_graph = []
    cur_failure_html = ''

    for failed_test in failing_tests:
        test_name = failed_test['name']
        total = str(failed_test['total'])
        passed = str(failed_test['passed'])
        failed = str(failed_test['failed'])
        skipped = str(failed_test['skipped'])
        error_desc = 'Stable'
        #check if this is a new failure or not

        #new failures would be in the new tests list
        if any( n_test['name'] == test_name for n_test in new_tests):
            error_desc = 'Fresh'
        else:
            for diff_elem in differing_tests:
                if diff_elem['test_name'] == test_name:
                    #this is a differing test, so just use the diff reason
                    error_desc = diff_elem['diff_reason']
                    break
        
        cur_failure_html = ''
        #test name and the type of error first
        cur_failure_html += styleTextIn('Test: '+str(test_name)+' ('+error_desc+')', 'h3')

        #do not regraph if the test has already been graphed before
        if test_name not in graphed_tests:
            cur_failure_html += genHTMLTable([
                                [bold('Graph'), getGraphHTML(graphInfo['tests'][test_name])]
                            ])

        #also show the current results
        test_results_table = [
            [ 'Total', 'Passed', 'Failed', 'Skipped'],
            [  total,   passed,   failed,   skipped ]
        ]
        cur_failure_html += genHTMLTable(test_results_table, headers=True)
        cur_failure_html += genHTMLTable([ [' ', ' ']])

        if test_name in graphed_tests:
            #if test has already been graphed put it at the top of the list of failures
            failing_tests_html = cur_failure_html + failing_tests_html
        else:
            failing_tests_html += cur_failure_html
            graphed_tests.append(test_name)

    return styleTextIn('Failing Tests', 'h2') + failing_tests_html

def handleDifferences(compareInfo, commit_history, graphInfo):
    #check for previous info
    print('\nGenerating Email HTML...')
    html=''
    htmlnewl='<p> </p>'

    graphed_tests=[]
    is_different = compareInfo['different']
    new_tests = compareInfo['new_tests']
    missing_tests = compareInfo['missing_tests']
    differing_tests = compareInfo['differing_tests']
    failing_tests = compareInfo['failing_tests']

    current_cl = compareInfo['current_cl']
    previous_cl = compareInfo['previous_cl']

    #was there a previous cl
    previousAvail = ( previous_cl is not None )
    #are there failing tests
    failAvail = ( len(failing_tests) > 0 )

    #set the main header of the html with the subject
    context = ''
    if is_different and failAvail:
        context = 'Diffs and Failures'
    elif is_different:
        context = 'Diffs'
    elif failAvail:
        context = 'Failures'
    else:
        context = 'Results'
   
    #the html subject
    html_subject =  styleTextIn("{} on '{}'".format(context,NODE), 'h1')

    #add the initial info
    print('Generating HTML for Initial Info....')
    gen_info_html = gen_general_info_html( compareInfo, commit_history['hashinfo'], previousAvail )
    
    #commits in between 
    print('Generating HTML for Commit History...')
    history_html = gen_commit_history_html( compareInfo, previousAvail )

    #commits in between 
    print('Generating HTML for New Tests...')
    new_tests_html = gen_new_tests_html( new_tests )
    
    #html for differing tests
    print('Generating HTML for Differing Tests...')
    differing_tests_html = gen_differing_tests_html( differing_tests, graphed_tests, graphInfo )

    #html for missing tests
    print('Generating HTML for Missing Tests...')
    missing_tests_html = gen_missing_tests_html( missing_tests )

    #html for failing tests
    print('Generating HTML for Failures...')
    failing_tests_html = gen_failing_tests_html( failing_tests, new_tests, differing_tests, graphed_tests, graphInfo )


    bodyhtml = '<body>{sub}{info}{hist}{new}{diffs}{missing}{fails}</body>'.format( sub     =   html_subject,
                                                                                    info    =  gen_info_html,
                                                                                    hist    =  history_html,
                                                                                    new     = new_tests_html,
                                                                                    diffs   = differing_tests_html,
                                                                                    missing = missing_tests_html,
                                                                                    fails   = failing_tests_html
                                                                            )

    fullhtml = '<!DOCTYPE html><html>{header}{body}</html>'.format(header=getHTMLHeader(), body=bodyhtml)

    
    print('Configuring Email Parameters...')
    node_regression = '({} > {})'.format(getShortHash(previous_cl, length=7), getShortHash(current_cl, length=7)) if previousAvail else getShortHash(current_cl, length=7)
    email_subject = 'TestDiffWatcher - {} - {} - {}'.format(context, NODE, node_regression)

    main_recipient = None
    #default recipient is specified by recipeints argument
    if len(RECIPIENTS) > 0:
        main_recipient = RECIPIENTS[0]

    if is_different and (current_cl in commit_history['hashinfo']):
        #only send email if there are changes in the results
        if not TESTING:
            main_recipient = commit_history['hashinfo'][current_cl]['author_email']
        else:
            #testing mode does not send email to commit author
            print('========> MAIN RECIPIENT HAS BEEN CHANGED TO {} SINCE IN TESTING <======'.format(main_recipient))

    attachments = []

    send_email = (not TESTING) or (TESTING and SEND_EMAIL)

    if send_email:
        if main_recipient is not None:
            try:
                print('Sending Email to {}...'.format(main_recipient))
                send_email_from_farm( fullhtml, email_subject, main_recipient, RECIPIENTS, content="html", files=attachments)
                print('Email Sent!')
            except Exception as e:
                raise Exception('Error sending email:\nsend_text(): \n'+str(e))
        else:
            raise Exception('No recipient specified! Use -recipients argument')
    else:
        print('Email Not Sent')

    if (SAVE_HTML != ''):
        with open(SAVE_HTML, 'w') as file:
            file.write(fullhtml)
        print('=================> HTML saved to '+SAVE_HTML+' <===========================')
    else:
        print("Run script with arguments '--testing -save_html <file_name>' to save generated HTML as 'file_name'")

def findDifferences(latest_cl, latest_link, table, confluence_cls, confluence_soup):
    #use the commit history to, to find the commit before
    commit_history = COMMIT_HISTORY_DIX

    hash_list = commit_history['hashes']
    hash_info = commit_history['hashinfo']

    
    prevCommitFound=False

    #cls/commits between the first cl (latest_cl) and the second cl on the confluence page
    cls_inbetween=[]
    foundHash=''
    historySearched=False

    if latest_cl in hash_list:
        #we can find the commit history, if the current cl is in the list of hashes
        print('Searching through history...')

        #get the index of the current hash in list
        index = hash_list.index(latest_cl)


        #the first search begins at the last commit
        #list is ordered from newest to oldest, so to find previous commits we ascend in the list
        curSearchIndx=index+1
        curhash=''

        while (not prevCommitFound) and (curSearchIndx < len(hash_list)):
            #get the curhash for this index
            curhash = hash_list[curSearchIndx]
            
            #check if the curhash is in the CL list
            if curhash in confluence_cls:
                #then we have found one that was run
                foundHash = curhash
                prevCommitFound = True
                print('Closest Commit On Confluence and in Commit History: '+str(foundHash))
            else:
                #this is not the one we are looking for so add it to clsinbetween
                cls_inbetween.append(curhash)

            curSearchIndx += 1

        print('Search Complete')
        historySearched=True

    else:
        #just get the last cl from the confluence page
        print('Commit "'+latest_cl+'" was not in history, comparing to last confluence output')
        try:
            foundHash = confluence_cls[1]
            prevCommitFound = True
        except IndexError:
            print('No last confluence output')
            prevCommitFound = False


    handle_diffs=False

    if prevCommitFound:
        #if we were able to find a commit, compare the tests output,
        #get the test info and compare them
        foundHashIndx = confluence_cls.index(foundHash)
        
        found_hash_table = get_results_table(foundHash, confluence_soup)

        #if there is any difference, then retrieve the info and send it out
        compareInfo = compareTableInfo(latest_cl, table, foundHash, found_hash_table)

        if compareInfo['different'] or (len(compareInfo['failing_tests']) != 0):
            #differences or failures should be handled
            handle_diffs=True

        found_hash_link = get_project_link(foundHash, confluence_soup)
        compareInfo['previous_job_link'] = str(found_hash_link)

    else:
        #if we are not able to find a commit, then handle this as a fresh commit
        #ie no history for this noded
        compareInfo = compareTableInfo(latest_cl, table, None, None)
        handle_diffs=True
        
    if handle_diffs:
        compareInfo['current_job_link'] = str(latest_link)
        compareInfo['cls_inbetween'] = cls_inbetween
        compareInfo['history_searched'] = historySearched
        print('Generating graph info...') 
        graph_info = gen_graph_info(confluence_cls, confluence_soup, verbose=False, limit=30)
        print('Parsing Differences')
        handleDifferences(compareInfo, commit_history, graph_info)
        return 1
    else:
        print(NODE+' continues to pass all tests')
        return 0

def commitDiff(auth, pageid):
    # Use Beautiful Soup to parse the html and extract the information in the tables
    print('Getting Confluence Soup...')
    confluence_soup = get_confluence_soup(auth, pageid)
    
    confluence_cls = get_cls_from_soup(confluence_soup)

    latest_cl = confluence_cls[0]
    
    latestResultTable = get_results_table(latest_cl, confluence_soup)
    link = get_project_link(latest_cl, confluence_soup)

    print('QUERYING COMMIT: "'+latest_cl+'" for "'+NODE+'"')
    ret = findDifferences(latest_cl, link, latestResultTable, confluence_cls, confluence_soup)
    #writing the confluence soup

    return ret
    

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
        "-runner",
        required=False,
        type = str,
        metavar='<master or fileshare>',
        help = "The node running this script"
    )

    parser.add_argument(
        "-recipients",
        required=False,
        type=str,
        metavar='<password>',
        help='Email recipients for TestWatcher, space separated'
    )

    #arguments used for testing purposes
    parser.add_argument('--testing', '--testing', action='store_true', help='Testing the script, disables email sending')
    parser.add_argument('--send_email', '--send_email', action='store_true', help='Used with --testing, enables email sending to only the specified recipients')
    parser.add_argument(
        "-save_html",
        required=False,
        type=str,
        metavar='<file address>',
        help='Save the generated HTML string to the specified file'
    )

    options = parser.parse_args()


    global TESTING
    global SEND_EMAIL
    global SAVE_HTML

    global NODE #the node name
    global RECIPIENTS #recipients for the email

    TESTING = options.testing
    SEND_EMAIL = options.send_email
    SAVE_HTML=''

    if options.save_html != None:
        SAVE_HTML=str(options.save_html)
        print('Generated HTML will be saved to {}'.format(SAVE_HTML))

    if TESTING:
        print('TESTING MODE')
        print('EMAIL SENDING: {}'.format('ENABLED' if SEND_EMAIL else 'DISABLED'))
    
    NODE = str(options.node)

    try:
        auth = get_login(USER_NAME, None)
    except Exception as e:
        print('main(): Could not get login credentials from node for confluence:')
        print('get_login(): '+str(e))
        return 2

    RECIPIENTS = []
    if options.recipients is not None:
        RECIPIENTS = str(options.recipients).split()

    #Find the html file name in the _results folder
    try:
        return commitDiff(auth, get_confl_pageid(NODE))
    except:
        print("main(): Error Occured:")
        traceback.print_exc()
        return 2

if __name__ == "__main__":
    sys.exit(main())
