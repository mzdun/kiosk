(function(){
Element.prototype.with = function() {
	for (var i = 0; i < arguments.length; ++i) {
		var arg = arguments[i];
		if (typeof(arg) == 'string')
			arg = document.createTextNode(arg);
		if (arg instanceof Node)
			this.appendChild(arg);
	}
	return this;
};
function $(name) { return document.createElement(name); }

function panel_error_ex(cb) {
	var td = $('td');
	td.className = 'error';
	cb(td);
	window.panel.with($('table').with($('tr').with(td)));
}

function panel_error(msg) {
	console.error(msg);

	panel_error_ex(function(_){
		_.with($('b').with('Error:'), ' ' + msg);
	});
}

function resize_pane(widget, width) {
	console.log(widget, width);
}

function build_frames()
{
	for (var i = 0; i < panes.length; ++i) {
		var pane = panes[i];
		var widget = pane.widget;
		var src = pane.src;

		if (typeof(widget) == 'undefined') {
			console.warn('Pane #' + i + ' has no "widget" attribute. Ignoring.');
			continue;
		}

		if (typeof(src) == 'undefined') {
			src = 'widgets/' + widget + '/index.html';
		}

		var resize = resize_pane.bind(this, widget);
		var iframe = $('iframe');
		iframe.onload = function () {
			this.contentWindow.resize_pane = resize;
		}
		iframe.setAttribute('src', src);
		var div = $('div').with(iframe);
		div.setAttribute('id', widget);
		panel.with(div);
	}
}

function move_pane(widget, x, y, width, height, styling) {
	console.log([x, y, width, height, widget, styling]);
	var className = 'panel widget';
	if (styling)
		className += ' ' + styling;

	var div = document.getElementById(widget);
	div.className = className;
	div.style.left   = x + "px";
	div.style.top    = y + "px";
	div.style.width  = width + "px";
	div.style.height = height + "px";
}

function layout() {
	var panel_x = 0;
	var panel_y = 0;
	var panel_width = panel.clientWidth;
	var panel_height = panel.clientHeight;

	for (var i = 0; i < panes.length; ++i) {
		var pane = panes[i];
		var widget = pane.widget;

		if (typeof(widget) == 'undefined') {
			continue;
		}

		var snap = pane.snap;
		if (!snap) {
			move_pane(widget, panel_x, panel_y, panel_width, panel_height);
			break;
		}

		var snap_name;
		var dir;
		var reverse_snap;

		if (snap == 'left' || snap == 'right') {
			dir = 'v';
			snap_name = 'width';
			reverse_snap = (snap == 'left') ? 'right' : 'left';
		} else {
			dir = 'h';
			snap_name = 'height';
			reverse_snap = (snap == 'top') ? 'bottom' : 'top';
		}

		var size = pane[snap_name] || 0;
		var styling = 'edge-' + dir + ' edge-' + reverse_snap;

		if (typeof(pane.current) != 'undefined' && pane.current >= 0)
			size = pane.current;

		if (!size)
			styling = undefined;

		var x = panel_x;
		var y = panel_y;
		var width = panel_width;
		var height = panel_height;

		if (snap == 'left' || snap == 'right') {
			width = size;
			if (snap == 'left') {
				panel_x += size + 1;
			} else {
				x = panel_x + panel_width - size;
			}
			panel_width -= size + 1;
		} else {
			height = size;
			if (snap == 'top') {
				panel_y += size + 1;
			} else {
				y = panel_y + panel_height - size;
			}
			panel_height -= size + 1;
		}

		move_pane(widget, x, y, width, height, styling);
	}
}

function onlayout(ev) {
	var xhr = ev.target;

	if (Math.floor(xhr.status / 100) > 3) {
		panel_error_ex(function(_){
			var url = document.body.getAttribute('layout');
			_.with(
				'GET ' + url + ' ' + xhr.status + ' (' + xhr.statusText +')'
			);
		});
		return;
	}

	if (!xhr.response) {
		var url = document.body.getAttribute('layout');
		panel_error(url + ' is not a JSON file.');
		return;
	}
	window.panes = xhr.response;
	build_frames();
	layout();
}

window.onload = function () {
	window.panel = $('div');
	panel.className = 'panel home-panel';
	var body = document.body;
	body.with(window.panel);
	
	var layout = body.getAttribute('layout');
	if (!layout) {
		panel_error('No @layout attribute in the body');
		return;
	}

	var xhr = new XMLHttpRequest();
	xhr.responseType = "json";
	xhr.addEventListener("load", onlayout);
	xhr.open("GET", layout);
	xhr.send();
};
})();