// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/testutils"
)

func TestGenerateAmbientNamespace_ValidInput_Success(t *testing.T) {
	contents := testutils.ReadFile(t, "bindings1.cpp")
	expectedOutput := testutils.ReadFile(t, "expectedambientnamespace1.d.ts")
	output, err := generateAmbientNamespace("namespace_one", contents)
	require.NoError(t, err)
	assert.Equal(t, expectedOutput, output)
}

func TestGenerateAmbientNamespace_FieldMissingAnnotation_ReturnsError(t *testing.T) {
	_, err := generateAmbientNamespace("namespace_one", `
value_object<SomeValueObject>("SomeValueObject")
	/**
		The number of columns that the frobulator needs.
		@type number
	 */
	.field("columns",   &SomeValueObject::columns)
	/**
	 * This is missing the type annotation!!!
	 */
	.field("misbehaving_field",    &SomeValueObject::object)
	/** @type string */
	.field("name",      &SomeValueObject::slot)
	/**
	  *  @type boolean
	  */
	.field("isInteger", &SomeValueObject::isInteger);
`)
	require.Error(t, err)
	assert.Contains(t, err.Error(), `Line 11: field "misbehaving_field" must be preceded by a @type annotation.`)
}

func TestGenerateAmbientNamespace_ConstantMissingAnnotation_ReturnsError(t *testing.T) {
	_, err := generateAmbientNamespace("namespace_one", `
/**
 *  @type boolean
 */
constant("good_constant", true);
constant("bad_constant", 0x2);
`)
	require.Error(t, err)
	assert.Contains(t, err.Error(), `Line 6: constant "bad_constant" must be preceded by a @type annotation.`)
}
