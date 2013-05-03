function exportVar {
  NAME=$1
  VALUE=$2
  echo export $NAME=\"$VALUE\"
  export $NAME="$VALUE"
}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# A valid Android SDK installation is required to build the sample app.
if [ -z "$ANDROID_SDK_ROOT" ]; then
  ANDROID_TOOL=$(which android 2>/dev/null)
  if [ -z "$ANDROID_TOOL" ]; then
    echo "ERROR: Please define ANDROID_SDK_ROOT in your environment to point"
    echo "       to a valid Android SDK installation."
    return 1
  fi
  ANDROID_SDK_ROOT=$(cd $(dirname "$ANDROID_TOOL")/.. && pwd)
  exportVar ANDROID_SDK_ROOT "$ANDROID_SDK_ROOT"
fi

# ant is required to be installed on your system and in your PATH
ant -version &> /dev/null
if [[ "$?" != "0" ]]; then
  echo "ERROR: Unable to find ant. Please install it before proceeding."
  exit 1
fi

# check to see that gclient sync ran successfully
THIRD_PARTY_EXTERNAL_DIR=${SCRIPT_DIR}/../third_party/externals
if [ ! -d "$THIRD_PARTY_EXTERNAL_DIR" ]; then
	echo ""
	echo "ERROR: Unable to find the required third_party dependencies needed to build."
	echo "       To fix this add the following line to your .gclient file and run 'gclient sync'"
	echo "        target_os = ['android']"
	echo ""
	exit 1;
fi

# determine the toolchain that we will be using
API_LEVEL=14

if [[ -z "$NDK_REV" ]];
then
    NDK_REV="8d"
fi

if [[ -z "$ANDROID_ARCH" ]];
then
    ANDROID_ARCH="arm"
fi

TOOLCHAIN_DIR=${SCRIPT_DIR}/../toolchains
if [ $(uname) == "Linux" ]; then
    echo "Using Linux toolchain."
    TOOLCHAIN_TYPE=ndk-r$NDK_REV-$ANDROID_ARCH-linux_v$API_LEVEL
elif [ $(uname) == "Darwin" ]; then
    echo "Using Mac toolchain."
    TOOLCHAIN_TYPE=ndk-r$NDK_REV-$ANDROID_ARCH-mac_v$API_LEVEL
else
    echo "Could not automatically determine toolchain!  Defaulting to Linux."
    TOOLCHAIN_TYPE=ndk-r$NDK_REV-$ANDROID_ARCH-linux_v$API_LEVEL
fi
exportVar ANDROID_TOOLCHAIN ${TOOLCHAIN_DIR}/${TOOLCHAIN_TYPE}/bin

# if the toolchain doesn't exist on your machine then we need to fetch it
if [ ! -d "$ANDROID_TOOLCHAIN" ]; then
  # gsutil must be installed on your system and in your PATH
  gsutil version &> /dev/null
  if [[ "$?" != "0" ]]; then
    echo "ERROR: Unable to find gsutil. Please install it before proceeding."
    exit 1
  fi
  # create the toolchain directory if needed
  if [ ! -d "$TOOLCHAIN_DIR" ]; then
    mkdir $TOOLCHAIN_DIR
  fi
  # enter the toolchain directory then download, unpack, and remove the tarball 
  pushd $TOOLCHAIN_DIR
  TARBALL=ndk-r$NDK_REV-v$API_LEVEL.tgz
  gsutil cp gs://chromium-skia-gm/android-toolchains/$TARBALL $TARBALL
  echo "Untarring $TOOLCHAIN_TYPE from $TARBALL."
  tar -xzf $TARBALL $TOOLCHAIN_TYPE
  echo "Removing $TARBALL"
  rm $TARBALL
  popd
fi

if [ ! -d "$ANDROID_TOOLCHAIN" ]; then
  echo "ERROR: unable to download/setup the required toolchain (${TOOLCHAIN_TYPE})"
  return 1;
fi

echo "The build is targeting NDK API level $API_LEVEL for use on Android 4.0 (NDK Revision $NDK_REV) and above"

