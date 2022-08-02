// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

func TestMakeCanonicalRuleName_ValidInput_Success(t *testing.T) {
	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			actual, err := makeCanonicalRuleName(input)
			require.NoError(t, err)
			assert.Equal(t, expected, actual)
		})
	}

	test("AlreadyCanonicalNoPath", "//:skia_public", "//:skia_public")
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

	test("TargetAtRoot", "//:skia_public", "", "/", "skia_public")
	test("PathWithTarget", "//foo/bar:wiz", "", "/foo/bar", "wiz")
	test("PathWithFile", "@abseil_cpp//absl/algorithm:algorithm.h", "@abseil_cpp", "/absl/algorithm", "algorithm.h")
	test("DirDefaultTarget", "//tools/flags", "", "/tools/flags", "flags")
	test("RepoDefaultTarget", "@libpng", "@libpng", "/", "libpng")
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

func TestGetRuleCMakeName_ValidInput_Success(t *testing.T) {
	test := func(name, rule, expectedName string) {
		t.Run(name, func(t *testing.T) {
			cmakeName, err := getRuleCMakeName(rule)
			require.NoError(t, err)
			assert.Equal(t, expectedName, cmakeName)
		})
	}

	test("PathWithTarget", "//include/private/chromium:private_hdrs", "include_private_chromium_private_hdrs")
	test("PathWithHost", "@repo//path/to/dir:file.txt", "at_repo_path_to_dir_file.txt")
	test("RootTarget", "//:skia_public", "skia_public")
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
