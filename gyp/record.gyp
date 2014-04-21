# An experimental library for faster recording of SkCanvas commands.
{
    'targets': [{
        'target_name': 'record',
        'type': 'static_library',
        'include_dirs': [
            '../include/config',
            '../include/core',
            '../include/record',
        ],
        'direct_dependent_settings': {
            'include_dirs': [
                '../include/record',  # Public headers.
            ],
        },
        'sources': [
            '../src/record/SkRecordOpts.cpp',
            '../src/record/SkRecordDraw.cpp',
            '../src/record/SkRecorder.cpp',
            '../src/record/SkRecording.cpp',
        ],
    }]
}
