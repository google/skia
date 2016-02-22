// Copyright 2015 The Shaderc Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SHADERC_H_
#define SHADERC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  // Forced shader kinds. These shader kinds force the compiler to compile the
  // source code as the specified kind of shader.
  shaderc_glsl_vertex_shader,
  shaderc_glsl_fragment_shader,
  shaderc_glsl_compute_shader,
  shaderc_glsl_geometry_shader,
  shaderc_glsl_tess_control_shader,
  shaderc_glsl_tess_evaluation_shader,
  // Deduce the shader kind from #pragma annotation in the source code. Compiler
  // will emit error if #pragma annotation is not found.
  shaderc_glsl_infer_from_source,
  // Default shader kinds. Compiler will fall back to compile the source code as
  // the specified kind of shader when #pragma annotation is not found in the
  // source code.
  shaderc_glsl_default_vertex_shader,
  shaderc_glsl_default_fragment_shader,
  shaderc_glsl_default_compute_shader,
  shaderc_glsl_default_geometry_shader,
  shaderc_glsl_default_tess_control_shader,
  shaderc_glsl_default_tess_evaluation_shader,
} shaderc_shader_kind;

typedef enum {
  shaderc_target_env_vulkan,         // create SPIR-V under Vulkan semantics
  shaderc_target_env_opengl,         // create SPIR-V under OpenGL semantics
  shaderc_target_env_opengl_compat,  // create SPIR-V under OpenGL semantics,
                                     // including compatibility profile
                                     // functions
  shaderc_target_env_default = shaderc_target_env_vulkan
} shaderc_target_env;

typedef enum {
  shaderc_profile_none,  // Used if and only if GLSL version did not specify
                         // profiles.
  shaderc_profile_core,
  shaderc_profile_compatibility,
  shaderc_profile_es,
} shaderc_profile;

// Used in the result module (shaderc_spv_module) to indicate the status of an
// compilation.
typedef enum {
  shaderc_compilation_status_success = 0,
  shaderc_compilation_status_invalid_stage,  // error stage deduction
  shaderc_compilation_status_compilation_error,
  shaderc_compilation_status_internal_error, // unexpected failure
  shaderc_compilation_status_null_result_module,
} shaderc_compilation_status;

// Usage examples:
//
// Aggressively release compiler resources, but spend time in initialization
// for each new use.
//      shaderc_compiler_t compiler = shaderc_compiler_initialize();
//      shader_spv_module_t module = shaderc_compile_into_spv(compiler,
//                    "int main() {}", 13, shaderc_glsl_vertex_shader, "main");
//      // Do stuff with module compilation results.
//      shaderc_module_release(module);
//      shaderc_compiler_release(compiler);
//
// Keep the compiler object around for a long time, but pay for extra space
// occupied.
//      shaderc_compiler_t compiler = shaderc_compiler_initialize();
//      // On the same, other or multiple simultaneous threads.
//      shader_spv_module_t module = shaderc_compile_into_spv(compiler,
//                    "int main() {}", 13, shaderc_glsl_vertex_shader, "main");
//      // Do stuff with module compilation results.
//      shaderc_module_release(module);
//      // Once no more compilations are to happen.
//      shaderc_compiler_release(compiler);

// An opaque handle to an object that manages all compiler state.
typedef struct shaderc_compiler* shaderc_compiler_t;

// Returns a shaderc_compiler_t that can be used to compile modules.
// A return of NULL indicates that there was an error initializing the compiler.
// Any function operating on shaderc_compiler_t must offer the basic
// thread-safety guarantee.
// [http://herbsutter.com/2014/01/13/gotw-95-solution-thread-safety-and-synchronization/]
// That is: concurrent invocation of these functions on DIFFERENT objects needs
// no synchronization; concurrent invocation of these functions on the SAME
// object requires synchronization IF AND ONLY IF some of them take a non-const
// argument.
shaderc_compiler_t shaderc_compiler_initialize(void);

// Releases the resources held by the shaderc_compiler_t.
// After this call it is invalid to make any future calls to functions
// involving this shaderc_compiler_t.
void shaderc_compiler_release(shaderc_compiler_t);

// An opaque handle to an object that manages options to a single compilation
// result.
typedef struct shaderc_compile_options* shaderc_compile_options_t;

// Returns a default-initialized shaderc_compile_options_t that can be used
// to modify the functionality of a compiled module.
// A return of NULL indicates that there was an error initializing the options.
// Any function operating on shaderc_compile_options_t must offer the
// basic thread-safety guarantee.
shaderc_compile_options_t shaderc_compile_options_initialize(void);

