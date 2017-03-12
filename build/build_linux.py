#!/usr/bin/env python

from build_any import cmake
import argparse, sys

parser = argparse.ArgumentParser(description='Builds cmake project for Linux host.')
parser.add_argument('--clean', action='store_true', default=False, help='removes the build environment before start')
args = parser.parse_args()

build = cmake('linux')
build.configure(clean=args.clean)
build.run()
build.archive(['ZIP', 'TGZ'])
