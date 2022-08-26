// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/interfaces/mocks"
	"google.golang.org/protobuf/encoding/prototext"
)

// These test query results are generated with the following command:
//
//	bazel cquery --noimplicit_deps \
//	   'kind("rule", deps(//src/core:core_srcs) + deps(//src/opts:private_hdrs))' \
//	   --output textproto
//
// Then the output data is manually pruned to include only the first three files.
const publicSrcsTextProto = `results {
	target {
	  type: RULE
	  rule {
		name: "//src/core:core_srcs"
		rule_class: "filegroup"
		location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
		attribute {
		  name: "$config_dependencies"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: ":action_listener"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "applicable_licenses"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "aspect_hints"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "data"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "deprecation"
		  type: STRING
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "distribs"
		  type: DISTRIBUTION_SET
		  string_list_value: "INTERNAL"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "features"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_function"
		  type: STRING
		  string_value: "split_srcs_and_hdrs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "generator_location"
		  type: STRING
		  string_value: "src/core/BUILD.bazel:397:20"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "generator_name"
		  type: STRING
		  string_value: "core"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "licenses"
		  type: LICENSE
		  license {
			license_type: "NOTICE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "core_srcs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "output_group"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "output_licenses"
		  type: LICENSE
		  license {
			license_type: "NONE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "path"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "restricted_to"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "srcs"
		  type: LABEL_LIST
		  string_list_value: "//src/core:SkAAClip.cpp"
		  string_list_value: "//src/core:SkATrace.cpp"
		  string_list_value: "//src/core:SkAlphaRuns.cpp"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "tags"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "target_compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "testonly"
		  type: BOOLEAN
		  int_value: 0
		  string_value: "false"
		  explicitly_specified: false
		  boolean_value: false
		}
		attribute {
		  name: "transitive_configs"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		rule_input: "//src/core:SkAAClip.cpp"
		rule_input: "//src/core:SkATrace.cpp"
		rule_input: "//src/core:SkAlphaRuns.cpp"
	  }
	}
	configuration {
	  checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
	}
  }
  results {
	target {
	  type: RULE
	  rule {
		name: "//src/opts:private_hdrs"
		rule_class: "filegroup"
		location: "/path/to/skia/src/src/opts/BUILD.bazel:26:10"
		attribute {
		  name: "$config_dependencies"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: ":action_listener"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "applicable_licenses"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "aspect_hints"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "data"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "deprecation"
		  type: STRING
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "distribs"
		  type: DISTRIBUTION_SET
		  string_list_value: "INTERNAL"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "features"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_function"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_location"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "generator_name"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "licenses"
		  type: LICENSE
		  license {
			license_type: "NOTICE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "private_hdrs"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "output_group"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "output_licenses"
		  type: LICENSE
		  license {
			license_type: "NONE"
		  }
		  explicitly_specified: false
		}
		attribute {
		  name: "path"
		  type: STRING
		  string_value: ""
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "restricted_to"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "srcs"
		  type: LABEL_LIST
		  string_list_value: "//src/opts:SkBitmapProcState_opts.h"
		  string_list_value: "//src/opts:SkBlitMask_opts.h"
		  string_list_value: "//src/opts:SkBlitRow_opts.h"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "tags"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "target_compatible_with"
		  type: LABEL_LIST
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "testonly"
		  type: BOOLEAN
		  int_value: 0
		  string_value: "false"
		  explicitly_specified: false
		  boolean_value: false
		}
		attribute {
		  name: "transitive_configs"
		  type: STRING_LIST
		  explicitly_specified: false
		  nodep: true
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  string_list_value: "//src:__pkg__"
		  explicitly_specified: true
		  nodep: true
		}
		rule_input: "//src/opts:SkBitmapProcState_opts.h"
		rule_input: "//src/opts:SkBlitMask_opts.h"
		rule_input: "//src/opts:SkBlitRow_opts.h"
	  }
	}
	configuration {
	  checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
	}
  }
`

