{
    'targets': [{
        'target_name': 'skia_launcher',
        'type': 'executable',
        'cflags': [ '-fPIE' ],
        'ldflags': [ '-pie' ],
        'sources': [ '../launcher/skia_launcher.cpp' ],
    }]
}
