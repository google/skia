# Set up the environment to build Skia for ChromeOS.

function exportVar {
  NAME=$1
  VALUE=$2
  echo export $NAME=\"$VALUE\"
  export $NAME="$VALUE"
}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Helper function to configure the GYP defines to the appropriate values
# based on the target device.
setup_device() {
  DEFINES="OS=linux"
  DEFINES="${DEFINES} host_os=$(uname -s | sed -e 's/Linux/linux/;s/Darwin/mac/')"
  DEFINES="${DEFINES} skia_os=chromeos"

  # TODO(borenet): We have to define skia_warnings_as_errors=0 for the arm
  # build, which throws lots of "mangling of va_list has changed" warnings.
  DEFINES="${DEFINES} skia_warnings_as_errors=0"

  # Setup the build variation depending on the target device
  TARGET_DEVICE="$1"

  if [ -z "$TARGET_DEVICE" ]; then
    echo "INFO: no target device type was specified so using the default 'x86-generic'"
    TARGET_DEVICE="x86-generic"
  fi

  # Toolchain prefixes.
  X86_TOOLCHAIN_PREFIX="i686-pc-linux-gnu"
  AMD64_TOOLCHAIN_PREFIX="x86_64-cros-linux-gnu"
  ARMV7_TOOLCHAIN_PREFIX="armv7a-cros-linux-gnueabi"

  case $TARGET_DEVICE in
    x86-generic)
        DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=32"
        CHROMEOS_TOOLCHAIN_TYPE=${X86_TOOLCHAIN_PREFIX}
        BOARD_TYPE="x86-generic"
        ;;
    amd64-generic)
        DEFINES="${DEFINES} skia_arch_type=x86 skia_arch_width=64"
        CHROMEOS_TOOLCHAIN_TYPE=${AMD64_TOOLCHAIN_PREFIX}
        BOARD_TYPE="amd64-generic"
        ;;
    arm-generic)
        DEFINES="${DEFINES} skia_arch_type=arm arm_neon=1 armv7=1 arm_thumb=0 skia_arch_width=32"
        CHROMEOS_TOOLCHAIN_TYPE=${ARMV7_TOOLCHAIN_PREFIX}
        BOARD_TYPE="arm-generic"
        ;;
    *)
        echo -n "ERROR: unknown device specified ($TARGET_DEVICE), valid values: "
        echo "x86-generic amd64-generic arm-generic"
        return 1;
        ;;
  esac

  DEFINES="${DEFINES} skia_cros_target=${BOARD_TYPE}"

  CHROMEOS_TOOLCHAIN_PREFIX="/usr/bin/${CHROMEOS_TOOLCHAIN_TYPE}"
  exportVar AR "$CHROMEOS_TOOLCHAIN_PREFIX-ar"
  if [[ -z "$CHROMEOS_MAKE_CCACHE" ]]; then
    exportVar CC "$CHROMEOS_TOOLCHAIN_PREFIX-gcc"
    exportVar CXX "$CHROMEOS_TOOLCHAIN_PREFIX-g++"
    exportVar LINK "$CHROMEOS_TOOLCHAIN_PREFIX-gcc"
  else
    exportVar CC "$CHROMEOS_MAKE_CCACHE $CHROMEOS_TOOLCHAIN_PREFIX-gcc"
    exportVar CXX "$CHROMEOS_MAKE_CCACHE $CHROMEOS_TOOLCHAIN_PREFIX-g++"
    exportVar LINK "$CHROMEOS_MAKE_CCACHE $CHROMEOS_TOOLCHAIN_PREFIX-gcc"
  fi
  exportVar RANLIB "$CHROMEOS_TOOLCHAIN_PREFIX-ranlib"
  exportVar OBJCOPY "$CHROMEOS_TOOLCHAIN_PREFIX-objcopy"
  exportVar STRIP "$CHROMEOS_TOOLCHAIN_PREFIX-strip"

  echo "The build is targeting the device: $TARGET_DEVICE"

  BUILD_PREFIX="/build/${BOARD_TYPE}"

  exportVar C_INCLUDE_PATH "${BUILD_PREFIX}/usr/include"
  exportVar CPLUS_INCLUDE_PATH "${BUILD_PREFIX}/usr/include"
  exportVar LIBRARY_PATH "${BUILD_PREFIX}/usr/lib"
  exportVar LD_LIBRARY_PATH "${BUILD_PREFIX}/usr/lib"

  exportVar GYP_DEFINES "$DEFINES"
  exportVar SKIA_OUT "out/config/chromeos-${TARGET_DEVICE}"
}
