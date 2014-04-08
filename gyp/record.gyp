# An experimental library for faster recording of SkCanvas commands.
{
    'targets': [{
        'target_name': 'record',
        'type': 'static_library',
        'include_dirs': [
            '../include/config',
            '../include/core',
        ],
        'direct_dependent_settings': {
            'include_dirs': [
                '../src/record',
            ],
        },
        'sources': [
            '../src/record/SkRecorder.cpp',
            '../src/record/SkRecordCulling.cpp',
            '../src/record/SkRecordDraw.cpp',
        ],
    }]
}
