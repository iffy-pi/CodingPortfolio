import sys
import os
import subprocess
import csv
import threading

def run_c_parser( input_file_addr, output_file_addr ):
	with open(output_file_addr, 'w') as output_file:
		child = subprocess.Popen(['testparser.out', input_file_addr], stdout=output_file, cwd=os.getcwd())
		child.wait()
		child.kill()

def get_row_string(row):
	row_str = None
	for cell in row:
		if not row_str:
			row_str = '["{}"'.format(cell)
		else:
			row_str += ', "{}"'.format(cell)

	row_str += ']'

	return row_str

def run_python_parser(input_file_addr, output_file_addr):
	with open( input_file_addr, 'r' ) as csv_file:
		python_csv_reader = csv.reader(csv_file, delimiter=',', quotechar='"')

		table_str = None
		for row in python_csv_reader:
			if not table_str:
				table_str = '[{}'.format(get_row_string(row))
			else:
				table_str += ', {}'.format(get_row_string(row))

		table_str += ']'

		with open(output_file_addr, 'w' ) as output_file:
			output_file.write('{}\n'.format(table_str))

def make_output_file_name(desc, is_c=False, is_python=False):
	if is_c:
		return os.path.join( "results", "{}_c_parser.txt".format(desc))

	elif is_python:
		return os.path.join( "results", "{}_py_parser.txt".format(desc))

	else:
		return None

def run_test( input_csv_file, output_file_desc ):

	c_output_file_name = make_output_file_name(output_file_desc, is_c=True)
	python_output_file_name = make_output_file_name(output_file_desc, is_python=True)

	py_thread = threading.Thread(target=run_python_parser, args=(input_csv_file, python_output_file_name))
	c_thread = threading.Thread(target=run_c_parser, args=(input_csv_file, c_output_file_name))

	py_thread.start()
	c_thread.start()

	py_thread.join()
	c_thread.join()
	

def run_test_set():

	for file in os.listdir('results'):
		os.remove( os.path.join('results', file))

	test_count = 1

	test_results = []
	for file in os.listdir( 'inputs' ):
		print('Running Test {}...'.format(test_count))
		test_name = "test_{}_{}".format(test_count, os.path.splitext(file)[0])

		input_csv_file = os.path.join( 'inputs', file)

		run_test( input_csv_file, test_name )

		c_output_file = make_output_file_name(test_name, is_c=True)
		py_output_file = make_output_file_name(test_name, is_python=True)

		a_test_result = {
			'test': test_count,
			'name': test_name,
			'input_file': input_csv_file,
			'c_output_file': c_output_file,
			'py_output_file': py_output_file,
			'missing_outputs': False,
			'different': False
		}

		c_output_exists = os.path.exists(c_output_file)
		py_output_exists = os.path.exists(py_output_file)

		if not c_output_exists or not py_output_exists:
			a_test_result['missing_outputs'] = True
			test_results.append(a_test_result)
			continue

		with open(c_output_file, 'r') as c_output:
			with open(py_output_file, 'r') as py_output:
				a_test_result['different'] = ( str(c_output.read()) != str(py_output.read()) )


		test_results.append(a_test_result)
		test_count += 1

	return test_results

def print_test_results(test_results, only_failures=False):
	
	for test in test_results:
		if ( not only_failures ) or ( only_failures and (test['missing_outputs'] or test['different'])):
			print('Test: {}'.format(test['test']))
			print('Input File: {}'.format(test['input_file']))
			print('Status: {}'.format( 'FAIL' if (test['missing_outputs'] or test['different']) else 'PASS' ))
			print('')
			print('C Output: {}'.format(test['c_output_file']))
			print('PY Output: {}'.format(test['py_output_file']))
			print('')
			if test['missing_outputs']:
				if not os.path.exists(test['c_output_file']):
					print('C Output does not exist!')
				if not os.path.exists(test['py_output_file']):
					print('PY Output does not exist!')

			if test['different']:
				print('Outputs have differening content!')

			print('===============================================>')


def main():
	# testing the result
	# run_c_parser( "inputs\\quotes_and_newlines.csv", "test_output.txt" )

	#run_python_parser( "inputs\\my_escaped_quotes.csv", "test_output.txt" )

	# run_test( "..\\addresses.csv", "test_output.txt" )

	test_results = run_test_set()

	print('Results:')

	print_test_results(test_results, only_failures=True)


if __name__ == "__main__":
	sys.exit(main())