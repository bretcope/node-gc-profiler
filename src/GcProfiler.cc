#include <node.h>
#include <nan.h>
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
	Nan::Persistent<v8::Function> _callback;
	
#ifdef WIN32
	
	double _pcFreq = 0.0;
	__int64 _counterStart = 0;
	
#else

	struct timespec _timePointStart;

#endif
	
	// function prototypes
	void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module);
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
	
	void Init (v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
	{
		exports->Set(Nan::New("loadProfiler").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(LoadProfiler)->GetFunction());
	}


	void LoadProfiler(const Nan::FunctionCallbackInfo<v8::Value>& info)
	{
		if (info.Length() == 0 || !info[0]->IsFunction())
		{
			Nan::ThrowTypeError("Must provide a callback function to the profiler.");
			return;
		}
		
		_callback.Reset(info[0].As<v8::Function>());
		
		Nan::AddGCPrologueCallback(Before);
		Nan::AddGCEpilogueCallback(After);
		
		return info.GetReturnValue().SetUndefined();
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
		Nan::HandleScope scope;
		
		GcProfilerData * data = (GcProfilerData*)req->data;
		
		const unsigned argc = 4;
		v8::Local<v8::Value> argv[argc] = {
			Nan::New<Number>(data->startTime),
			Nan::New<Number>(data->duration),
			Nan::New<Number>((int)data->type),
			Nan::New<Number>((int)data->flags)
		};
		
		delete data;
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New(_callback), argc, argv);
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
