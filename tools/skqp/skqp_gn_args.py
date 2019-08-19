# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SkqpGnArgs = {
    'extra_cflags':                     '[ "-DSK_ENABLE_DUMP_GPU", "-DSK_BUILD_FOR_SKQP" ]',
    'skia_enable_fontmgr_android':      'false',
    'skia_enable_fontmgr_empty':        'true',
    'skia_enable_pdf':                  'false',
    'skia_enable_skottie':              'false',
    'skia_skqp_global_error_tolerance': '8',
    'skia_tools_require_resources':     'true',
    'skia_use_dng_sdk':                 'false',
    'skia_use_expat':                   'true',
    'skia_use_icu':                     'false',
    'skia_use_libheif':                 'false',
    'skia_use_lua':                     'false',
    'skia_use_piex':                    'false',
    'skia_use_vulkan':                  'true',
}

def GetGNArgs(arch, android_ndk, debug, api_level):
    def gn_quote(s):
        return '"%s"' % s
    gn_args = {
        'target_cpu':  gn_quote(arch),
        'ndk':         gn_quote(android_ndk),
        'is_debug':    'true' if debug else 'false',
        'ndk_api':     api_level,
    }
    gn_args.update(SkqpGnArgs)
    return gn_args
