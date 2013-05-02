# This GYP file stores the dependencies necessary to build Skia on the Android
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.
#
# We tried adding this gyp file to the android directory at the root of
# the Skia repo, but that resulted in the generated makefiles being created
# outside of the intended output directory. So to avoid this we created a simple
# shim here that includes the android_system.gypi file.  The actual dependencies
# are defined and maintained in that gypi file.
#
# Also this assumes that the android directory is a sibling to the directory
# that contains your primary Skia checkout. If it is not then you must manually
# edit the includes below to specify the actual location of the android.gypi.
# This is due to the fact that we cannot use variables in an includes as the
# variable expansion step for gyp happens after the includes are processed.
{
  'includes': [
    '../platform_tools/android/gyp/skia_android.gypi',
  ],
}
