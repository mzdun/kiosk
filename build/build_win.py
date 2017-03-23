from build_any import cmake
import argparse
import multiprocessing

parser = argparse.ArgumentParser(description='Builds cmake project for Linux host.')
parser.add_argument('--clean', action='store_true', default=False, help='removes the build environment before start')
parser.add_argument('--debug', action='store_true', default=False, help='builds and packs Debug instead of Release')
parser.add_argument('--no-pack', action='store_true', default=False, help='does not pack at the end of the process')
args = parser.parse_args()

build = cmake('win32\\debug' if args.debug else 'win32\\release')
build.configure('-DCMAKE_BUILD_TYPE={}'.format('Debug' if args.debug else 'Release'), '-G', 'Visual Studio 15 2017', clean=args.clean)
build.run('--config', 'Debug' if args.debug else 'Release', '--', '/maxcpucount:' + str(multiprocessing.cpu_count()))

if not args.no_pack:
	if args.debug:
		build.archive(['ZIP'], '-C', 'Debug')
	else:
		build.archive(['ZIP'])
