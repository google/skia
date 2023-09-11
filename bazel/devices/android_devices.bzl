"""This module defines the ANDROID_DEVICES dictionary."""

# Dictionary from device names to platform constraints (excluding the OS).
ANDROID_DEVICES = {
    "pixel_5": ["@platforms//cpu:arm64"],
    "pixel_7": ["@platforms//cpu:arm64"],
}