// The expected gn/core.gni file contents for publicSrcsTextProto.
// This expected result is handmade.
const publicSrcsExpectedGNI = `# DO NOT EDIT: This is a generated file.

_src = get_path_info("../src", "abspath")

# //src/core:core_srcs
skia_core_sources = [
  "$_src/core/SkAAClip.cpp",
  "$_src/core/SkATrace.cpp",
  "$_src/core/SkAlphaRuns.cpp",
  "$_src/opts/SkBitmapProcState_opts.h",
  "$_src/opts/SkBlitMask_opts.h",
  "$_src/opts/SkBlitRow_opts.h",
]

skia_core_sources += skia_pathops_sources
skia_core_sources += skia_skpicture_sources

skia_core_public += skia_pathops_public
skia_core_public += skia_skpicture_public
`

var fileListWriteOrder = map[string][]string{
	"gn/core.gni": {
		"skia_core_sources",
	},
}

var testExporterParams = GNIExporterParams{
	WorkspaceDir: "/path/to/workspace",
	GNIFileVars:  fileListWriteOrder,
}

func TestGNIExporterExport_ValidInput_Success(t *testing.T) {
	protoData, err := textProtoToProtobuf(publicSrcsTextProto)
	require.NoError(t, err)

	fs := mocks.NewFileSystem(t)
	var contents bytes.Buffer
	fs.On("OpenFile", mock.Anything).Once().Run(func(args mock.Arguments) {
		path := args.String(0)
		assert.True(t, filepath.IsAbs(path))
		assert.Equal(t, "/path/to/workspace/gn/core.gni", filepath.ToSlash(path))
	}).Return(&contents, nil).Once()
	e := NewGNIExporter(testExporterParams, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil).Once()
	err = e.Export(qcmd)
	require.NoError(t, err)

	assert.Equal(t, publicSrcsExpectedGNI, contents.String())
}

func TestGNIExporterCheckCurrent_CurrentData_ReturnZero(t *testing.T) {
	fs := mocks.NewFileSystem(t)
	fs.On("ReadFile", mock.Anything).Run(func(args mock.Arguments) {
		path := args.String(0)
		assert.True(t, filepath.IsAbs(path))
		assert.Equal(t, "/path/to/workspace/gn/core.gni", filepath.ToSlash(path))
	}).Return([]byte(publicSrcsExpectedGNI), nil)
	e := NewGNIExporter(testExporterParams, fs)
	qcmd := mocks.NewQueryCommand(t)
	var errBuff bytes.Buffer
	numOutOfDate, err := e.CheckCurrent(qcmd, &errBuff)
	os.Stdout.Write(errBuff.Bytes()) // Echo output messages to stdout.
	assert.NoError(t, err)
	assert.Zero(t, numOutOfDate)
}

func TestMakeRelativeFilePathForGNI_MatchingRootDir_Success(t *testing.T) {
	test := func(name, target, expectedPath string) {
		t.Run(name, func(t *testing.T) {
			path, err := makeRelativeFilePathForGNI(target)
			require.NoError(t, err)
			assert.Equal(t, expectedPath, path)
		})
	}

	test("src", "src/core/file.cpp", "$_src/core/file.cpp")
	test("include", "include/core/file.h", "$_include/core/file.h")
	test("modules", "modules/mod/file.cpp", "$_modules/mod/file.cpp")
}

func TestMakeRelativeFilePathForGNI_IndalidInput_ReturnError(t *testing.T) {
	test := func(name, target string) {
		t.Run(name, func(t *testing.T) {
			_, err := makeRelativeFilePathForGNI(target)
			assert.Error(t, err)
		})
	}

	test("EmptyString", "")
	test("UnsupportedRootDir", "//valid/rule/incorrect/root/dir:file.cpp")
}

func TestGetRuleGNIVariableName_ValidRuleName_Success(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(publicSrcsTextProto), &qr)
	require.NoError(t, err)
	r := findRule(&qr, "//src/core:core_srcs")
	require.NotNil(t, r)
	name, err := getRuleGNIVariableName(r)
	require.NoError(t, err)
	assert.Equal(t, "skia_core_sources", name)
}

func TestIsTargetCppHeaderFile_ValidHeaderFileNames_ReturnTrue(t *testing.T) {
	test := func(name, target string) {
		t.Run(name, func(t *testing.T) {
			assert.True(t, isTargetCppHeaderFile(target))
		})
	}

	test("LowerH", "//include/core:file.h")
	test("UpperH", "//include/core:file.H")
	test("MixedHpp", "//include/core:file.Hpp")
}

