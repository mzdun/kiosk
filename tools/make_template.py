from __future__ import print_function
import os, sys, argparse, re, errno

def mkdir_p(path):
	try:
		os.makedirs(path)
	except OSError as exc:  # Python >2.5
		if exc.errno == errno.EEXIST and os.path.isdir(path): pass
		else: raise

parser = argparse.ArgumentParser(description='Creates a built-in file template for automatic responses.', fromfile_prefix_chars='@')
parser.add_argument('--header', action='store_true', default=False, help='Output as if in include file')
parser.add_argument('--version', metavar='V', help='Kiosk version for the footer.', required=True)
parser.add_argument('-D', metavar='NAME=VALUE', action='append', help='Variables for the template to use')
parser.add_argument('-o', metavar='PATH', help='Output filename.', required=True)

args = parser.parse_args()

def icon(src, width, height):
	with open(src) as f:
		return ' '.join(re.split('\s+', f.read().split('\n', 1)[1])) \
			.replace('> <', '><') \
			.replace(' />', '/>') \
			.replace(' >', '>') \
			.replace(' width="256px"', ' width="{}"'.format(width)) \
			.replace(' height="256px"', ' height="{}"'.format(height))

if args.D is None: args.D = {}

mkdir_p(os.path.dirname(args.o))
args.o = open(args.o, 'w')

vars = {}
for define in args.D:
	key, val = define.split('=', 1)
	vars[key] = val

vars['js'] = \
	"window.onload=function(){document.getElementById('chromium-version').innerText=chrome_version();};" + \
	"function chrome_version(){" + \
		"var C=navigator.appVersion.split(')');" + \
		"for(var n in C){" + \
			"var ver=C[n].split('(')[0].split(/\s+/g);" + \
			"for(var m in ver){" + \
				"if (ver[m].startsWith('Chrome/')) return ver[m];" + \
			"}" + \
		"}" + \
	"}"
vars['style'] = \
	"body{font-family:sans-serif;color:gray;background:black;margin:0;padding:0}" + \
	".content{display:grid;grid-template-rows:min-content 1fr min-content;grid-template-columns:4em 1fr;height:100%}" + \
	"@media(min-width:800px){.content{width:800px;margin:0 auto}}" + \
	"header{grid-row:1;grid-column:1/span 2;grid-template-columns:4em 1fr;display:grid}" + \
	"header>svg{display:inline-block;grid-row:1;grid-column:1;justify-self:center;align-self:center}" + \
	"h1{grid-row:1;grid-column:2;margin:.5em}" + \
	"p{margin:1em}" + \
	"content{grid-row:2;grid-column:2}" + \
	"footer{grid-row:3;grid-column:2;font-size:50%;text-align:center}" + \
	"hr{width:60%;border:solid .5px silver;text-align:left}" + \
	"footer span,h1{color:white}"
vars['icon'] = icon("../res/kiosk.svg", "3em", "3em");
vars['kiosk-version'] = args.version

body = \
	"<html><head><style>{style}</style><script type='text/javascript'>{js}</script><title>{title}</title></head>" + \
	"<body><div class='content'>" + \
	"<header>{icon}<h1>{header}</h1></header>" + \
	"<content>{content}</content>" + \
	"<footer><hr/><p>Powered by <span>Kiosk/{kiosk-version}</span> and <span id='chromium-version'></span></p></footer>" + \
	"</div></body></html>"

known = ["title", "header", "content"]

if args.header:
	for name in known:
		if name not in vars: vars[name] = ')" },\n\t{ tmplt::' + name + ', nullptr },\n\t{ tmplt::text, R"('
	print('#pragma once', file=args.o)
	print(file=args.o)
	print('enum class tmplt { text, title, header, content };', file=args.o)
	print('struct tmplt_block {', file=args.o)
	print('\ttmplt kind;', file=args.o)
	print('\tconst char* value;', file=args.o)
	print('};\n', file=args.o)
	print('constexpr tmplt_block tmplt_contents[] = {', file=args.o)
	print('\t{ tmplt::text, R"(' + body.format(**vars) + ')" }', file=args.o)
	print('};', file=args.o)
else:
	for name in known:
		if name not in vars: vars[name] = '${' + name + '}'
	print(body.format(**vars), file=args.o)
