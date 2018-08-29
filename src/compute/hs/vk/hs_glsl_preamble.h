//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_GLSL_PREAMBLE_ONCE
#define HS_GLSL_PREAMBLE_ONCE

//
//
//

#define HS_HASH                   #
#define HS_EVAL(a)                a
#define HS_GLSL_EXT()             HS_EVAL(HS_HASH)##extension
#define HS_GLSL_EXT_REQUIRE(name) HS_GLSL_EXT() name : require
#define HS_GLSL_VERSION(ver)      HS_EVAL(HS_HASH)##version ver

//
//
//

HS_GLSL_VERSION(450)
HS_GLSL_EXT_REQUIRE(GL_KHR_shader_subgroup_basic)
HS_GLSL_EXT_REQUIRE(GL_KHR_shader_subgroup_shuffle)

#if HS_KEY_WORDS == 2
HS_GLSL_EXT_REQUIRE(GL_ARB_gpu_shader_int64)
#endif

//
//
//

#endif

//
//
//
