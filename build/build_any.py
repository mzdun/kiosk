from __future__ import print_function
import sys, os, shutil, errno, subprocess

def mkdir_p(path):
	try:
		os.makedirs(path)
	except OSError as exc:  # Python >2.5
		if exc.errno == errno.EEXIST and os.path.isdir(path): pass
		else: raise

def rm_r(path):
	if not os.path.exists(path): return
	if os.path.isfile(path):
		os.remove(path)
	else:
		shutil.rmtree(path)

def clean_dir(bin):
	print('++ rm -r {}'.format(bin))
	rm_r(bin)
	print('++ mkdir -p {}'.format(bin))
	mkdir_p(bin)

def run(*cmd):
	print('++', *cmd)
	if sys.platform.startswith('win'):
		shell = True
	else:
		shell = False

	subprocess.check_call(cmd, shell=shell)

def safe(callable, *args, **kwargs):
	try:
		callable(*args)
	except:
		if 'msg' in kwargs: print(kwargs['msg'], file=sys.stderr)
		sys.exit(2)

class cmake:
	def __init__(self, os_name):
		self.srcdir = os.path.abspath(os.path.join(os.path.split(__file__)[0], os.pardir, 'source'))
		self.bindir = os.path.join(os.getcwd(), 'bin', os_name)
		self.pkgdir = os.path.join(os.getcwd(), 'pkgs', os_name)

	def configure(self, *args, **kwargs):
		if 'clean' in kwargs and kwargs['clean']:
			safe(clean_dir, self.bindir,
				msg='{}: cannot use as cmake binary directory'.format(self.bindir))
		else:
			print('++ mkdir -p {}'.format(self.bindir))
			safe(mkdir_p, self.bindir,
				msg='{}: cannot use as cmake binary directory'.format(self.bindir))

		safe(os.chdir, self.bindir,
			msg='{}: cannot change directory'.format(self.bindir))
		safe(run, 'cmake', *(args + ('-DCPACK_OUTPUT_FILE_PREFIX={}'.format(self.pkgdir.replace('\\', '/')), self.srcdir)))

	def run(self, *args):
		safe(run, 'cmake', '--build', self.bindir, *args)

	def archive(self, packers, *additional):
		print('++ mkdir -p {}'.format(self.pkgdir))
		safe(mkdir_p, self.bindir,
			msg='{}: cannot use as cpack directory'.format(self.pkgdir))

		safe(os.chdir, self.bindir,
			msg='{}: cannot change directory'.format(self.bindir))

		for packer in packers:
			safe(run, 'cpack', '-G', packer, *additional)
