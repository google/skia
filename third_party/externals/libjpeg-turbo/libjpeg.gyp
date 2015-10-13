# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  # This file is not used when use_system_libjpeg==1. Settings for building with
  # the system libjpeg is in third_party/libjpeg/libjpeg.gyp.
  'variables': {
    'shared_generated_dir': '<(SHARED_INTERMEDIATE_DIR)/third_party/libjpeg_turbo',
    'conditions': [
      [ 'OS=="win"', {
        'object_suffix': 'obj',
      }, {
        'object_suffix': 'o',
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'libjpeg',
      'type': 'static_library',
      'include_dirs': [
        '.',
      ],
      'defines': [
        'WITH_SIMD',
        'MOTION_JPEG_SUPPORTED',
        'NO_GETENV',
      ],
      'sources': [
        'jcapimin.c',
        'jcapistd.c',
        'jccoefct.c',
        'jccolor.c',
        'jcdctmgr.c',
        'jchuff.c',
        'jchuff.h',
        'jcinit.c',
        'jcmainct.c',
        'jcmarker.c',
        'jcmaster.c',
        'jcomapi.c',
        'jconfig.h',
        'jcparam.c',
        'jcphuff.c',
        'jcprepct.c',
        'jcsample.c',
        'jdapimin.c',
        'jdapistd.c',
        'jdatadst.c',
        'jdatasrc.c',
        'jdcoefct.c',
        'jdcolor.c',
        'jdct.h',
        'jddctmgr.c',
        'jdhuff.c',
        'jdhuff.h',
        'jdinput.c',
        'jdmainct.c',
        'jdmarker.c',
        'jdmaster.c',
        'jdmerge.c',
        'jdphuff.c',
        'jdpostct.c',
        'jdsample.c',
        'jerror.c',
        'jerror.h',
        'jfdctflt.c',
        'jfdctfst.c',
        'jfdctint.c',
        'jidctflt.c',
        'jidctfst.c',
        'jidctint.c',
        'jidctred.c',
        'jinclude.h',
        'jmemmgr.c',
        'jmemnobs.c',
        'jmemsys.h',
        'jmorecfg.h',
        'jpegint.h',
        'jpeglib.h',
        'jpeglibmangler.h',
        'jquant1.c',
        'jquant2.c',
        'jutils.c',
        'jversion.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
        ],
      },
      'msvs_disabled_warnings': [4018, 4101],
      # VS2010 does not correctly incrementally link obj files generated
      # from asm files. This flag disables UseLibraryDependencyInputs to
      # avoid this problem.
      'msvs_2010_disable_uldi_when_referenced': 1,
      'conditions': [
        [ 'OS!="win"', {'product_name': 'jpeg_turbo'}],
        # Add target-specific source files.
        [ 'target_arch=="ia32"', {
          'sources': [
            'simd/jsimd_i386.c',
            'simd/jccolmmx.asm',
            'simd/jccolss2.asm',
            'simd/jcgrammx.asm',
            'simd/jcgrass2.asm',
            'simd/jcqnt3dn.asm',
            'simd/jcqntmmx.asm',
            'simd/jcqnts2f.asm',
            'simd/jcqnts2i.asm',
            'simd/jcqntsse.asm',
            'simd/jcsammmx.asm',
            'simd/jcsamss2.asm',
            'simd/jdcolmmx.asm',
            'simd/jdcolss2.asm',
            'simd/jdmermmx.asm',
            'simd/jdmerss2.asm',
            'simd/jdsammmx.asm',
            'simd/jdsamss2.asm',
            'simd/jf3dnflt.asm',
            'simd/jfmmxfst.asm',
            'simd/jfmmxint.asm',
            'simd/jfss2fst.asm',
            'simd/jfss2int.asm',
            'simd/jfsseflt.asm',
            'simd/ji3dnflt.asm',
            'simd/jimmxfst.asm',
            'simd/jimmxint.asm',
            'simd/jimmxred.asm',
            'simd/jiss2flt.asm',
            'simd/jiss2fst.asm',
            'simd/jiss2int.asm',
            'simd/jiss2red.asm',
            'simd/jisseflt.asm',
            'simd/jsimdcpu.asm',
          ],
        }],
        [ 'target_arch=="x64" and msan!=1', {
          'sources': [
            'simd/jsimd_x86_64.c',
            'simd/jccolss2-64.asm',
            'simd/jcgrass2-64.asm',
            'simd/jcqnts2f-64.asm',
            'simd/jcqnts2i-64.asm',
            'simd/jcsamss2-64.asm',
            'simd/jdcolss2-64.asm',
            'simd/jdmerss2-64.asm',
            'simd/jdsamss2-64.asm',
            'simd/jfss2fst-64.asm',
            'simd/jfss2int-64.asm',
            'simd/jfsseflt-64.asm',
            'simd/jiss2flt-64.asm',
            'simd/jiss2fst-64.asm',
            'simd/jiss2int-64.asm',
            'simd/jiss2red-64.asm',
          ],
        }],
        # MemorySanitizer doesn't support assembly code, so keep it disabled in
        # MSan builds for now.
        [ 'msan==1', {
          'sources': [
            'jsimd_none.c',
          ],
        }],
        # The ARM SIMD implementation can be used for devices that support
        # the NEON instruction set. This can safely be done dynamically by
        # probing CPU features at runtime, if you wish.
        [ 'target_arch=="arm"', {
          'conditions': [
            [ 'arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)', {
              'sources': [
                'simd/jsimd_arm.c',
                'simd/jsimd_arm_neon.S',
              ],
            }, {
              'sources': [
                'jsimd_none.c',
              ],
            }]
          ],
        }],
        [ 'target_arch=="arm64"', {
          'sources': [
            'simd/jsimd_arm64.c',
            'simd/jsimd_arm64_neon.S',
          ],
        }],
        [ 'target_arch=="mipsel" or target_arch=="mips64el"', {
          'sources': [
            'jsimd_none.c',
          ],
        }],

        # Build rules for an asm file.
        # On Windows, we use the precompiled yasm binary. On Linux, we build
        # our patched yasm and use it except when use_system_yasm is 1. On
        # Mac, we always build our patched yasm and use it because of
        # <http://www.tortall.net/projects/yasm/ticket/236>.
        [ 'OS=="win"', {
          'variables': {
            'yasm_path': '../yasm/binaries/win/yasm<(EXECUTABLE_SUFFIX)',
            'conditions': [
              [ 'target_arch=="ia32"', {
                'yasm_format': '-fwin32',
                'yasm_flags': [
                  '-D__x86__',
                  '-DWIN32',
                  '-DMSVC',
                  '-Iwin/'
                ],
              }, {
                'yasm_format': '-fwin64',
                'yasm_flags': [
                  '-D__x86_64__',
                  '-DWIN64',
                  '-DMSVC',
                  '-Iwin/'
                ],
              }],
            ],
          },
        }],
        [ 'OS=="mac" or OS=="ios"', {
          'dependencies': [
            '../yasm/yasm.gyp:yasm#host',
          ],
          'variables': {
            'yasm_path': '<(PRODUCT_DIR)/yasm',
            'conditions': [
              [ 'target_arch=="ia32"', {
                'yasm_format': '-fmacho',
                'yasm_flags': [
                  '-D__x86__',
                  '-DMACHO',
                  '-Imac/'
                ],
              }, {
                'yasm_format': '-fmacho64',
                'yasm_flags': [
                  '-D__x86_64__',
                  '-DMACHO',
                  '-Imac/'
                ],
              }],
            ],
          },
        }],
        [ 'OS=="linux" or OS=="freebsd" or (OS=="android" and (target_arch=="ia32" or target_arch=="x64"))', {
          'conditions': [
            [ 'use_system_yasm==0', {
              'dependencies': [
                '../yasm/yasm.gyp:yasm#host',
              ],
            }],
          ],
          'variables': {
            'conditions': [
              [ 'use_system_yasm==1', {
                'yasm_path': '<!(which yasm)',
              }, {
                'yasm_path': '<(PRODUCT_DIR)/yasm',
              }],
              [ 'target_arch=="ia32"', {
                'yasm_format': '-felf',
                'yasm_flags': [
                  '-D__x86__',
                  '-DELF',
                  '-Ilinux/'
                ],
              }, {
                'yasm_format': '-felf64',
                'yasm_flags': [
                  '-D__x86_64__',
                  '-DELF',
                  '-Ilinux/'
                ],
              }],
            ],
          },
        }],
      ],
      'rules': [
        {
          'rule_name': 'assemble',
          'extension': 'asm',
          'conditions': [
            [ 'target_arch=="ia32" or target_arch=="x64"', {
              'inputs': [ '<(yasm_path)', ],
              'outputs': [
                '<(shared_generated_dir)/<(RULE_INPUT_ROOT).<(object_suffix)',
              ],
              'action': [
                '<(yasm_path)',
                '<(yasm_format)',
                '<@(yasm_flags)',
                '-DRGBX_FILLER_0XFF',
                '-DSTRICT_MEMORY_ACCESS',
                '-Isimd/',
                '-o', '<(shared_generated_dir)/<(RULE_INPUT_ROOT).<(object_suffix)',
                '<(RULE_INPUT_PATH)',
              ],
              'process_outputs_as_sources': 1,
              'message': 'Building <(RULE_INPUT_ROOT).<(object_suffix)',
            }],
          ]
        },
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