// Returns a copy of the given shaderc_compile_options_t.
// If NULL is passed as the parameter the call is the same as
// shaderc_compile_options_init.
shaderc_compile_options_t shaderc_compile_options_clone(
    const shaderc_compile_options_t options);

// Releases the compilation options. It is invalid to use the given
// shaderc_compile_options_t object in any future calls. It is safe to pass
// NULL to this function, and doing such will have no effect.
void shaderc_compile_options_release(shaderc_compile_options_t options);

// Adds a predefined macro to the compilation options. This has the
// same effect as passing -Dname=value to the command-line compiler.
// If value is NULL, it has the effect same as passing -Dname to the
// command-line compiler. If a macro definition with the same name has
// previously been added, the value is replaced with the new value.
// The null-terminated strings that the name and value parameters point to
// must remain valid for the duration of the call, but can be modified or
// deleted after this function has returned.
void shaderc_compile_options_add_macro_definition(
    shaderc_compile_options_t options, const char* name, const char* value);

// Sets the compiler mode to generate debug information in the output.
void shaderc_compile_options_set_generate_debug_info(
    shaderc_compile_options_t options);

// Sets the compiler mode to emit a disassembly text instead of a binary. In
// this mode, the byte array result in the shaderc_spv_module returned
// from shaderc_compile_into_spv() will consist of SPIR-V assembly text.
// Note the preprocessing only mode overrides this option, and this option
// overrides the default mode generating a SPIR-V binary.
void shaderc_compile_options_set_disassembly_mode(
    shaderc_compile_options_t options);

// Forces the GLSL language version and profile to a given pair. The version
// number is the same as would appear in the #version annotation in the source.
// Version and profile specified here overrides the #version annotation in the
// source. Use profile: 'shaderc_profile_none' for GLSL versions that do not
// define profiles, e.g. versions below 150.
void shaderc_compile_options_set_forced_version_profile(
    shaderc_compile_options_t options, int version, shaderc_profile profile);

// To support file inclusion, libshaderc invokes a callback into its client to
// resolve the full path and content of the included file.
// The client callback should follow the specified function signature below, and
// it should be passed to libshaderc through the corresponding setter function.
// When the including of a file is done, libshaderc will call another client
// callback to clean up the resources used for the including process. The client
// should implement the clean up method and pass it to libshaderc together with
// the response method.

// The struct that contains the information to be returned to the libshaderc.
// The client-side implemented response method should return a pointer of this
// struct. The underlying data is owned by client code.
struct shaderc_includer_response {
  const char* path;
  size_t path_length;
  const char* content;
  size_t content_length;
};

// The function signature of the client-side implemented response method. It
// returns a pointer to shaderc_includer_response struct.
typedef shaderc_includer_response* (*shaderc_includer_response_get_fn)(
    void* user_data, const char* filename);

// The function signature of the client-side implemented clean-up method.
// Includer will call this callback function when the including process is done
// with the fullpath and content data.
typedef void (*shaderc_includer_response_release_fn)(
    void* user_data, shaderc_includer_response* data);

// Sets the callback functions for the includer. When the includer queries for
// the full path and content of a file, client's method will be called to
// response. And when the query is done, client will be notified to clean up.
// TODO: File inclusion needs to be context-aware.
// e.g.
//  In file: /path/to/main_shader.vert:
//  #include "include/a"
//  In file: /path/to/include/a":
//  #include "b"
//  When compiling /path/to/main_shader.vert, the compiler should be able to
//  go to /path/to/include/b to find the file b.
//  This needs context info from compiler to client includer, and may needs
//  interface changes.
void shaderc_compile_options_set_includer_callbacks(
    shaderc_compile_options_t options, shaderc_includer_response_get_fn getter,
    shaderc_includer_response_release_fn releasor, void* user_data);

// Sets the compiler mode to do only preprocessing. The byte array result in the
// module returned by the compilation is the text of the preprocessed shader.
// This option overrides all other compilation modes, such as disassembly mode
// and the default mode of compilation to SPIR-V binary.
void shaderc_compile_options_set_preprocessing_only_mode(
    shaderc_compile_options_t options);

// Sets the compiler mode to suppress warnings, overriding warnings-as-errors
// mode. When both suppress-warnings and warnings-as-errors modes are
// turned on, warning messages will be inhibited, and will not be emitted
// as error messages.
void shaderc_compile_options_set_suppress_warnings(
    shaderc_compile_options_t options);