func TestIsTargetCppHeaderFile_InvalidHeaderFileNames_ReturnFalse(t *testing.T) {
	test := func(name, target string) {
		t.Run(name, func(t *testing.T) {
			assert.False(t, isTargetCppHeaderFile(target))
		})
	}

	test("SourceFile", "//src/core:file.cpp")
	test("InvalidSfx", "//src/core:file.HH")
}

func TestShouldSkipRule_ShouldNotSkip_ReturnFalse(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(publicSrcsTextProto), &qr)
	require.NoError(t, err)
	r := findRule(&qr, "//src/core:core_srcs")
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.False(t, skip)
}

func TestShouldSkipRule_CoreGNI_ReturnTrue(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//src/opts:srcs"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
    }
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//src/opts:srcs")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.True(t, skip)
}

func TestShouldSkipRule_ExcludedPrivateHeaders_ReturnFalse(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/private:private_hdrs"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//visibility:private"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/private:private_hdrs")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.False(t, skip)
}

func TestShouldSkipRule_PublicInName_ReturnFalse(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/codec:public_hdrs"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//visibility:private"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/codec:public_hdrs")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.False(t, skip)
}

func TestShouldSkipRule_PublicVisibility_ReturnFalse(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/codec:include"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//visibility:public"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/codec:include")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.False(t, skip)
}

func TestShouldSkipRule_PackageVisibilityNoSource_ReturnTrue(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/codec:include"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//:__pkg__"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/codec:include")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.True(t, skip)
}

func TestShouldSkipRule_PackageVisibilitySrcsWithHdr_ReturnFalse(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/codec:sources"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
	  attribute {
	    name: "srcs"
	    type: LABEL_LIST
	    string_list_value: "//src/core:SkAAClip.cpp"
	    string_list_value: "//src/core:SkATrace.h"
	    string_list_value: "//src/core:SkAlphaRuns.cpp"
	    explicitly_specified: true
	    nodep: false
	  }
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//:__pkg__"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/codec:sources")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.False(t, skip)
}

func TestShouldSkipRule_PackageVisibilitySrcsOnlyHeaders_ReturnTrue(t *testing.T) {
	// hand-crafted textproto to contain a single rule for test.
	const ruleProto = `results {
  target {
    type: RULE
    rule {
      name: "//include/codec:sources"
      rule_class: "filegroup"
      location: "/path/to/skia/src/src/core/BUILD.bazel:397:20"
	  attribute {
	    name: "srcs"
	    type: LABEL_LIST
	    string_list_value: "//src/core:SkAAClip.h"
	    string_list_value: "//src/core:SkATrace.h"
	    string_list_value: "//src/core:SkAlphaRuns.h"
	    explicitly_specified: true
	    nodep: false
	  }
      attribute {
        name: "visibility"
        type: STRING_LIST
        string_list_value: "//:__pkg__"
        explicitly_specified: true
        nodep: true
      }
	}
  }
  configuration {
    checksum: "d16aa11033851c6aac7f80d42f69ce16f44935bba14f1c71f7cfc07b0d4d60b2"
  }
}`

	r, err := unmarshalAndGetRule(ruleProto, "//include/codec:sources")
	require.NoError(t, err)
	require.NotNil(t, r)
	skip, err := shouldSkipRule(r)
	require.NoError(t, err)
	assert.True(t, skip)
}

func TestAbsToWorkspacePath_ReturnsRelativePath(t *testing.T) {
	fs := mocks.NewFileSystem(t)
	e := NewGNIExporter(testExporterParams, fs)
	require.NotNil(t, e)

	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, e.absToWorkspacePath(input))
		})
	}

	test("FileInDir", "/path/to/workspace/foo/bar.txt", "foo/bar.txt")
	test("DirInDr", "/path/to/workspace/foo/bar", "foo/bar")
	test("RootFile", "/path/to/workspace/root.txt", "root.txt")
	test("WorkspaceDir", "/path/to/workspace", "")
	test("WorkspaceWithSlash", "/path/to/workspace/", "")
}

func TestWorkspaceToAbsPath_ReturnsAbsolutePath(t *testing.T) {
	fs := mocks.NewFileSystem(t)
	e := NewGNIExporter(testExporterParams, fs)
	require.NotNil(t, e)

	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, e.workspaceToAbsPath(input))
		})
	}

	test("FileInDir", "foo/bar.txt", "/path/to/workspace/foo/bar.txt")
	test("DirInDir", "foo/bar", "/path/to/workspace/foo/bar")
	test("RootFile", "root.txt", "/path/to/workspace/root.txt")
	test("WorkspaceDir", "", "/path/to/workspace")
}

