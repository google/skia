#
# Copyright 2013 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

{
  'targets': [
    {
      'target_name': 'lua',
      'type': 'static_library',
      'suppress_wildcard': '1',
      'sources': [
        # core
        '../third_party/lua/src/lapi.c',
        '../third_party/lua/src/lcode.c',
        '../third_party/lua/src/lctype.c',
        '../third_party/lua/src/ldebug.c',
        '../third_party/lua/src/ldo.c',
        '../third_party/lua/src/ldump.c',
        '../third_party/lua/src/lfunc.c',
        '../third_party/lua/src/lgc.c',
        '../third_party/lua/src/llex.c',
        '../third_party/lua/src/lmem.c',
        '../third_party/lua/src/lobject.c',
        '../third_party/lua/src/lopcodes.c',
        '../third_party/lua/src/lparser.c',
        '../third_party/lua/src/lstate.c',
        '../third_party/lua/src/lstring.c',
        '../third_party/lua/src/ltable.c',
        '../third_party/lua/src/ltm.c',
        '../third_party/lua/src/lundump.c',
        '../third_party/lua/src/lvm.c',
        '../third_party/lua/src/lzio.c',

        # libraries
        '../third_party/lua/src/lauxlib.c',
        '../third_party/lua/src/lbaselib.c',
        '../third_party/lua/src/lbitlib.c',
        '../third_party/lua/src/lcorolib.c',
        '../third_party/lua/src/ldblib.c',
        '../third_party/lua/src/liolib.c',
        '../third_party/lua/src/lmathlib.c',
        '../third_party/lua/src/loslib.c',
        '../third_party/lua/src/lstrlib.c',
        '../third_party/lua/src/ltablib.c',
        '../third_party/lua/src/loadlib.c',
        '../third_party/lua/src/linit.c',
      ],
      'include_dirs': [
        '../third_party/lua/src/',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '../third_party/lua/src/',
        ],
      },
      'defines': [
        "getlocaledecpoint()='.'",
      ],
      'variables': {
        'skia_lua_flags' : [
          '-Wno-parentheses-equality',
          '-Wno-pointer-bool-conversion',
          '-Wno-array-bounds',
        ],
      },
      'cflags': [ '<@(skia_lua_flags)' ],
      'xcode_settings': { 'WARNING_CFLAGS': [ '<@(skia_lua_flags)' ], },
      'conditions': [
        ['skia_os != "win"',
         {
            'defines': [
              'LUA_USE_POSIX',  # Fix warning re dangerous tmpnam.
            ],
          }
        ],
        [ 'skia_clang_build == 1', {
          'cflags':[
            '-w',
          ],
        }],
      ],
    },
  ],
}
