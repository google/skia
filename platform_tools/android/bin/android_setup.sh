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

# Helper function to determine and download the toolchain that we will be using.
setup_toolchain() {
  API_LEVEL=14

  if [[ -z "$NDK_REV" ]];
  then
    NDK_REV="8e"
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
    # create the toolchain directory if needed
    if [ ! -d "$TOOLCHAIN_DIR" ]; then
      mkdir $TOOLCHAIN_DIR
    fi
    # enter the toolchain directory then download, unpack, and remove the tarball
    pushd $TOOLCHAIN_DIR
    TARBALL=ndk-r$NDK_REV-v$API_LEVEL.tgz

    echo "Downloading $TARBALL ..."
    ${SCRIPT_DIR}/download_toolchains.py http://chromium-skia-gm.commondatastorage.googleapis.com/android-toolchains/$TARBALL $TOOLCHAIN_DIR/$TARBALL
    if [[ "$?" != "0" ]]; then
      echo "ERROR: Unable to download toolchain $TARBALL."
      exit 1
    fi

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
}

# Helper function to configure the GYP defines to the appropriate values
# based on the target device.
setup_device() {
  DEFINES="OS=android"
  DEFINES="${DEFINES} host_os=$(uname -s | sed -e 's/Linux/linux/;s/Darwin/mac/')"
  DEFINES="${DEFINES} skia_os=android"
  DEFINES="${DEFINES} android_base=${SCRIPT_DIR}/.."
  DEFINES="${DEFINES} skia_shared_lib=1"

  # Setup the build variation depending on the target device
  TARGET_DEVICE="$1"

  if [ -z "$TARGET_DEVICE" ]; then
    echo "INFO: no target device type was specified so using the default 'arm_v7'"
    TARGET_DEVICE="arm_v7"
  fi

  case $TARGET_DEVICE in
    nexus_s)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 arm_version=7 arm_thumb=1"
      DEFINES="${DEFINES} skia_texture_cache_mb_limit=24"
      ;;
    nexus_4 | nexus_7 | nexus_10)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 arm_version=7 arm_thumb=1"
      ;;
    xoom)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 arm_version=7 arm_thumb=1"
      ;;
    galaxy_nexus)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 arm_version=7 arm_thumb=1"
      DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
      ;;
    razr_i)
      DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=32"
      DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
      ;;
    arm_v7)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon_optional=1 arm_version=7 arm_thumb=0"
      ;;
    arm_v7_thumb)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon_optional=1 arm_version=7 arm_thumb=1"
      ;;
    arm)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 arm_thumb=0"
      ;;
    arm_thumb)
      DEFINES="${DEFINES} skia_arch_type=arm arm_neon=0 arm_thumb=1"
      ;;
    x86)
      DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=32"
      DEFINES="${DEFINES} skia_texture_cache_mb_limit=32"
      ;;
    *)
      echo -n "ERROR: unknown device specified ($TARGET_DEVICE), valid values: "
      echo "nexus_[s,4,7,10] xoom galaxy_nexus razr_i arm arm_thumb arm_v7 arm_v7_thumb x86"
      return 1;
      ;;
  esac

  echo "The build is targeting the device: $TARGET_DEVICE"

  # Set up the toolchain.
  setup_toolchain
  DEFINES="${DEFINES} android_toolchain=${TOOLCHAIN_TYPE}"

  # Use the "android" flavor of the Makefile generator for both Linux and OS X.
  exportVar GYP_GENERATORS "make-android"
  exportVar GYP_DEFINES "$DEFINES"
  exportVar SKIA_OUT "out/config/android-${TARGET_DEVICE}"
}

# adb_pull_if_needed(android_src, host_dst)
adb_pull_if_needed() {

  # get adb location
  source $SCRIPT_DIR/utils/setup_adb.sh

  # read input params
  ANDROID_SRC="$1"
  HOST_DST="$2"

  if [ -d $HOST_DST ];
  then
    HOST_DST="${HOST_DST}/$(basename ${ANDROID_SRC})"
  fi

  echo "HOST: $HOST_DST"

  if [ -f $HOST_DST ];
  then
    #get the MD5 for dst and src
    ANDROID_MD5=`$ADB shell md5 $ANDROID_SRC`
    HOST_MD5=`md5sum $HOST_DST`

    if [ "${ANDROID_MD5:0:32}" != "${HOST_MD5:0:32}" ];
    then
      $ADB pull $ANDROID_SRC $HOST_DST
#   else
#      echo "md5 match of android [$ANDROID_SRC] and host [$HOST_DST]"
    fi
  else
    $ADB pull $ANDROID_SRC $HOST_DST
  fi
}