func TestGetFileData_SamePath_ReturnSamePtr(t *testing.T) {
	e := NewGNIExporter(testExporterParams, mocks.NewFileSystem(t))
	require.NotNil(t, e)
	first := e.getFileData("gn/anything.gni")
	second := e.getFileData("gn/anything.gni")
	assert.Equal(t, first, second)
}

func TestGetFileData_DifferentPath_ReturnDifferentPtr(t *testing.T) {
	e := NewGNIExporter(testExporterParams, mocks.NewFileSystem(t))
	require.NotNil(t, e)
	first := e.getFileData("gn/anything.gni")
	second := e.getFileData("gn/another.gni")
	assert.NotEqual(t, first, second)
}

func TestGetGNILineVariable_LinesWithVariables_ReturnVariable(t *testing.T) {
	test := func(name, inputLine, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, getGNILineVariable(inputLine))
		})
	}

	test("EqualWithSpaces", `foo = [ "something" ]`, "foo")
	test("EqualNoSpaces", `foo=[ "something" ]`, "foo")
	test("EqualSpaceBefore", `foo =[ "something" ]`, "foo")
	test("MultilineList", `foo = [`, "foo")
}

func TestGetGNILineVariable_LinesWithVariables_NoMatch(t *testing.T) {
	test := func(name, inputLine, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, getGNILineVariable(inputLine))
		})
	}

	test("FirstCharSpace", ` foo = [ "something" ]`, "") // Impl. requires formatted file.
	test("NotList", `foo = bar`, "")
	test("ListLiteral", `[ "something" ]`, "")
	test("ListInComment", `# foo = [ "something" ]`, "")
	test("MissingVariable", `=[ "something" ]`, "")
	test("EmptyString", ``, "")
}

func TestShouldExportFile(t *testing.T) {
	writeOrder := map[string][]string{
		"gn/core.gni": {
			"skia_core_public"},
		"gn/effects.gni": {
			"skia_effects_public",
			"skia_effects_sources"},
	}

	exporterParams := GNIExporterParams{
		WorkspaceDir: "/path/to/workspace",
		GNIFileVars:  writeOrder,
	}

	e := NewGNIExporter(exporterParams, mocks.NewFileSystem(t))

	test := func(name, filepath string, exported bool) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, exported, e.shouldExportFile(filepath))
		})
	}

	test("Core", "gn/core.gni", true)
	test("Core", "gn/effects.gni", true)
	test("Core", "gn/effects_imagefilters.gni", false)
}

func TestGetLocationGNFilePath_MatchingPaths_ReturnExpected(t *testing.T) {

	writeOrder := map[string][]string{
		"gn/core.gni": {
			"skia_core_sources",
		},
		"gn/effects.gni": {
			"skia_effects_public",
			"skia_effects_sources"},
	}

	exporterParams := GNIExporterParams{
		WorkspaceDir: "/path/to/workspace",
		GNIFileVars:  writeOrder,
	}

	fs := mocks.NewFileSystem(t)
	e := NewGNIExporter(exporterParams, fs)
	require.NotNil(t, e)

	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, e.getLocationGNFilePath(input))
		})
	}

	test("FullPatch", "/path/to/workspace/include/core", "gn/core.gni")
	// TODO(skbug.com/12345): When all *.gni files are supported this becomes an error.
	test("NoExport", "/path/to/workspace/src/effects/imagefilters", defaultGNI)
}

func TestFoo_DeprecatedFiles_ReturnsTrue(t *testing.T) {
	assert.True(t, isSourceFileDeprecated("include/core/SkDrawLooper.h"))
}

func TestFoo_NotDeprecatedFiles_ReturnsFalse(t *testing.T) {
	assert.False(t, isSourceFileDeprecated("include/core/SkColor.h"))
}

func TestExtractTopLevelFolder_PathsWithTopDir_ReturnsTopDir(t *testing.T) {
	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, extractTopLevelFolder(input))
		})
	}
	test("TopIsDir", "foo/bar/baz.txt", "foo")
	test("TopIsVariable", "$_src/bar/baz.txt", "$_src")
	test("TopIsFile", "baz.txt", "baz.txt")
	test("TopIsAbsDir", "/foo/bar/baz.txt", "")
}

