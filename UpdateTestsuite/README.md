# UpdateTestsuite
## Overview

The script `update_testsuite.py` was designed to sync the test framework folder on the set of test machines.

The script is designed to work with a server and node organization. That is, the servers are the machines that have the latest version of the test folder and the nodes are the machines that need to be synced to this latest version.

The script supports ignoring certain file extensions, files and/or directories and is built to work on both Windows and Linux systems.

## More Information

The pipeline is as follows:

The script is run on the server machine with the following parameters:
```sh
update_testsuite.py -role server
```
This triggers the server to update their local git repository of the test folder and index the files in the test folder. The files are indexed by storing the file name and MD5 Hash of it in a python hash map. For example:

```python
{
	'/a/path/to/some/file': 'h9ay8c89ayh9ce9y981hd98qy67132'
}
```
This hash map is stored in a log file that is accessible to both servers and nodes.

Once this is complete, the script is run on the node end with the following parameters:
```sh
update_testsuite.py -role node -servercreds "user pass"
```
The servercreds parameter is used as credentials by the node for accessing files on the server.

The nodes first get the file log from the server. Then they generate their own log using the same method as the server. The server file index and the node file index are then compared to determine what is different.

Keys (files) present on the node but not on the server indicate that the file must be deleted. Files that are present on the server but not on the node, or files with different MD5 hashes will be copied from the server to the node.

If the node does not have any copy of the test folder, a directory clone will be used instead of a file by file copy. This is done using robocopy on Windows and rsync on Linux.

Files/directories can be ignored since the comparison is done using the generated file index. Therefore, if the file or directory is not indexed in both the server and node file log then it will not be synced. Ignoring files is useful for cache files or result files that are specific to the node.