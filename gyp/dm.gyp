# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
# vim: set expandtab tabstop=4 shiftwidth=4
{
    'includes': [ 'apptype_console.gypi' ],

    'targets': [{
        'target_name': 'dm',
        'type': 'executable',
        'include_dirs': [
            '../bench',
            '../dm',
            '../gm',
            '../tests',
            '../src/images',
            '../src/lazy',
            '../src/core',
            '../src/effects',
            '../src/pipe/utils/',
            '../src/utils',
            '../src/utils/debugger',
            '../tools',
        ],
        'includes': [
            'bench.gypi',
            'gmslides.gypi',
            'pathops_unittest.gypi',
            'tests.gypi',
        ],
        'sources': [
            '../dm/DM.cpp',
            '../dm/DMBenchTask.cpp',
            '../dm/DMCpuGMTask.cpp',
            '../dm/DMExpectationsTask.cpp',
            '../dm/DMGpuGMTask.cpp',
            '../dm/DMPDFRasterizeTask.cpp',
            '../dm/DMPDFTask.cpp',
            '../dm/DMPipeTask.cpp',
            '../dm/DMQuiltTask.cpp',
            '../dm/DMRecordTask.cpp',
            '../dm/DMReplayTask.cpp',
            '../dm/DMReporter.cpp',
            '../dm/DMSKPTask.cpp',
            '../dm/DMSerializeTask.cpp',
            '../dm/DMTask.cpp',
            '../dm/DMTaskRunner.cpp',
            '../dm/DMTestTask.cpp',
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
            'etc1.gyp:libetc1',
            'flags.gyp:flags',
            'gputest.gyp:skgputest',
            'jsoncpp.gyp:jsoncpp',
            'skia_lib.gyp:skia_lib',
            'tools.gyp:crash_handler',
        ],
        'conditions': [
          ['skia_android_framework', {
              'libraries': [ '-lskia' ],
          }],
          ['skia_poppler_enabled', {
              'sources':      [ '../src/utils/SkPDFRasterizer.cpp' ],
              'defines':      [ 'SK_BUILD_POPPLER' ],
              'dependencies': [ 'poppler.gyp:*' ],
          }],
        ],
    }]
}
