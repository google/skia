`GrGLInterface` now expects functions that take two `GLuints` instead of one `GLuint64` for `glWaitSync` and `glClientWaitSync`
when building with Emscripten. `GrGLMakeAssembledWebGLInterface` binds directly to the `emscipten_gl*` functions declared in the `<webgl/*>` headers rather than the functions declared
in `GLES3/gl32.h` and `GLES3/gl2ext.h`.