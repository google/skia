# Set up the environment to build Skia for ChromeOS.
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################

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
  # Setup the build variation depending on the target device
  TARGET_DEVICE="${SDK_BOARD}"

  if [ -z "$TARGET_DEVICE" ]; then
    echo "ERROR: No target device specified!"
    return 1
  fi

  DEFINES="OS=linux host_os=linux skia_os=chromeos skia_gpu=0"

  case $TARGET_DEVICE in
    link)
        DEFINES="${DEFINES} skia_arch_type=x86_64"
        GENERIC_BOARD_TYPE="amd64-generic"
        ;;
    daisy)
        DEFINES="${DEFINES} skia_arch_type=arm arm_version=7 arm_neon=1"
        # TODO(borenet): We have to define skia_warnings_as_errors=0 for the arm
        # build, which throws lots of "mangling of va_list has changed" warnings.
        DEFINES="${DEFINES} skia_warnings_as_errors=0"
        GENERIC_BOARD_TYPE="arm-generic"
        ;;
    *)
        echo -n "ERROR: unknown device specified ($TARGET_DEVICE), valid values: "
        echo "x86-alex link daisy"
        return 1;
        ;;
  esac

  echo "The build is targeting the device: $TARGET_DEVICE"

  exportVar GENERIC_BOARD_TYPE ${GENERIC_BOARD_TYPE}
  exportVar GYP_DEFINES "$DEFINES"
  exportVar GYP_GENERATORS "ninja"
  exportVar GYP_GENERATOR_FLAGS ""
  SKIA_OUT=${SKIA_OUT:-out}
  exportVar SKIA_OUT "${SKIA_OUT}/config/chromeos-${TARGET_DEVICE}"
  exportVar builddir_name "."
}
