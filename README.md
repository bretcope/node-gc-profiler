# node-gc-profiler

A cross-platform (tested on Linux, Windows, and OSX) profiler for the v8 garbage collector running inside Node.js. It will emit an event after every GC cycle providing you information about the duration and type of cycle.

```
npm install gc-profiler
```

Example:

```js
var profiler = require('gc-profiler');
profiler.on('gc', function (info) {
  console.log(info);
});
```

The `info` object contains the following properties:

| Property | Type | Description |
| -------- | ---- | ----------- |
| date | Date | The approximate start time of the GC cycle. This uses the c++ time library internally, which only has one-second resolution. |
| duration | number | The duration of the GC cycle in milliseconds. |
| type | string | Either `Scavenge` or `MarkSweepCompact` depending on the type of GC cycle. |
| compacted | boolean | True for a compacted cycle. |
| forced | boolean | True for a forced cycle. |
