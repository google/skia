# An experimental library for faster recording of SkCanvas commands.
{
    'targets': [{
        'target_name': 'record',
        'type': 'static_library',
        'includes': [ 'record.gypi' ],
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
    }]
}
