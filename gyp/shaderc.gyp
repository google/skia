# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Builds shaderc for the Vulkan backend
{
  'variables': {
    'conditions': [
      [ 'CONFIGURATION_NAME == "Release_Developer"', {
        'shaderc_build_type' : 'Release',
      }, {
        'shaderc_build_type' : '<(CONFIGURATION_NAME)',
      }],
      [ 'skia_os == "win"', {
        'shaderc_lib_name' : 'shaderc_combined.lib',
      }, {
        'shaderc_lib_name' : 'libshaderc_combined.a',
      }],
      [ 'MSVS_VERSION == "2013"', {
        'shaderc_project_type' : 'MSVS2013',
      }],
      [ 'MSVS_VERSION == "2015"', {
        'shaderc_project_type' : 'MSVS2015',
      }],
    ],
    'skia_build_type' : '<(CONFIGURATION_NAME)',
  },  
  'targets': [  
    {
      # Call out to a python script to build shaderc_combined and then copy it
      # to out/<Configuration>
      'target_name': 'shaderc_combined',
      'type': 'none',
      'actions': [
        {
          'action_name': 'compile_shaderc',
          'inputs': [
            '<!@(python find.py ../third_party/externals/shaderc "*")',
            '../tools/build_shaderc.py',
          ],
          'outputs': [
             '../out/<(skia_build_type)/shaderc_out_<(skia_arch_type)/libshaderc/<(shaderc_build_type)/<(shaderc_lib_name)',
          ],
          'action': ['python', '../tools/build_shaderc.py', '-s', '../third_party/externals/shaderc', '-o', '../out/<(skia_build_type)/shaderc_out_<(skia_arch_type)', '-a', '<(skia_arch_type)', '-t', '<(shaderc_build_type)', '-p', '<(shaderc_project_type)'],
        },
      ],     
      'copies': [
        {
          'destination': '../out/<(skia_build_type)',
          'files': ['../out/<(skia_build_type)/shaderc_out_<(skia_arch_type)/libshaderc/<(shaderc_build_type)/<(shaderc_lib_name)'],
        },
      ],
      'all_dependent_settings': {
        'link_settings': {
          'libraries': [
            '-lshaderc_combined',
          ],
        },
      },
    },
  ],
}
