# Poppler, assuming it's been installed to the (Linux) system.
{
    'targets': [{
        'target_name': 'poppler',
        'type': 'none',
        'direct_dependent_settings': {
            'libraries': [
                '-lpoppler-cpp',
            ],
            'include_dirs': [
                '/usr/include/poppler/cpp',
            ],
        },
    }],
}
