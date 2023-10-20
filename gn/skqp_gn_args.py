# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

def GetGNArgs(api_level, debug, arch=None, ndk=None, is_android_bp=False):
    gn_args = {
        'ndk_api':                          api_level,
        'is_debug':                         'true' if debug else 'false',
        'skia_enable_fontmgr_android':      'false',
        'skia_enable_fontmgr_empty':        'true',
        'skia_enable_graphite':             'true',
        'skia_enable_pdf':                  'false',
        'skia_enable_skottie':              'false',
        'skia_enable_skshaper':             'false',
        'skia_enable_sksl_tracing':         'true',
        'skia_enable_sktext':               'false',
        'skia_enable_svg':                  'false',
        'skia_enable_tools':                'true',
        'skia_tools_require_resources':     'true',
        'skia_use_dng_sdk':                 'false',
        'skia_use_expat':                   'true',
        'skia_use_freetype':                'false',
        'skia_use_icu':                     'false',
        'skia_use_libheif':                 'false',
        'skia_use_lua':                     'false',
        'skia_use_piex':                    'false',
        'skia_use_vulkan':                  'true',
        'skia_use_wuffs':                   'true',
    }

    def gn_quote(s):
        return '"%s"' % s

    if is_android_bp is True:
        gn_args.update({
            'target_os':          gn_quote("android"),
            'target_cpu':         gn_quote("none"),
            'is_official_build':  'true',
            # gn_to_bp.py copies vk_mem_alloc.h to //vma_android/include
            'skia_vulkan_memory_allocator_dir': '"//vma_android"',
        })
    else:
        gn_args.update({
            'target_cpu':  gn_quote(arch),
            'ndk':         gn_quote(ndk),
        })
    return gn_args
