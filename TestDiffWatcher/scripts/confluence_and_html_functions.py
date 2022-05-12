import argparse
import json
import keyring
import requests
import re
import ast
import platform

from bs4 import BeautifulSoup
from shutil import copyfile

'''
GENERAL FILE THAT CONTAINS ALL FUNCTIONS USED FOR CONFLUENCE RELATED ACTIONS
USED BY POST_TO_CONFLUENCE(AND LUXSDK), TEST_DIFF and POST_TEST_EMAIL LUXSDK
'''


# Confluence REST API needs to know the title and pageid for a given page.
# This map links a given node to its respective title and pageid on confluence
# format: <Node Name>: [ <confluence page title, usually machine name>, <confluence page id> ]
#get node information using get_confl_pagetitle() and get_confl_pageid() functions
TESTNODE_CONFLUENCE_PAGES = {
            "Win10_Navi10" : ["AMF-WIN10-NAVI10", 327418887],
}

BASE_URL = "https://confluence.com/rest/api/content"
USER_NAME = "user"

def pprint(data):
    '''
    Pretty prints json data.
    '''
    print( json.dumps(
        data,
        sort_keys = True,
        indent = 4,
        separators = (', ', ' : ')))


def get_page_ancestors(auth, pageid):

    # Get basic page information plus the ancestors property

    url = '{base}/{pageid}?expand=ancestors'.format(
        base = BASE_URL,
        pageid = pageid)

    r = requests.get(url, auth = auth)

    r.raise_for_status()

    return r.json()['ancestors']


def get_page_info(auth, pageid):

    url = '{base}/{pageid}'.format(
        base = BASE_URL,
        pageid = pageid)

    r = requests.get(url, auth = auth)

    r.raise_for_status()

    return r.json()


def get_page_data(auth, pageid):
    url = '{base}/{pageid}?expand=body.storage'.format(
        base = BASE_URL,
        pageid = pageid
    )
    r = requests.get(url, auth = auth)

    r.raise_for_status()
    
    return r.json()

def get_confl_pagetitle(node):
    if node not in TESTNODE_CONFLUENCE_PAGES.keys():
        raise Exception('{} not in list of confluence pages, please add to TESTNODE_CONFLUENCE_PAGES'.format(node))

    return TESTNODE_CONFLUENCE_PAGES[node][0]

def get_confl_pageid(node):
    if node not in TESTNODE_CONFLUENCE_PAGES.keys():
        raise Exception('{} not in list of confluence pages, please add to TESTNODE_CONFLUENCE_PAGES'.format(node))

    return TESTNODE_CONFLUENCE_PAGES[node][1]


def get_login(username, passwd=None):
    '''
    Looks in the keyring (credentials manager on Windows) for an already
    exisiting entry. If the given username does not exist in the keyring,
    it will prompt a password from the commandline. For this to work properly with
    Jenkins, the credentials need to manually added for each node in the keyring
    for Jenkins' username
    Get the password for username out of the keyring.
    '''
    if passwd is not None:
        return (username, passwd)

    passwd = keyring.get_password('confluence_script', username)

    if passwd is None:
        raise Exception('Password could not be retrieved from keyring')

    return (username, passwd)

def parse_html(htmlIn):
    return BeautifulSoup(str(htmlIn), 'html.parser')

def get_raw_confluence_soup(auth, pageid):
    current_confluence_html = get_page_data(auth, pageid)["body"]["storage"]["value"]
    return parse_html(current_confluence_html)

def get_confluence_soup(auth, pageid):
    #auth: authentication tuple (username, password)
    #pageid: pageid on amd confluence page
    #get the raw html soup
    confluence_soup = get_raw_confluence_soup(auth, pageid)
    #populate the cl ids on the HTML tags for easy access
    populate_cl_ids(confluence_soup)
    return confluence_soup


def write_confluence_soup(confluence_soup, auth, pageid, title):
    new_confluence_html = confluence_soup

    #write the combined html to the confluence page
    confluence_info = get_page_info(auth, pageid)
    ver = int(confluence_info['version']['number']) + 1

    ancestors = get_page_ancestors(auth, pageid)

    anc = ancestors[-1]
    del anc['_links']
    del anc['_expandable']
    del anc['extensions']

    if title is not None:
        confluence_info['title'] = title

    data = {
        'id' : str(pageid),
        'type' : 'page',
        'title' : confluence_info['title'],
        'version' : {'number' : ver},
        'ancestors' : [anc],
        'body'  : {
            'storage' :
            {
                'representation' : 'storage',
                'value' : str(new_confluence_html),
            }
        }
    }

    data = json.dumps(data)

    url = '{base}/{pageid}'.format(base = BASE_URL, pageid = pageid)

    #print('Writing to Confluence Server...')
    r = requests.put(
        url,
        data = data,
        auth = auth,
        headers = { 'Content-Type' : 'application/json' }
    )

    r.raise_for_status()

    print ("Wrote '%s' version %d" % (confluence_info['title'], ver))

