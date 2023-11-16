// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"google.golang.org/protobuf/encoding/prototext"
)

// This input test data (in textproto format) started as output of
// a bazel cquery call - like:
//
//	bazel cquery --noimplicit_deps 'kind("rule", deps(//:skia_public))' --output textproto > out.txt
//
// and then hand edited to create a small valid query result with specific
// files, copts, and other cc_library/cc_binary rule attributes.
const ruleTestTextProto = `results {
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

func TestGetName_MatchingValue(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(ruleTestTextProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)
	assert.Equal(t, "//src/apps:hello", r.GetName())

	cr := newCMakeRule(r)
	assert.Equal(t, r.GetName(), cr.getName())
}

func TestHasSrcs_SourcesExist_ReturnsTrue(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(ruleTestTextProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/libs:sum")
	require.NotNil(t, r)
	assert.Equal(t, "//src/libs:sum", r.GetName())

	cr := newCMakeRule(r)
	assert.True(t, cr.hasSrcs())
}

func TestHasSrcs_NoSources_ReturnsFalse(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(ruleTestTextProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)
	assert.Equal(t, "//src/apps:hello", r.GetName())

	cr := newCMakeRule(r)
	assert.False(t, cr.hasSrcs())
}

func TestHasDependency_DependencyExist_ReturnsTrue(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(ruleTestTextProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)
	assert.Equal(t, "//src/apps:hello", r.GetName())

	cr := newCMakeRule(r)
	require.NoError(t, cr.addDependency("//dependency/name"))
	assert.True(t, cr.hasDependency("//dependency/name"))
}

func TestHasDependency_NoDependency_ReturnsFalse(t *testing.T) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(ruleTestTextProto), &qr)
	require.NoError(t, err)

	r := findRule(&qr, "//src/apps:hello")
	require.NotNil(t, r)
	assert.Equal(t, "//src/apps:hello", r.GetName())

	cr := newCMakeRule(r)
	assert.False(t, cr.hasDependency("//dependency/name"))
}

func TestSetContents_SetContents_WriteSucceeds(t *testing.T) {
	cr := newCMakeRule(nil)
	cr.setContents([]byte("Contents Value\n"))
	var writeBuff bytes.Buffer
	nb, err := cr.write(&writeBuff)
	require.NoError(t, err)
	assert.Equal(t, 15, nb)
	assert.Equal(t, []byte("Contents Value\n"), writeBuff.Bytes())
}
