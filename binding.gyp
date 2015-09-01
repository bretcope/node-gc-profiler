{
	"targets": [
		{
			"target_name": "GcProfiler",
			"sources": [ "src/GcProfiler.cc" ],
			"include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
		}
	]
}