def stripToHash(cl_header_html):
      return cl_header_html.string.replace('CL#', '')

def populate_cl_ids(confluence_soup):
    #takes the soup and populates the components with the appropriate ids
    #this is a temporary work around until we figure out how to add ids 

    """
    currently confluence page is set up in the following way:
    CL header
    info_table
    results_table

    Where all CL headers are the only thing on page with h3 style
    We can therefore use this h3 style as a finder, and then id all the content on the page from there
    """
    page_contents = confluence_soup.contents
    cl_headers = confluence_soup.find_all('h3')
    for cl_header in cl_headers:
        """
        if cl_header is at index i
        info_table is at index i+1
        results_table is at index i+2
        therefore use indexing for the id
        """
        indx = page_contents.index(cl_header)
        cur_info_table = page_contents[indx+1]
        cur_results_table = page_contents[indx+2]

        #now set the ids appropriately for each object
        cur_cl = stripToHash(cl_header)
        cl_header['id'] = cur_cl+'_cl_header'
        cur_info_table['id'] = cur_cl+'_info_table'
        cur_results_table['id'] = cur_cl+'_results_table'

def remove_all_tags_for_cl(cl, confluence_soup):

    def soup_part_of_commit(html_id):
        #returns title that match the regex expression
        regex_in = HTML_SEARCH_COMMIT+'.*'
        return html_id and re.compile(regex_in).search(html_id)

    #first find all the matching objects
    HTML_SEARCH_COMMIT = cl
    associated_tags = confluence_soup.find_all(id=soup_part_of_commit)
    for tag in associated_tags:
        tag.extract()

def get_cls_from_soup(confluence_soup, ommit=[], unique=True, limit=None):
    #read the list of headers from the page
    #for each extract the commit hash

    cl_list = []

    def is_commit_header(html_id):
        return html_id and re.compile(".*_cl_header").search(html_id)

    for cl_html in confluence_soup.find_all(id=is_commit_header):
        cur_cl = stripToHash(cl_html)
        if (cur_cl in ommit) or (unique and (cur_cl in cl_list)):
            #if the cur_cl should be ommitted
            #or we have unique set and the cur cl is already in cl_list
            #then remove all tags for this cl
            remove_all_tags_for_cl(cur_cl, confluence_soup)
        else:
            cl_list.append(cur_cl)

    if limit is not None:
        #finally limit the cl_list to max entries if needed
        if len(cl_list) > (limit):
            cls_to_remove = cl_list[limit:]
            cl_list = cl_list[:limit]

            for cl in cls_to_remove:
                print('Removing CL: {cl}'.format(cl=cl))
                remove_all_tags_for_cl(cl, confluence_soup)

    return cl_list

def get_results_table(cl, confluence_soup):
    return confluence_soup.find_all(attrs={'id':cl+'_results_table'})[0]

def get_info_table(cl, confluence_soup):
    return confluence_soup.find_all(attrs={'id':cl+'_info_table'})[0]

def parseHTMLRow(rowIn):
    parsed_row = []
    #try to parse as header row
    row_cells = BeautifulSoup(str(rowIn), 'html.parser').find_all(['td','th'])

    if len(row_cells) != 0:
        #do full parse
        for cell in row_cells:
            parsed_row.append(cell.get_text())
        return parsed_row
    else:
        raise Exception('Could not find header or data row for html: '+str(rowIn))


def parseHTMLTable(tableIn):
    #returns table in list format
    """
    table [
        [row1],
        [row2],
        [row3]
    ]
    """

    parsedtable = []

    #parse the table using beautiful soup
    html_rows = BeautifulSoup(str(tableIn), "html.parser").find_all('tr')

    for html_row in html_rows:
        parsedrow = parseHTMLRow(html_row)
        #add it to the parsedtable
        parsedtable.append(parsedrow)

    return parsedtable

def get_row_indx(keys, table_soup):
    #returns the index of the row that matches keys in table soup
    row_indx = 0
    for row_soup in table_soup.tbody.find_all('tr'):
        #print(row_soup)
        i = 0
        is_match = True
        elements = row_soup.find_all(['td', 'th'])
        #print(elements)

        while( i < len(keys) and is_match):
            #compare the current element on the row to the value in the key
            #since we are matching by default, set match false if one of the elements dont match the key
            if (keys[i] is not None) and (elements[i].string != str(keys[i])):
                #print('({ind}): False match for element {el} with key {k}'.format(k=str(keys[i]), el=elements[i].string, ind=i))
                is_match = False
            i = i+1

        if is_match:
            #if the current row is a match, then return the value(s) on that row
            return row_indx

        row_indx = row_indx + 1
    return -1

