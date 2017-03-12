#!/usr/bin/env python

from __future__ import print_function
from build_any import cmake, safe, clean_dir
import os, sys, subprocess
import build_any
import argparse

parser = argparse.ArgumentParser(description='Builds cmake project for Linux host.')
parser.add_argument('--clean', action='store_true', default=False, help='removes the build environment before start')
args = parser.parse_args()

KNOWN_DIRS = [
	'.',
	'~',
	'~/raspberrypi',
	'~/raspi'
]

KNOWN_FILES = [
	'.pi.cmake',
	'pi.cmake',
	'.rpi.cmake',
	'rpi.cmake',
	'.raspi.cmake',
	'raspi.cmake',
	'.raspberrypi.cmake',
	'raspberrypi.cmake',
]

def find_toolchain():
	for dirname in KNOWN_DIRS:
		for filename in KNOWN_FILES:
			tc = os.path.expanduser(os.path.join(dirname, filename))
			if os.path.isfile(tc):
				return tc

toolchain = find_toolchain()
if toolchain is None:
	print('No pi.cmake found...', file=sys.stderr)
	sys.exit(2)

build = cmake('armv6')
build.configure('-DCMAKE_TOOLCHAIN_FILE={}'.format(toolchain), clean=args.clean)
build.run()
build.archive(['DEB', 'TGZ'])
