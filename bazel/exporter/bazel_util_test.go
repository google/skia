// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"google.golang.org/protobuf/encoding/prototext"
)

func TestMakeCanonicalRuleName_ValidInput_Success(t *testing.T) {
	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			actual, err := makeCanonicalRuleName(input)
			require.NoError(t, err)
			assert.Equal(t, expected, actual)
		})
	}

	test("AlreadyCanonicalNoPath", "//:core", "//:core")
	test("AlreadyCanonicalWithPath", "//foo/bar:wiz", "//foo/bar:wiz")
	test("NoRepoDefaultTarget", "//tools/flags", "//tools/flags:flags")
	test("RepoWithDefaultTarget", "@libpng", "@libpng//:libpng")
}

func TestMakeCanonicalRuleName_InvalidInput_ReturnError(t *testing.T) {
	test := func(name, input string) {
		t.Run(name, func(t *testing.T) {
			_, err := makeCanonicalRuleName(input)
			assert.Error(t, err)
		})
	}

	test("EmptyString", "")
	test("InvalidRepoName", "@@repo")
	test("InvalidTargetName", "//:::target_name")
}

func TestParseRule_ValidRules_Success(t *testing.T) {

	test := func(name, rule, expectedRepo, expectedPath, expectedTarget string) {
		t.Run(name, func(t *testing.T) {
			repo, path, target, err := parseRule(rule)
			require.NoError(t, err)
			assert.Equal(t, expectedRepo, repo)
			assert.Equal(t, expectedPath, path)
			assert.Equal(t, expectedTarget, target)
		})
	}

	test("TargetAtRoot", "//:core", "", "/", "core")
	test("PathWithTarget", "//foo/bar:wiz", "", "/foo/bar", "wiz")
	test("PathWithFile", "@abseil_cpp//absl/algorithm:algorithm.h", "@abseil_cpp", "/absl/algorithm", "algorithm.h")
	test("DirDefaultTarget", "//tools/flags", "", "/tools/flags", "flags")
	test("RepoDefaultTarget", "@libpng", "@libpng", "/", "libpng")
	test("TargetWithPath", "//src/sksl:generated/sksl_compute.dehydrated.sksl", "", "/src/sksl", "generated/sksl_compute.dehydrated.sksl")
}

func TestParseLocation_ValidInput_Success(t *testing.T) {
	path, line, pos, err := parseLocation("/path/to/file.txt:12:875")
	require.NoError(t, err)
	assert.Equal(t, "/path/to/file.txt", path)
	assert.Equal(t, 12, line)
	assert.Equal(t, 875, pos)
}

func TestGetLocationDir_ValidInput_Success(t *testing.T) {
	path, err := getLocationDir("/path/to/file.txt:12:875")
	require.NoError(t, err)
	assert.Equal(t, "/path/to", path)
}

func TestGetRuleSimpleName_ValidInput_Success(t *testing.T) {
	test := func(name, rule, expectedName string) {
		t.Run(name, func(t *testing.T) {
			cmakeName, err := getRuleSimpleName(rule)
			require.NoError(t, err)
			assert.Equal(t, expectedName, cmakeName)
		})
	}

	test("PathWithTarget", "//include/private/chromium:private_hdrs", "include_private_chromium_private_hdrs")
	test("PathWithHost", "@repo//path/to/dir:file.txt", "at_repo_path_to_dir_file.txt")
	test("RootTarget", "//:core", "core")
	test("HostTarget", "@repo//:deps", "at_repo_deps")
	test("DirDefaultTarget", "//tools/flags", "tools_flags_flags") // Input rule shorthand for "//tools/flags:flags".
	test("RepoDefaultTarget", "@libpng", "at_libpng_libpng")       // Input rule shorthand for "@libpng//:libpng".
}

func TestGetRuleStringArrayAttribute_NoAttrib_ReturnsNilSlice(t *testing.T) {
	var rule build.Rule
	slice, err := getRuleStringArrayAttribute(&rule, "missing-attrib")
	assert.NoError(t, err)
	assert.Empty(t, slice)
}

func TestAppendUnique_NotPresent_Appended(t *testing.T) {
	slice := appendUnique([]string{"one"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}

func TestAppendUnique_Present_NotAppended(t *testing.T) {
	slice := appendUnique([]string{"one", "two"}, "two")
	assert.Equal(t, []string{"one", "two"}, slice)
}

func TestIsExternalRule_IsExternal_ExpectTrue(t *testing.T) {
	assert.True(t, isExternalRule("@abseil_cpp//absl/algorithm:algorithm.h"))
}

func TestIsExternalRule_IsInternal_ExpectFalse(t *testing.T) {
	assert.False(t, isExternalRule("//:core"))
}

func TestIsFileRule_InvalidRule_ReturnsFalse(t *testing.T) {
	assert.False(t, isFileTarget(""))
}

func TestIsFileRule_ValidFileRule_ReturnsTrue(t *testing.T) {
	assert.True(t, isFileTarget("//dir/path:hello.c"))
}

func TestIsFileRule_ValidNonFileRule_ReturnsFalse(t *testing.T) {
	assert.False(t, isFileTarget("//dir/path:hello"))
}

func TestFindRule_RuleExists_Success(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(textProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)
	assert.Equal(t, "//src/apps:hello", r.GetName())
}

func TestFindRule_RuleDoesntExists_ReturnsNil(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(textProto), &qr)
	require.NoError(t, err)

	assert.Nil(t, findRule(&qr, "//path/to:nonexistent_rule"))
}

func TestFindRule_InvalidRule_ReturnsNil(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(textProto), &qr)
	require.NoError(t, err)

	assert.Nil(t, findRule(&qr, ""))
}

func TestGetFilePathFromFileTarget_ValidPaths_ReturnsPath(t *testing.T) {
	test := func(name, target, expected string) {
		t.Run(name, func(t *testing.T) {
			path, err := getFilePathFromFileTarget(target)
			require.NoError(t, err)
			assert.Equal(t, expected, path)
		})
	}

	test("FileInDir", "//src/core:source.cpp", "src/core/source.cpp")
	test("RootFile", "//:source.cpp", "source.cpp")
}

func TestGetFilePathFromFileTarget_InvalidTarget_ReturnsError(t *testing.T) {
	test := func(name, target string) {
		t.Run(name, func(t *testing.T) {
			_, err := getFilePathFromFileTarget(target)
			require.Error(t, err)
		})
	}

	test("EmptyString", "")
	test("InvalidTarget", "//")
	test("NoFile", "//src/core:srcs")
	test("DefaultTarget", "//src/core")
}
