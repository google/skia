# Source list for SkRecord
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
# The Skia build defines this in common_variables.gypi.
{
    'sources': [
        '<(skia_src_path)/record/SkRecordDraw.cpp',
        '<(skia_src_path)/record/SkRecordOpts.cpp',
        '<(skia_src_path)/record/SkRecorder.cpp',
        '<(skia_src_path)/record/SkRecording.cpp',
    ]
}
