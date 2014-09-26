#include <node.h>
#include "../node_modules/nan/nan.h"
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

using namespace v8;

namespace GcProfiler
{
	struct GcProfilerData
	{
		uv_work_t request;
		time_t startTime;
		GCType type;
		GCCallbackFlags flags;
		double duration;
	};

	// static variables
	GcProfilerData * _data;
	Persistent<Function> _callback;
	Persistent<Context> _context;
	
#ifdef WIN32
	
	double _pcFreq = 0.0;
	__int64 _counterStart = 0;
	
#else

	struct timespec _timePointStart;

#endif
	
	// function prototypes
	void Init(Handle<Object> exports);
	NAN_METHOD(LoadProfiler);
	NAN_GC_CALLBACK(Before);
	NAN_GC_CALLBACK(After);
	void UvAsyncWork(uv_work_t * req);
	void UvAsyncAfter(uv_work_t * req);
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
		_data = new GcProfilerData();
		_data->startTime = time(NULL);
		StartTimer();
	}
	
	NAN_GC_CALLBACK(After)
	{
		_data->duration = EndTimer();
		_data->type = type;
		_data->flags = flags;
		_data->request.data = _data;
		
		// can't call the callback immediately - need to defer to when the event loop is ready
		uv_queue_work(uv_default_loop(), &_data->request, UvAsyncWork, (uv_after_work_cb)UvAsyncAfter);
	}
	
	void UvAsyncWork(uv_work_t * req)
	{
		// we don't actually have any work to do, we only care about the "after" callback
	}
	
	void UvAsyncAfter(uv_work_t * req)
	{
		NanScope();
		
		GcProfilerData * data = (GcProfilerData*)req->data;
		
		const unsigned argc = 4;
		Handle<Value> argv[argc] = {
			NanNew<Number>(data->startTime),
			NanNew<Number>(data->duration),
			NanNew<Number>((int)data->type),
			NanNew<Number>((int)data->flags)
		};
		
		delete data;
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(_callback), argc, argv);
	}

#ifdef __MACH__
#define CLOCK_REALTIME 0

	void clock_gettime(int /* assume CLOCK_REALTIME */, struct timespec* ts)
	{
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts->tv_sec = mts.tv_sec;
		ts->tv_nsec = mts.tv_nsec;
	}

#endif
	
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
		clock_gettime(CLOCK_REALTIME, &_timePointStart);
	}
	
	double EndTimer ()
	{
		struct timespec end;
		clock_gettime(CLOCK_REALTIME, &end);

		// subtract end from _timePointStart
		struct timespec result;

		if ((end.tv_nsec - _timePointStart.tv_nsec) < 0)
		{
			result.tv_sec = end.tv_sec - _timePointStart.tv_sec - 1;
			result.tv_nsec = 1000000000 + end.tv_nsec - _timePointStart.tv_nsec;
		}
		else
		{
			result.tv_sec = end.tv_sec - _timePointStart.tv_sec;
			result.tv_nsec = end.tv_nsec - _timePointStart.tv_nsec;
		}

		// convert to ms
		return (result.tv_sec * 1000) + double(result.tv_nsec) / 1000000; // ns -> ms
	}

#endif

};

// vim: noet ts=4 sw=4
