(function () {
	function ref_(key) {
		this.root = key;
	}
	
	ref_.prototype.load = function(key) {
		if (!key.startsWith(this.root))
			return null;

		return localStorage.getItem(key);
	}

	ref_.prototype.emplace = function(key, val, obj) {
		key = key.substr(this.root.length);
		if (key.endsWith(':n')) {
			key = key.substr(0, key.length - 2);
			val = Number(val);
		}
		var path = key.split('.');
		var dir = [];
		var name = path[path.length - 1];
		if (path.length > 1)
			dir = path.slice(0, path.length - 1);

		var ctx = obj;
		for (x in dir) {
			dname = dir[x];
			if (!(dname in ctx))
				ctx[dname] = {};
			ctx = ctx[dname];
		}
		ctx[name] = val;
	}

	function Storage() {}

	function simple_load_(key) {
		var val = localStorage.getItem(key);
		if (val === null) {
			if (key.endsWith(':n'))
				key = key.substr(0, key.length - 2);
			else
				key += ':n';
			val = localStorage.getItem(key);
		}
		
		if (val !== null && key.endsWith(':n'))
			return Number(val);
		return val;
	}
	
	Storage.prototype.load = function(key) {
		var val = simple_load_(key);
		if (val !== null)
			return val;

		if (key.endsWith(':n'))
			key = key.substr(0, key.length - 2);
		root = new ref_(key + '.');

		var vals = {};
		var found = false;

		for (var i = 0; i < localStorage.length; ++i) {
			var key = localStorage.key(i);
			var val = root.load(key);
			if (val === null) continue;
			found = true;
			root.emplace(key, val, vals);
		}
		if (!found) return null;
		return vals;
	}

	Storage.prototype.store = function (key, val) {
		// localStorage.setItem(key, val);
		if (typeof(val) != 'object') {
			console.log(key + ': ' + val);
			if (typeof(val) == 'number')
				key += ':n';
			localStorage.setItem(key, val);
			return;
		}

		for (x in val)
			store(key + '.' + x, val[x]);
	}

	window.stg = new Storage();
})();
