# This GYP file stores the dependencies necessary to build Skia on the Android
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.
#
# NOTE: We tried adding the gyp file to the android/ directory at the root of
# the Skia repo, but that resulted in the generated makefiles being created
# outside of the out directory. We may be able to move the bulk of this gyp
# to the /android directory and put a simple shim here, but that has yet to be
# tested.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'cpu_features',
      'type': 'static_library',
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/cpufeatures',
        ],
      },
      'sources': [
        '../third_party/cpufeatures/cpu-features.c',
        '../third_party/cpufeatures/cpu-features.h',
      ],
      'cflags': [
        '-w',
      ],
    },
    {
      'target_name': 'expat',
      'type': 'static_library',
      'sources': [
        '../third_party/externals/expat/lib/xmlparse.c',
        '../third_party/externals/expat/lib/xmlrole.c',
        '../third_party/externals/expat/lib/xmltok.c',
      ],
      'include_dirs': [
        '../third_party/externals/expat',
        '../third_party/externals/expat/lib',
      ],
      'cflags': [
        '-w',
        '-fexceptions',
      ],
      'defines': [
        'HAVE_EXPAT_CONFIG_H',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/expat/lib',  # For expat.h
        ],
      }
    },
    {
      'target_name': 'gif',
      'type': 'static_library',
      'sources': [
        '../third_party/externals/gif/dgif_lib.c',
        '../third_party/externals/gif/gifalloc.c',
        '../third_party/externals/gif/gif_err.c',
      ],
      'include_dirs': [
        '../third_party/externals/gif',
      ],
      'cflags': [
        '-w',
        '-DHAVE_CONFIG_H',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/gif',
        ],
      }
    },
    {
      'target_name': 'png',
      'type': 'static_library',
      'sources': [
        '../third_party/externals/png/png.c',
        '../third_party/externals/png/pngerror.c',
        '../third_party/externals/png/pnggccrd.c',
        '../third_party/externals/png/pngget.c',
        '../third_party/externals/png/pngmem.c',
        '../third_party/externals/png/pngpread.c',
        '../third_party/externals/png/pngread.c',
        '../third_party/externals/png/pngrio.c',
        '../third_party/externals/png/pngrtran.c',
        '../third_party/externals/png/pngrutil.c',
        '../third_party/externals/png/pngset.c',
        '../third_party/externals/png/pngtrans.c',
        '../third_party/externals/png/pngvcrd.c',
        '../third_party/externals/png/pngwio.c',
        '../third_party/externals/png/pngwrite.c',
        '../third_party/externals/png/pngwtran.c',
        '../third_party/externals/png/pngwutil.c',
      ],
      'include_dirs': [
        '../third_party/externals/png',
      ],
      'cflags': [
        '-w',
        '-fvisibility=hidden',
      ],
      'link_settings': {
        'libraries': [
          '-lz',
        ],
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/png',
        ],
      }
    },
    {
      'target_name': 'jpeg',
      'type': 'static_library',
      'sources': [
        '../third_party/externals/jpeg/jcapimin.c',
        '../third_party/externals/jpeg/jcapistd.c',
        '../third_party/externals/jpeg/jccoefct.c',
        '../third_party/externals/jpeg/jccolor.c',
        '../third_party/externals/jpeg/jcdctmgr.c',
        '../third_party/externals/jpeg/jchuff.c',
        '../third_party/externals/jpeg/jcinit.c',
        '../third_party/externals/jpeg/jcmainct.c',
        '../third_party/externals/jpeg/jcmarker.c',
        '../third_party/externals/jpeg/jcmaster.c',
        '../third_party/externals/jpeg/jcomapi.c',
        '../third_party/externals/jpeg/jcparam.c',
        '../third_party/externals/jpeg/jcphuff.c',
        '../third_party/externals/jpeg/jcprepct.c',
        '../third_party/externals/jpeg/jcsample.c',
        '../third_party/externals/jpeg/jctrans.c',
        '../third_party/externals/jpeg/jdapimin.c',
        '../third_party/externals/jpeg/jdapistd.c',
        '../third_party/externals/jpeg/jdatadst.c',
        '../third_party/externals/jpeg/jdatasrc.c',
        '../third_party/externals/jpeg/jdcoefct.c',
        '../third_party/externals/jpeg/jdcolor.c',
        '../third_party/externals/jpeg/jddctmgr.c',
        '../third_party/externals/jpeg/jdhuff.c',
        '../third_party/externals/jpeg/jdinput.c',
        '../third_party/externals/jpeg/jdmainct.c',
        '../third_party/externals/jpeg/jdmarker.c',
        '../third_party/externals/jpeg/jdmaster.c',
        '../third_party/externals/jpeg/jdmerge.c',
        '../third_party/externals/jpeg/jdphuff.c',
        '../third_party/externals/jpeg/jdpostct.c',
        '../third_party/externals/jpeg/jdsample.c',
        '../third_party/externals/jpeg/jdtrans.c',
        '../third_party/externals/jpeg/jerror.c',
        '../third_party/externals/jpeg/jfdctflt.c',
        '../third_party/externals/jpeg/jfdctfst.c',
        '../third_party/externals/jpeg/jfdctint.c',
        '../third_party/externals/jpeg/jidctflt.c',
        '../third_party/externals/jpeg/jidctfst.c',
        '../third_party/externals/jpeg/jidctint.c',
        '../third_party/externals/jpeg/jidctred.c',
        '../third_party/externals/jpeg/jquant1.c',
        '../third_party/externals/jpeg/jquant2.c',
        '../third_party/externals/jpeg/jutils.c',
        '../third_party/externals/jpeg/jmemmgr.c',
        '../third_party/externals/jpeg/jmem-android.c', # ashmem is also available
      ],
      'include_dirs': [
        '../third_party/externals/jpeg',
      ],
      'cflags': [
        '-w',
        '-fvisibility=hidden',
        '-DAVOID_TABLES',
        '-O3',
        '-fstrict-aliasing',
        '-fprefetch-loop-arrays',
        '-DANDROID_TILE_BASED_DECODE',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/jpeg',
        ],
      }
    },
    {
      # This target is a dependency for all console-type Skia applications which
      # will run on Android.  Since Android requires us to load native code in
      # shared libraries, we need a common entry point to wrap around main().
      # Here we also change the type of all would-be executables to be shared
      # libraries.  The alternative would be to introduce a condition in every
      # executable target which changes to a shared library if the target OS is
      # Android.  This is nicer because the switch is in one place.
      'target_name': 'Android_EntryPoint',
      'type': 'static_library',
      'direct_dependent_settings': {
        'target_conditions': [
          # '_type' is an 'automatic variable' which is defined for any
          # target which defines a key-value pair with 'type' as the key (so,
          # all of them).  Conditionals inside 'target_conditions' are evaluated
          # *after* all other definitions and conditionals are evaluated, so
          # we're guaranteed that '_type' will be defined when we get here.
          # For more info, see:
          # - http://code.google.com/p/gyp/wiki/InputFormatReference#Variables
          # - http://codereview.appspot.com/6353065/
          ['_type == "executable"', {
            'type': 'shared_library',
          }],
        ],
      },
    },
    {
      # This target is a dependency for Skia Sample application which runs on
      # Android.  Since Android requires us to load native code in shared
      # libraries, we need a common entry point to wrap around main(). Here
      # we also change the type of all would-be executables to be shared
      # libraries.  The alternative would be to introduce a condition in every
      # executable target which changes to a shared library if the target OS is
      # Android.  This is nicer because the switch is in one place.
      'target_name': 'Android_SampleApp',
      'type': 'static_library',
      'direct_dependent_settings': {
        'target_conditions': [
          # '_type' is an 'automatic variable' which is defined for any
          # target which defines a key-value pair with 'type' as the key (so,
          # all of them).  Conditionals inside 'target_conditions' are evaluated
          # *after* all other definitions and conditionals are evaluated, so
          # we're guaranteed that '_type' will be defined when we get here.
          # For more info, see:
          # - http://code.google.com/p/gyp/wiki/InputFormatReference#Variables
          # - http://codereview.appspot.com/6353065/
          ['_type == "executable"', {
            'type': 'shared_library',
          }],
        ],
        'sources': [
          '../app/jni/com_skia_SkiaSampleRenderer.cpp',
        ],
      },

    },
  ]
}
