// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/interfaces/mocks"
)

const workspaceTestWorkspaceDir = "/path/to/workspace"

// This input test data (in textproto format) started as output of
// a bazel cquery call - like:
//
//	bazel cquery --noimplicit_deps 'kind("rule", deps(//:core))' --output textproto > out.txt
//
// and then hand edited to create a small valid query result with specific
// files, copts, and other cc_library/cc_binary rule attributes.
const workspaceTestTextProto = `results {
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

// Return the workspace (cmakeWorkspace) for the test data.
//
// Note: this helper function makes no test assertions as the
// Export function is already tested.
func getTestWorkspace(t *testing.T) (cmakeWorkspace, error) {
	protoData, err := textProtoToProtobuf(workspaceTestTextProto)
	if err != nil {
		return cmakeWorkspace{}, skerr.Wrap(err)
	}

	// Export to CMake buffer.
	fs := mocks.NewFileSystem(t)
	fs.On("OpenFile", mock.Anything).Return(new(bytes.Buffer), nil).Once()
	e := NewCMakeExporter("projName", workspaceTestWorkspaceDir, "CMakeLists.txt", fs)
	qcmd := mocks.NewQueryCommand(t)
	qcmd.On("Read", mock.Anything).Return(protoData, nil)
	err = e.Export(qcmd)
	if err != nil {
		return cmakeWorkspace{}, skerr.Wrap(err)
	}

	return e.workspace, nil
}

func TestIsGetRule_ValidName_ReturnsRule(t *testing.T) {
	workspace, err := getTestWorkspace(t)
	require.NoError(t, err)

	r := workspace.getRule("//src/libs:sum")
	require.NotNil(t, r)
	assert.Equal(t, "//src/libs:sum", r.getName())
}

func TestIsGetRule_InvalidName_ReturnsNil(t *testing.T) {
	workspace, err := getTestWorkspace(t)
	require.NoError(t, err)

	r := workspace.getRule("//non/existent:rule")
	assert.Nil(t, r)
}

func TestCreateRule_ValidBazelRule_NotNil(t *testing.T) {
	workspace, err := getTestWorkspace(t)
	require.NoError(t, err)

	// Get the rule from the existing workspace (since we can't make one)
	cmakeRule := workspace.getRule("//src/libs:sum")
	require.NotNil(t, cmakeRule)

	otherWorkspace := newCMakeWorkspace()
	require.NotNil(t, otherWorkspace)
	otherCMakeRule := otherWorkspace.createRule(cmakeRule.rule)
	require.NotNil(t, otherCMakeRule)
}

func TestIsRuleWritten_NotWritten_ReturnsFalse(t *testing.T) {
	var state writeState
	assert.False(t, state.isRuleWritten("//unwritten:rule"))
}

func TestIsRuleWritten_Written_ReturnsTrue(t *testing.T) {
	var state writeState
	state.setRuleWritten("//written:rule")
	assert.True(t, state.isRuleWritten("//written:rule"))
}
