# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
# vim: set expandtab tabstop=4 shiftwidth=4
{
    'includes': [ 'apptype_console.gypi' ],

    'targets': [{
        'target_name': 'dm',
        'type': 'executable',
        'include_dirs': [
            '../dm',
            '../gm',
            '../src/images',
            '../src/lazy',
            '../src/core',
            '../src/effects',
            '../src/pipe/utils/',
            '../src/utils',
            '../src/utils/debugger',
        ],
        'includes': [ 'gmslides.gypi' ],
        'sources': [
            '../dm/DM.cpp',
            '../dm/DMCpuTask.cpp',
            '../dm/DMExpectationsTask.cpp',
            '../dm/DMGpuTask.cpp',
            '../dm/DMPipeTask.cpp',
            '../dm/DMReplayTask.cpp',
            '../dm/DMReporter.cpp',
            '../dm/DMSerializeTask.cpp',
            '../dm/DMTask.cpp',
            '../dm/DMTaskRunner.cpp',
            '../dm/DMTileGridTask.cpp',
            '../dm/DMUtil.cpp',
            '../dm/DMWriteTask.cpp',
            '../gm/gm.cpp',
            '../gm/gm_expectations.cpp',

            '../src/pipe/utils/SamplePipeControllers.cpp',
            '../src/utils/debugger/SkDebugCanvas.cpp',
            '../src/utils/debugger/SkDrawCommand.cpp',
            '../src/utils/debugger/SkObjectParser.cpp',
        ],
        'dependencies': [
            'skia_lib.gyp:skia_lib',
            'flags.gyp:flags',
            'jsoncpp.gyp:jsoncpp',
            'gputest.gyp:skgputest',
        ],
    }]
}
