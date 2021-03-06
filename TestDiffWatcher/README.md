# TestDiff Watcher
## Overview

These set of scripts were designed to compare test results for a given machine and email a certain set of code managers if any differences were detected including:

- New test failures
- Improved results or fixes
- New tests added
- Tests removed

The main script `scripts/test_diff.py` queries the confluence database of results and compares the latest two results (the current results and the previous results). If any differences are found, it will email the commit author as well as the testing framework managers.

The email sent is HTML generated by the script which contains commit information for the two test results (author, date posted, link to commit) and the identified differences (visualized with graphs when appropriate).


## Testing the script
To test the script, you will need to be given credentials for the script to access confluence pages and send emails. Please email imaduabuchi02@gmail.com for access.

You will also need to have python installed and install the following modules:
```
pip install requests
pip install keyring
pip install tabulate
pip install names
pip install matplotlib
```

When you have your credentials, run `public_testing.py` to set the Confluence and Email Credentials.

You can also use `public_testing.py` to view the test results page of the sample node Win10_Navi21, generate sample results and post sample results to confluence. This is not necessary for testing `test_diff.py` since Win10_Navi21 is already configured with sample test results.

**You can test the script with the following:**
Save the generated HTML to a file to view:
```
test_diff.py -node Win10_Navi21 -save_html file_address
```

Send the HTML in an email
```
test_diff.py -node Win10_Navi21 -emailto yourname@address.com
```



