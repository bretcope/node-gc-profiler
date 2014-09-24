#include <node.h>
#include "../node_modules/nan/nan.h"
#include <time.h>

#ifdef WIN32
#include <windows.h>
#else
#include <chrono>
#endif

using namespace v8;

namespace GcProfiler
{
	// static variables
	Persistent<Function> _callback;
	time_t _startTime;
	
#ifdef WIN32
	
	double _pcFreq = 0.0;
	__int64 _counterStart = 0;
	
#else

	typedef std::chrono::duration<double, std::ratio<1, 1000>> millisecondsRatioDouble;
	std::chrono::time_point<std::chrono::high_resolution_clock> _timePointStart;

#endif
	
	// function prototypes
	void Init(Handle<Object> exports);
	NAN_METHOD(LoadProfiler);
	NAN_GC_CALLBACK(Before);
	NAN_GC_CALLBACK(After);
	void StartTimer();
	double EndTimer();
	
	// init
	NODE_MODULE(GcProfiler, Init)
	
	// --- functions ---
	
	void Init (Handle<Object> exports)
	{
		NODE_SET_METHOD(exports, "loadProfiler", LoadProfiler);
	}

	NAN_METHOD(LoadProfiler)
	{
		NanScope();
		
		if (args.Length() == 0 || !args[0]->IsFunction())
		{
			NanThrowTypeError("Must provide a callback function to the profiler.");
		}
		
		NanAssignPersistent(_callback, args[0].As<Function>());
		NanAddGCPrologueCallback(Before);
		NanAddGCEpilogueCallback(After);
		
		NanReturnUndefined();
	}
	
	NAN_GC_CALLBACK(Before)
	{
		_startTime = time(NULL);
		StartTimer();
	}
	
	NAN_GC_CALLBACK(After)
	{
		double duration = EndTimer();
		const unsigned argc = 4;
		Handle<Value> argv[argc] = {
			NanNew<Number>(_startTime),
			NanNew<Number>(duration),
			NanNew<Number>((int)type),
			NanNew<Number>((int)flags)
		};
		
		_startTime = 0;
		
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(_callback), argc, argv);
		
	}
	
#ifdef WIN32
	
	void StartTimer ()
	{
		LARGE_INTEGER li;
		
		if (_pcFreq == 0.0)
		{
			QueryPerformanceFrequency(&li);
			_pcFreq = (double)li.QuadPart / 1000; // so that the freq is in ms instead of seconds.
		}
		
		QueryPerformanceCounter(&li);
		_counterStart = li.QuadPart;
	}
	
	double EndTimer ()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart - _counterStart) / _pcFreq;
	}
#else

	void StartTimer ()
	{
		_timePointStart = std::chrono::high_resolution_clock::now();
	}
	
	double EndTimer ()
	{
		auto duration = std::chrono::high_resolution_clock::now() - _timePointStart;
		return millisecondsRatioDouble(duration).count();
	}

#endif

};
