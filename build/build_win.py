from build_any import cmake
import argparse

parser = argparse.ArgumentParser(description='Builds cmake project for Linux host.')
parser.add_argument('--clean', action='store_true', default=False, help='removes the build environment before start')
parser.add_argument('--debug', action='store_true', default=False, help='builds and packs Debug instead of Release')
args = parser.parse_args()

build = cmake('win3\\debug' if args.debug else 'win32\\release')
build.configure(clean=args.clean)
build.run('--config', 'Debug' if args.debug else 'Release')

if args.debug:
	build.archive(['ZIP'], '-C', 'Debug')
else:
	build.archive(['ZIP'])
