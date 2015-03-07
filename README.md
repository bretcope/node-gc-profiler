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
| forced | boolean | True for a forced cycle. |
| flags | number | The raw GCCallbackFlags provided from v8. |

The `profiler.GCCallbackFlags` enumeration is provided to help decode the `flags` property.

```js
profiler.GCCallbackFlags = {
  kNoGCCallbackFlags: 0,
  kGCCallbackFlagCompacted: 1 << 0, // this flag is never set in v8 versions >= 3.6.5
  kGCCallbackFlagConstructRetainedObjectInfos: 1 << 1,
  kGCCallbackFlagForced: 1 << 2
};
```
