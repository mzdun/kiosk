(function(){

function $(id) { return document.getElementById(id); }

function ii(i, len) {
	var s = i + '';
	len = len || 2;
	while (s.length < len) s = '0' + s;
	return s;
}

var dddd = ["Niedziela", "Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota"];

function update_time()
{
	var d = new Date();
	$('clock').innerText = d.getHours() + ':' + ii(d.getMinutes(), 2);
	$('date').innerText = dddd[d.getDay()] + '\xa0' + d.getFullYear() + '/' + ii(d.getMonth() + 1, 2) + '/' + ii(d.getDate(), 2);
}

window.onload = function () {
	update_time();
	var next_minute = function () {
		setInterval(update_time, 60000);
		update_time();
	};
	var msecs = (new Date()).getSeconds() * 1000;
	setTimeout(next_minute, 60000 - msecs);
};

})();
