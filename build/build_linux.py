#!/usr/bin/env python

from build_any import cmake
import argparse, sys

parser = argparse.ArgumentParser(description='Builds cmake project for Linux host.')
parser.add_argument('--clean', action='store_true', default=False, help='removes the build environment before start')
parser.add_argument('--debug', action='store_true', default=False, help='builds and packs Debug instead of Release')
parser.add_argument('--no-pack', action='store_true', default=False, help='does not pack at the end of the process')
args = parser.parse_args()

build = cmake('linux/debug' if args.debug else 'linux/release')
build.configure('-DCMAKE_BUILD_TYPE={}'.format('Debug' if args.debug else 'Release'), clean=args.clean)
build.run()
if not args.no_pack:
	build.archive(['DEB', 'ZIP', 'TGZ'])