// Sets the target shader environment, affecting which warnings or errors will
// be issued.  The version will be for distinguishing between different versions
// of the target environment.  "0" is the only supported version at this point
void shaderc_compile_options_set_target_env(shaderc_compile_options_t options,
                                            shaderc_target_env target,
                                            uint32_t version);

// Sets the compiler mode to treat all warnings as errors. Note the
// suppress-warnings mode overrides this option, i.e. if both
// warning-as-errors and suppress-warnings modes are set, warnings will not
// be emitted as error messages.
void shaderc_compile_options_set_warnings_as_errors(
    shaderc_compile_options_t options);

// An opaque handle to the results of a call to shaderc_compile_into_spv().
typedef struct shaderc_spv_module* shaderc_spv_module_t;

// Takes a GLSL source string and the associated shader kind, input file
// name, compiles it according to the given additional_options. If the shader
// kind is not set to a specified kind, but shaderc_glslc_infer_from_source,
// the compiler will try to deduce the shader kind from the source
// string and a failure in deducing will generate an error. Currently only
// #pragma annotation is supported. If the shader kind is set to one of the
// default shader kinds, the compiler will fall back to the default shader
// kind in case it failed to deduce the shader kind from source string.
// The input_file_name is a null-termintated string. It is used as a tag to
// identify the source string in cases like emitting error messages. It
// doesn't have to be a 'file name'.
// By default the source string will be compiled into SPIR-V binary
// and a shaderc_spv_module will be returned to hold the results of the
// compilation. When disassembly mode or preprocessing only mode is enabled
// in the additional_options, the source string will be compiled into char
// strings and held by the returned shaderc_spv_module.  The entry_point_name
// null-terminated string defines the name of the entry point to associate
// with this GLSL source. If the additional_options parameter is not NULL,
// then the compilation is modified by any options present. May be safely
// called from multiple threads without explicit synchronization. If there
// was failure in allocating the compiler object NULL will be returned.
shaderc_spv_module_t shaderc_compile_into_spv(
    const shaderc_compiler_t compiler, const char* source_text,
    size_t source_text_size, shaderc_shader_kind shader_kind,
    const char* input_file_name, const char* entry_point_name,
    const shaderc_compile_options_t additional_options);

// The following functions, operating on shaderc_spv_module_t objects, offer
// only the basic thread-safety guarantee.

// Releases the resources held by module.  It is invalid to use module for any
// further operations.
void shaderc_module_release(shaderc_spv_module_t module);

// Returns true if the result in module was a successful compilation.
bool shaderc_module_get_success(const shaderc_spv_module_t module);

// Returns the number of bytes in a SPIR-V module result string. When the module
// is compiled with disassembly mode or preprocessing only mode, the result
// string is a char string. Otherwise, the result string is binary string.
size_t shaderc_module_get_length(const shaderc_spv_module_t module);

// Returns the number of warnings generated during the compilation.
size_t shaderc_module_get_num_warnings(const shaderc_spv_module_t module);

// Returns the number of errors generated during the compilation.
size_t shaderc_module_get_num_errors(const shaderc_spv_module_t module);

// Returns the compilation status, indicating whether the compilation succeeded,
// or failed due to some reasons, like invalid shader stage or compilation
// errors.
shaderc_compilation_status shaderc_module_get_compilation_status(
    const shaderc_spv_module_t);

// Returns a pointer to the start of the SPIR-V bytes, either SPIR-V binary or
// char string. When the source string is compiled into SPIR-V binary, this is
// guaranteed to be castable to a uint32_t*. If the source string is compiled in
// disassembly mode or preprocessing only mode, the pointer will point to the
// result char string.
const char* shaderc_module_get_bytes(const shaderc_spv_module_t module);

// Returns a null-terminated string that contains any error messages generated
// during the compilation.
const char* shaderc_module_get_error_message(const shaderc_spv_module_t module);

// Provides the version & revision of the SPIR-V which will be produced
void shaderc_get_spv_version(unsigned int* version, unsigned int* revision);

// Parses the version and profile from a given null-terminated string
// containing both version and profile, like: '450core'. Returns false if
// the string can not be parsed. Returns true when the parsing succeeds. The
// parsed version and profile are returned through arguments.
bool shaderc_parse_version_profile(const char* str, int* version,
                                   shaderc_profile* profile);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SHADERC_H_
