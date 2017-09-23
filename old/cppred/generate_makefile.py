#!/usr/bin/python3

import os
import os.path
import fnmatch
import sys

def concat(xs):
	s = ''
	empty = True
	for x in xs:
		if not empty:
			s += ' '
		else:
			empty = False
		s += x
	return s

def list_dir(path, list):
	for x in os.listdir(path):
		if os.path.isfile(x) and fnmatch.fnmatch(x, '*.cpp'):
			list.append((x, x.replace('.cpp', '.o'), None))

def main():
	files = [
		('../generated_files/cpu.generated.cpp', 'cpu.generated.o', '.')
	]
	list_dir('.', files)
	sys.stdout = open('Makefile', 'w')

	cxx = 'c++'
	cxxflags = '-O3 -std=c++14 ' + os.environ['INCLUDES']
	libs = os.environ['LIBS']
	output_file = 'pdboy'

	objects = concat([x[1] for x in files])

	print('%s: %s'%(output_file, objects))
	print('\t%s %s -s -o %s %s -pthread'%(cxx, objects, output_file, libs))
	print('')
	print('clean:')
	print('\trm %s %s'%(output_file, objects))
	
	for x in files:
		print('')
		print('%s:'%(x[1]))
		cmd = '\t%s %s %s -c -o %s'%(cxx, x[0], cxxflags, x[1])
		if x[2] != None:
			cmd += ' -I' + x[2]
		print(cmd)


main()

