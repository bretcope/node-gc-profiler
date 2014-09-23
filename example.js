"use strict";

var main = require('./');
main.on('gc', function (info)
{
	console.log(info.type, info.duration);
});

run();

function run ()
{
	buildLots();
	setImmediate(run);
}

function buildLots()
{
	var arr = [];
	for (var i = 0; i < 10000000; i++)
	{
		arr.push(i);
	}
}
