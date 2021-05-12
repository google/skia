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

func TestGenerateAmbientNamespace(t *testing.T) {
	contents := testutils.ReadFile(t, "bindings1.cpp")
	expectedOutput := testutils.ReadFile(t, "expectedambientnamespace1.d.ts")
	output, err := generateAmbientNamespace("namespace_one", contents)
	require.NoError(t, err)
	assert.Equal(t, expectedOutput, output)
}
