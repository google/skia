// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/interfaces/mocks"
	"google.golang.org/protobuf/encoding/prototext"
)

const testWorkspaceDir = "/path/to/workspace"
const testCMakeOutFname = "/not/necessarily/in/workspace/CMakeLists.txt"

// This input test data (in textproto format) started as output of
// a bazel cquery call - like:
//
//	bazel cquery --noimplicit_deps 'kind("rule", deps(//:skia_public))' --output textproto
//
// and then hand edited to create a small valid query result with specific
// files, copts, and other cc_library/cc_binary rule attributes.
const textProto = `results {
	target {
		type: RULE
		rule {
		  name: "//src/libs:sum"
		  rule_class: "cc_library"
		  location: "/path/to/workspace/src/libs/BUILD.bazel:8:11"
		  attribute {
			name: "copts"
			type: STRING_LIST
			string_list_value: "-O2"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			name: "defines"
			type: STRING_LIST
			string_list_value: "SUMDEF"
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "includes"
			type: STRING_LIST
			string_list_value: "."
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "linkopts"
			type: STRING_LIST
			string_list_value: "-L/library/dir"
			explicitly_specified: false
			nodep: false
		  }
		  attribute {
			name: "name"
			type: STRING
			string_value: "sum"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			  name: "srcs"
			  type: LABEL_LIST
			  string_list_value: "//src/libs:sum.cpp"
			  explicitly_specified: true
			  nodep: false
		  }
		  attribute {
			name: "hdrs"
			type: LABEL_LIST
			string_list_value: "//src/libs:sum.h"
			explicitly_specified: true
			nodep: false
		  }
		  attribute {
			name: "visibility"
			type: STRING_LIST
			string_list_value: "//visibility:public"
			explicitly_specified: true
			nodep: true
		  }
		}
	  },
	}
	results {
	  target {
	  type: RULE
	  rule {
		name: "//src/apps:hello"
		rule_class: "cc_binary"
		location: "/path/to/workspace/src/apps/BUILD.bazel:8:11"
		attribute {
		  name: "copts"
		  type: STRING_LIST
		  string_list_value: "-O1"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
		  name: "defines"
		  type: STRING_LIST
		  string_list_value: "APPDEF"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "linkopts"
		  type: STRING_LIST
		  string_list_value: "-L/app/dir"
		  explicitly_specified: false
		  nodep: false
		}
		attribute {
		  name: "name"
		  type: STRING
		  string_value: "hello"
		  explicitly_specified: true
		  nodep: false
		}
		attribute {
			name: "srcs"
			type: LABEL_LIST
			string_list_value: "//src/apps:hello-world.cpp"
			explicitly_specified: true
			nodep: false
		}
		attribute {
			name: "deps"
			type: LABEL_LIST
			string_list_value: "//src/libs:sum"
			explicitly_specified: false
			nodep: false
		}
		attribute {
		  name: "visibility"
		  type: STRING_LIST
		  string_list_value: "//visibility:public"
		  explicitly_specified: true
		  nodep: true
		}
	  }
	}
  }`

func TestExport_QueryReadError_ReturnsError(t *testing.T) {
	fs := mocks.NewFileSystem(t)
	e := NewCMakeExporter("projName", testWorkspaceDir, testCMakeOutFname, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return([]byte{}, skerr.Fmt("expected error"))
	err := e.Export(qcmd)
	assert.Error(t, err)
}

func TestExport_InvalidProtobuf_ReturnsError(t *testing.T) {
	fs := mocks.NewFileSystem(t)
	e := NewCMakeExporter("projName", testWorkspaceDir, testCMakeOutFname, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(make([]byte, 50), skerr.Fmt("empty data"))
	err := e.Export(qcmd)
	assert.Error(t, err)
}

func TestExport_ValidProtobuf_Success(t *testing.T) {
	protoData, err := textProtoToProtobuf(textProto)
	require.NoError(t, err)

	var contents bytes.Buffer
	fs := mocks.NewFileSystem(t)
	fs.On("OpenFile", mock.Anything).Once().Run(func(args mock.Arguments) {
		assert.True(t, filepath.IsAbs(args.String(0)))
		assert.Equal(t, args.String(0), testCMakeOutFname)
	}).Return(&contents, nil)
	e := NewCMakeExporter("projName", testWorkspaceDir, testCMakeOutFname, fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil)
	err = e.Export(qcmd)
	require.NoError(t, err)

	// This expected CMake output text is created by hand.
	const expected = `# DO NOT EDIT: This file is auto-generated.
cmake_minimum_required(VERSION 3.13)

project(projName LANGUAGES C CXX)

set(DEFAULT_COMPILE_FLAGS_MACOS "-std=c++17 -Wno-psabi --target=arm64-apple-macos11")
set(DEFAULT_COMPILE_FLAGS_LINUX "-std=c++17 -Wno-psabi -Wno-attributes")

if (APPLE)
  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_MACOS}")
else()
  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_LINUX}")
endif()


# //src/apps:hello
add_executable(src_apps_hello "")
target_sources(src_apps_hello
  PRIVATE
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/apps/hello-world.cpp"
)
set_target_properties(src_apps_hello PROPERTIES COMPILE_FLAGS
  "${DEFAULT_COMPILE_FLAGS} -O1"
)
set_target_properties(src_apps_hello PROPERTIES LINK_FLAGS
  "-L/app/dir"
)
set_target_properties(src_apps_hello PROPERTIES COMPILE_DEFINITIONS
  "APPDEF;SUMDEF"
)
set_target_properties(src_apps_hello PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/src/libs;${CMAKE_SOURCE_DIR}"
)

# //src/libs:sum
add_library(src_libs_sum "")
target_sources(src_libs_sum
  PRIVATE
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/libs/sum.cpp"
    # Headers:
    "${CMAKE_SOURCE_DIR}/src/libs/sum.h"
)
set_target_properties(src_libs_sum PROPERTIES COMPILE_FLAGS
  "${DEFAULT_COMPILE_FLAGS} -O2"
)
set_target_properties(src_libs_sum PROPERTIES LINK_FLAGS
  "-L/library/dir"
)
set_target_properties(src_libs_sum PROPERTIES COMPILE_DEFINITIONS
  "SUMDEF"
)
set_target_properties(src_libs_sum PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/src/libs;${CMAKE_SOURCE_DIR}"
)
`

	assert.Equal(t, expected, contents.String())
}

func TestGetRuleCopts_CoptsExists_Success(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(textProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)

	copts, err := getRuleCopts(r)
	require.NoError(t, err)
	assert.Equal(t, []string{"${DEFAULT_COMPILE_FLAGS}", "-O1"}, copts)
}
