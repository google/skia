# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Builds shaderc for the Vulkan backend
{
  'variables': {
    'variables': { # This is the dreaded nested variables dict so that we can
                   # have dependent variables
      'shaderc_build_configuration': '<(CONFIGURATION_NAME)',
      'conditions': [
        [ 'skia_os == "win"', {
          'shaderc_lib_name' : 'shaderc_combined.lib',
        }, {
          'shaderc_lib_name' : 'libshaderc_combined.a',
        }],
        [ 'skia_os == "win"', {
          'conditions': [
            [ 'MSVS_VERSION == "2013"', {
              'shaderc_project_type' : 'MSVS2013',
            }],
            [ 'MSVS_VERSION == "2015"', {
              'shaderc_project_type' : 'MSVS2015',
            }],
          ],
        }, {
          'shaderc_project_type' : 'ninja',
        }],
      ],
      'shaderc_out_path': '<(PRODUCT_DIR)/shaderc_out_<(skia_arch_type)',
    },
    # Export out of nested variables.
    'shaderc_build_configuration': '<(shaderc_build_configuration)',
    'shaderc_project_type': '<(shaderc_project_type)',
    'shaderc_out_path': '<(shaderc_out_path)',
    'shaderc_lib_name': '<(shaderc_lib_name)',
    'android_toolchain%': '',
    
    # On Windows the library winds up inside a 'Debug' or 'Release' dir, not so
    # with ninja project build.
    'conditions': [
      [ 'skia_os == "win"', {
        'shaderc_lib_full_path': '<(shaderc_out_path)/libshaderc/<(shaderc_build_configuration)/<(shaderc_lib_name)',
      }, {
        'shaderc_lib_full_path': '<(shaderc_out_path)/libshaderc/<(shaderc_lib_name)',
      }],
    ]
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
            '<!@(python find.py ../third_party/externals/shaderc2 "*")',
            '../tools/build_shaderc.py',
          ],
          'outputs': [
             '<(shaderc_lib_full_path)',
          ],
          'action': ['python', '../tools/build_shaderc.py', '-s', '../third_party/externals/shaderc2', '-o', '<(shaderc_out_path)', '-a', '<(skia_arch_type)', '-t', '<(shaderc_build_configuration)', '-p', '<(shaderc_project_type)', '-c', '<(android_toolchain)'],
        },
      ],     
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': ['<(shaderc_lib_full_path)'],
        },
      ],
      'all_dependent_settings': {
        'link_settings': {
          'libraries': [
            '<(shaderc_lib_name)',
          ],
        },
      },
    },
  ],
}