func TestExtractTopLevelFolder_PathsWithNoTopDir_ReturnsEmptyString(t *testing.T) {
	test := func(name, input, expected string) {
		t.Run(name, func(t *testing.T) {
			assert.Equal(t, expected, extractTopLevelFolder(input))
		})
	}
	test("EmptyString", "", "")
	test("EmptyAbsRoot", "/", "")
	test("MultipleSlashes", "///", "")
}

func TestAddGNIVariablesToWorkspacePaths_ValidInput_ReturnsVariables(t *testing.T) {
	test := func(name string, inputPaths, expected []string) {
		t.Run(name, func(t *testing.T) {
			gniPaths, err := addGNIVariablesToWorkspacePaths(inputPaths)
			require.NoError(t, err)
			assert.Equal(t, expected, gniPaths)
		})
	}
	test("EmptySlice", nil, []string{})
	test("AllVariables",
		[]string{"src/include/foo.h",
			"include/foo.h",
			"modules/foo.cpp"},
		[]string{"$_src/include/foo.h",
			"$_include/foo.h",
			"$_modules/foo.cpp"})
}

func TestAddGNIVariablesToWorkspacePaths_InvalidInput_ReturnsError(t *testing.T) {
	test := func(name string, inputPaths []string) {
		t.Run(name, func(t *testing.T) {
			_, err := addGNIVariablesToWorkspacePaths(inputPaths)
			assert.Error(t, err)
		})
	}
	test("InvalidTopDir", []string{"nomatch/include/foo.h"})
	test("RuleNotPath", []string{"//src/core:source.cpp"})
}

func TestConvertTargetsToFilePaths_ValidInput_ReturnsPaths(t *testing.T) {
	test := func(name string, inputTargets, expected []string) {
		t.Run(name, func(t *testing.T) {
			paths, err := convertTargetsToFilePaths(inputTargets)
			require.NoError(t, err)
			assert.Equal(t, expected, paths)
		})
	}
	test("EmptySlice", nil, []string{})
	test("Files",
		[]string{"//src/include:foo.h",
			"//include:foo.h",
			"//modules:foo.cpp"},
		[]string{"src/include/foo.h",
			"include/foo.h",
			"modules/foo.cpp"})
}

func TestConvertTargetsToFilePaths_InvalidInput_ReturnsError(t *testing.T) {
	test := func(name string, inputTargets []string) {
		t.Run(name, func(t *testing.T) {
			_, err := convertTargetsToFilePaths(inputTargets)
			assert.Error(t, err)
		})
	}
	test("EmptyString", []string{""})
	test("ValidTargetEmptyString", []string{"//src/include:foo.h", ""})
	test("EmptyStringValidTarget", []string{"//src/include:foo.h", ""})
}

func TestFilterDeprecatedFiles_ContainsDeprecatedFiles_DeprecatedFiltered(t *testing.T) {
	test := func(name string, inputFiles, expected []string) {
		t.Run(name, func(t *testing.T) {
			paths := filterDeprecatedFiles(inputFiles)
			assert.Equal(t, expected, paths)
		})
	}
	test("OneDeprecated",
		[]string{"include/core/SkDrawLooper.h"},
		[]string{})
	test("MultipleDeprecated",
		[]string{
			"include/core/SkDrawLooper.h",
			"include/effects/SkBlurDrawLooper.h"},
		[]string{})
	test("FirstDeprecated",
		[]string{
			"include/core/SkDrawLooper.h",
			"not/deprecated/file.h"},
		[]string{"not/deprecated/file.h"})
	test("LastDeprecated",
		[]string{
			"not/deprecated/file.h",
			"include/core/SkDrawLooper.h"},
		[]string{"not/deprecated/file.h"})
}

func TestFilterDeprecatedFiles_NoDeprecatedFiles_SliceUnchanged(t *testing.T) {
	test := func(name string, inputFiles, expected []string) {
		t.Run(name, func(t *testing.T) {
			paths := filterDeprecatedFiles(inputFiles)
			assert.Equal(t, expected, paths)
		})
	}
	test("EmptySlice", nil, []string{})
	test("NoneDeprecated",
		[]string{
			"not/deprecated/file.h",
			"also/not/deprecated/file.h"},
		[]string{
			"not/deprecated/file.h",
			"also/not/deprecated/file.h"})
}
