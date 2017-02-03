# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Builds ' dear imgui' (AKA ImGui) for use in graphical tools
{
  'targets': [
    {
      'target_name': 'imgui',
      'type': 'static_library',
      'include_dirs': [
        '../third_party/externals/imgui',
      ],
      'sources': [
        '../third_party/externals/imgui/imgui.cpp',
        '../third_party/externals/imgui/imgui.h',
        '../third_party/externals/imgui/imgui_draw.cpp',
        # Intended to be edited by the end-user. Need a different location?
        '../third_party/externals/imgui/imconfig.h',
        '../third_party/externals/imgui/imgui_internal.h',
        '../third_party/externals/imgui/stb_rect_pack.h',
        '../third_party/externals/imgui/stb_textedit.h',
        '../third_party/externals/imgui/stb_truetype.h',
        # This is just demo code, but it's really nice to have:
        '../third_party/externals/imgui/imgui_demo.cpp',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/imgui',
        ],
      },
    },
  ],
}