LS="/bin/ls"  # Use directly to avoid any 'ls' alias that might be defined.
GCC=$($LS $ANDROID_TOOLCHAIN/*-gcc | head -n1)
if [ -z "$GCC" ]; then
    echo "ERROR: Could not find Android cross-compiler in: $ANDROID_TOOLCHAIN"
    return 1
fi

# Remove the '-gcc' at the end to get the full toolchain prefix
ANDROID_TOOLCHAIN_PREFIX=${GCC%%-gcc}

exportVar AR "$ANDROID_TOOLCHAIN_PREFIX-ar"
if [[ -z "$ANDROID_MAKE_CCACHE" ]]; then
  exportVar CC "$ANDROID_TOOLCHAIN_PREFIX-gcc"
  exportVar CXX "$ANDROID_TOOLCHAIN_PREFIX-g++"
  exportVar LINK "$ANDROID_TOOLCHAIN_PREFIX-gcc"
else
  exportVar CC "$ANDROID_MAKE_CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
  exportVar CXX "$ANDROID_MAKE_CCACHE $ANDROID_TOOLCHAIN_PREFIX-g++"
  exportVar LINK "$ANDROID_MAKE_CCACHE $ANDROID_TOOLCHAIN_PREFIX-gcc"
fi
exportVar RANLIB "$ANDROID_TOOLCHAIN_PREFIX-ranlib"
exportVar OBJCOPY "$ANDROID_TOOLCHAIN_PREFIX-objcopy"
exportVar STRIP "$ANDROID_TOOLCHAIN_PREFIX-strip"

# Helper function to configure the GYP defines to the appropriate values
# based on the target device.
setup_device() {
  DEFINES="OS=android"
  DEFINES="${DEFINES} host_os=$(uname -s | sed -e 's/Linux/linux/;s/Darwin/mac/')"
  DEFINES="${DEFINES} skia_os=android"
  DEFINES="${DEFINES} android_base=${SCRIPT_DIR}/.."
  DEFINES="${DEFINES} android_toolchain=${TOOLCHAIN_TYPE}"

  # Setup the build variation depending on the target device
  TARGET_DEVICE="$1"

  if [ -z "$TARGET_DEVICE" ]; then
    echo "INFO: no target device type was specified so using the default 'arm_v7'"
    TARGET_DEVICE="arm_v7"
  fi

  case $TARGET_DEVICE in
    nexus_s)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 armv7=1 arm_thumb=0"
        DEFINES="${DEFINES} skia_texture_cache_mb_limit=24"
        ;;
    nexus_4 | nexus_7 | nexus_10)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 armv7=1 arm_thumb=0"
        ;;
    xoom)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 armv7=1 arm_thumb=0"
        ;;
    galaxy_nexus)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 armv7=1 arm_thumb=0"
        DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
        ;;
    razr_i)
        DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=32"
        DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
        ;;
    arm_v7)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon_optional=1 armv7=1 arm_thumb=0"
        ;;
    arm_v7_thumb)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon_optional=1 armv7=1 arm_thumb=1"
        ;;
    arm)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 armv7=0 arm_thumb=0"
        ;;
    arm_thumb)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 armv7=0 arm_thumb=1"
        ;;
    x86)
        DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=32"
        DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
        ;;
    *)
        echo -n "ERROR: unknown device specified ($TARGET_DEVICE), valid values: "
        echo "nexus_[s,4,7,10] xoom galaxy_nexus arm arm_thumb arm_v7 arm_v7_thumb x86"
        return 1;
        ;;
  esac

  echo "The build is targeting the device: $TARGET_DEVICE"

  exportVar GYP_DEFINES "$DEFINES"
  exportVar SKIA_OUT "out/config/android-${TARGET_DEVICE}"
}

# Run the setup device command initially as a convenience for the user
#setup_device
#echo "** The device has been setup for you by default. If you would like to **"
#echo "** use a different device then run the setup_device function with the **"
#echo "** appropriate input.                                                 **"

# Use the "android" flavor of the Makefile generator for both Linux and OS X.
exportVar GYP_GENERATORS "make-android"

# Helper function so that when we run "make" to build for clank it exports
# the toolchain variables to make.
#make_android() {
#  CC="$CROSS_CC" CXX="$CROSS_CXX" LINK="$CROSS_LINK" \
#  AR="$CROSS_AR" RANLIB="$CROSS_RANLIB" \
#    command make $*
#}