def get_values_from_table_soup(keys, table_soup):
    """
    If a given row in the table soup, matches [keys], then it will return the remaining elements
    If one of the keys is None, it will match to anything
    after keys in a list
    Will get the first match
    """
    matching_indx = get_row_indx(keys, table_soup)

    if matching_indx == -1:
        return []

    elements = table_soup.tbody.find_all('tr')[matching_indx].find_all(['td', 'th'])
    values = []
    for i in range(len(keys), len(elements)):
        values.append(elements[i].string)

    return values

def set_values_in_table_soup(keys, values, table_soup):
    """
    if a row matches [keys] then it will set values to the elements on that row after the keys
    values will only overwrite as much as it has 
    """
    matching_indx = get_row_indx(keys, table_soup)

    if matching_indx == -1:
        #print('here!!')
        return False

    elements = table_soup.tbody.find_all('tr')[matching_indx].find_all(['td', 'th'])

    for i in range(len(keys), len(elements)):
        elements[i].string = values[ i-len(keys) ]
    return True

def make_hyperlink(link, text=None):
    if text is None:
        text = link
    return '<a href="{link}">{disp}</a>'.format(link=link, disp=text)

def make_html_row(row, header=False, side_header=False):
    html = '<tr>'

    if side_header:
        #header values on the side rather than on the top, so only first element will get th formatting
        html += '<th>{value}</th>'.format(value=row.pop(0))

    tag = 'td'
    if header:
        tag = 'th'

    for item in row:
        html += '<{tag}>{value}</{tag}>'.format(tag=tag, value=str(item))

    html += '</tr>'
    return html

def make_html_table(rows, header=False, side_header=False):
    #returns row html
    html = '<table><tbody>'

    for row in rows:
        html += make_html_row(row, header=header, side_header=side_header)

    html += '</tbody></table>'

    return html

def make_html_row_soup(row, header=False, side_header=False):
    return parse_html(make_html_row(row,header,side_header)).contents[0]

def make_html_table_soup(rows, header=False, side_header=False):
    return parse_html(make_html_table(rows,header,side_header)).contents[0]

def get_multiple_row_indxs(table_soup, keys=None, keyset=None):
    #returns the list of row indices in the table soup that match the keys
    matching_indxs = []
    row_indx = 0

    #keyset will always override keys
    if (keyset is None) and (keys is None):
        raise Exception('No keys to match row soups against')

    if keyset is None:
        #no keyset specified, but keys specified
        keyset = []
        keyset.append(keys)

    for row_soup in table_soup.tbody.find_all('tr'):
        for keys in keyset:
            i = 0
            is_match = True
            elements = row_soup.find_all(['td', 'th'])
            #print(elements)

            while( i < len(keys) and is_match):
                #compare the current element on the row to the value in the key
                #since we are matching by default, set match false if one of the elements dont match the key
                if (keys[i] is not None) and (elements[i].string != str(keys[i])):
                    #print('({ind}): False match for element {el} with key {k}'.format(k=str(keys[i]), el=elements[i].string, ind=i))
                    is_match = False
                i = i+1

            if is_match:
                #if the current row is a match, then return the value(s) on that row
                matching_indxs.append(row_indx)

        row_indx = row_indx + 1
    return matching_indxs

def get_matching_rows_from_table_soup( table_soup, keys=None, keyset=None):
    row_indxs = get_multiple_row_indxs(table_soup, keys=keys, keyset=keyset)
    matching_row_soups = []
    for row_indx in row_indxs:
        matching_row_soups.append(table_soup.tbody.find_all('tr')[row_indx])

    return matching_row_soups

def styleTextIn(text, marker):
    return '<'+marker+'>'+text+'</'+marker+'>'

def make_html_color_cell(value, header=False, color=None, condition=False):
    #will wrap value in row cell html
    #if condition is true, it will add the specified color
    tag = 'td'

    if header:
        tag = 'th'

    bg_color = ''
    if (color is not None) and condition:
        bg_color = ' bgcolor="{color}"'.format(color=color)

    return '<{tag}{an_attr}>{value}</{tag}>'.format(an_attr=bg_color, value=value, tag=tag)

def make_commit_link(cl):
    return 'https://github.com/some/repository/commit/{cl}'.format(cl=cl)

def main():
    return

if __name__ == "__main__":
    sys.exit(main())