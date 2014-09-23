"use strict";

var EventEmitter = require('events').EventEmitter;
var GcProfiler = require('./build/Release/GcProfiler');

var main = new EventEmitter();
module.exports = main;

var GC_TYPES = {
	1: 'Scavenge',
	2: 'MarkSweepCompact',
	3: 'All'
};

var GCCallbackFlags = {
	kNoGCCallbackFlags: 0,
	kGCCallbackFlagCompacted: 1 << 0,
	kGCCallbackFlagConstructRetainedObjectInfos: 1 << 1,
	kGCCallbackFlagForced: 1 << 2
};

GcProfiler.loadProfiler(function (startTime, ms, type, flags)
{
	var info = {
		date: new Date(startTime * 1000),
		duration: ms,
		type: GC_TYPES[type],
		compacted: !!(flags & GCCallbackFlags.kGCCallbackFlagCompacted),
		forced: !!(flags && GCCallbackFlags.kGCCallbackFlagForced)
	};
	
	main.emit('gc', info);
});
