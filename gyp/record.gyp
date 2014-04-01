# An experimental library for faster recording of SkCanvas commands.
{
    'targets': [{
        'target_name': 'record',
        'type': 'static_library',
        'include_dirs': [
            '../include/config',
            '../include/core',
        ],
        'sources': [
            '../src/record/SkRecorder.cpp',
        ],
    }]
}
