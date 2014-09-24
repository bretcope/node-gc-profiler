{
	"targets": [
		{
			"target_name": "GcProfiler",
			"sources": [ "src/GcProfiler.cc" ],
			"cflags": [
				"-std=c++11"
			],
			"conditions": [
				[ 'OS=="mac"', {
					"xcode_settings": {
						'OTHER_CPLUSPLUSFLAGS': ['-std=c++11', '-stdlib=libc++'],
						'OTHER_LDFLAGS': ['-stdlib=libc++'],
						'MACOSX_DEPLOYMENT_TARGET': '10.7'
					}
				}]
			]
		}
	]
}