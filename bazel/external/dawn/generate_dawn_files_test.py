#!/usr/bin/env python3
# Run with: python3 bazel/external/dawn/generate_dawn_files_test.py
import unittest
import os
from generate_dawn_files import CMakeParser, CMakeValue

class TestCMakeParser(unittest.TestCase):
    def setUp(self):
        self.parser = CMakeParser(prefix="TEST_PREFIX", rel_path="src/dawn/test", dawn_root=".")

    def test_clean_path(self):
        # 1. Standard path normalization
        self.assertEqual(self.parser.clean_path("src/dawn/test/File.cpp"), "src/dawn/test/File.cpp")
        self.assertEqual(self.parser.clean_path("src/dawn/test/../File.cpp"), "src/dawn/File.cpp")

        # 2. Stripping variables
        self.assertEqual(self.parser.clean_path("${Dawn_SOURCE_DIR}/src/dawn/File.cpp"), "src/dawn/File.cpp")
        self.assertEqual(self.parser.clean_path("${CMake_SOURCE_DIR}/src/dawn/File.cpp"), "src/dawn/File.cpp")

        # 3. Mapped WebGPU generated headers
        self.assertEqual(
            self.parser.clean_path("webgpu-headers/webgpu_cpp_chained_struct.h"),
            "include/webgpu/webgpu_cpp_chained_struct.h"
        )
        self.assertEqual(
            self.parser.clean_path("webgpu-headers/webgpu.h"),
            "include/dawn/webgpu.h"
        )
        self.assertEqual(
            self.parser.clean_path("webgpu-headers/webgpu_cpp.h"),
            "include/dawn/webgpu_cpp.h"
        )

    def test_append_file_to_group_routing(self):
        # 1. Vulkan Backends
        self.parser._append_file_to_group("src/dawn/native/vulkan/DeviceVk.cpp", is_public_headers=False)
        self.assertIn("src/dawn/native/vulkan/DeviceVk.cpp", self.parser.output_vars["TEST_PREFIX_VULKAN"]["SRCS"])

        # 2. Metal Backends
        self.parser._append_file_to_group("src/dawn/native/metal/DeviceMTL.h", is_public_headers=False)
        self.assertIn("src/dawn/native/metal/DeviceMTL.h", self.parser.output_vars["TEST_PREFIX_METAL"]["PRIV_HDRS"])

        # 3. Android Specifics
        self.parser._append_file_to_group("src/dawn/native/vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.cpp", is_public_headers=False)
        self.assertIn(
            "src/dawn/native/vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.cpp",
            self.parser.output_vars["TEST_PREFIX_ANDROID"]["SRCS"]
        )

        # 4. Apple Platform Files (containing apple, cocoa, objc, osx, iosurface, mac)
        self.parser._append_file_to_group("src/dawn/utils/ObjCUtils.h", is_public_headers=False)
        self.assertIn("src/dawn/utils/ObjCUtils.h", self.parser.output_vars["TEST_PREFIX_APPLE"]["PRIV_HDRS"])

        self.parser._append_file_to_group("src/dawn/common/IOSurfaceUtils.cpp", is_public_headers=False)
        self.assertIn("src/dawn/common/IOSurfaceUtils.cpp", self.parser.output_vars["TEST_PREFIX_APPLE"]["SRCS"])

        # 5. Windows Platform Files
        self.parser._append_file_to_group("src/dawn/common/WindowsUtils.cpp", is_public_headers=False)
        self.assertIn("src/dawn/common/WindowsUtils.cpp", self.parser.output_vars["TEST_PREFIX_WIN32"]["SRCS"])

        # 6. Unix/Posix/X11 Specifics
        self.parser._append_file_to_group("src/dawn/utils/PosixTimer.cpp", is_public_headers=False)
        self.assertIn("src/dawn/utils/PosixTimer.cpp", self.parser.output_vars["TEST_PREFIX_UNIX"]["SRCS"])

        # 7. Null & WebGPU directories
        self.parser._append_file_to_group("src/dawn/native/null/NullBackend.cpp", is_public_headers=False)
        self.assertIn("src/dawn/native/null/NullBackend.cpp", self.parser.output_vars["TEST_PREFIX_NULL"]["SRCS"])

        self.parser._append_file_to_group("src/dawn/native/webgpu/WebGPUBackend.cpp", is_public_headers=False)
        self.assertIn("src/dawn/native/webgpu/WebGPUBackend.cpp", self.parser.output_vars["TEST_PREFIX_WEBGPU"]["SRCS"])

    def test_resolve_var(self):
        # Bind standard variable
        self.parser.variables["test_var"] = [CMakeValue("File1.cpp"), CMakeValue("File2.cpp")]
        resolved = [cv.value for cv in self.parser.resolve_var("test_var")]
        self.assertEqual(resolved, ["File1.cpp", "File2.cpp"])

        # Recursive variable
        self.parser.variables["nested_var"] = [CMakeValue("${test_var}")]
        resolved_nested = [cv.value for cv in self.parser.resolve_var("nested_var")]
        self.assertEqual(resolved_nested, ["File1.cpp", "File2.cpp"])

    def test_parse_set_and_list_commands(self):
        cmake_content = """
            set(test_sources
                "File1.cpp"
                "File2.cpp"
            )
            list(APPEND test_sources
                "File3.cpp"
            )
        """
        self.parser.parse(cmake_content)
        resolved = [cv.value for cv in self.parser.resolve_var("test_sources")]
        self.assertEqual(resolved, ["File1.cpp", "File2.cpp", "File3.cpp"])

if __name__ == '__main__':
    unittest.main()
