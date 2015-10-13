# Debugging Tips

There are many ways to debug ANGLE using generic or platform-dependent tools. Here is a list of tips on how to use them.

## Running ANGLE under apitrace on Linux

[Apitrace](http://apitrace.github.io/) that captures traces of OpenGL commands for later analysis, allowing us to see how ANGLE translates OpenGL ES commands. In order to capture the trace, it inserts a driver shim using `LD_PRELOAD` that records the command and then forwards it to the OpenGL driver.

The problem with ANGLE is that it exposes the same symbols as the OpenGL driver so apitrace captures the entry point calls intended for ANGLE and reroutes them to the OpenGL driver. In order to avoid this problem, use the following:
 1. Compile ANGLE as a static library so that it doesn't get shadowed by apitrace's shim using the `-D angle_gl_library_type=static_library` gyp flag.
 2. Ask apitrace to explicitly load the driver instead of using a dlsym on the current module. Otherwise apitrace will use ANGLE's symbols as the OpenGL driver entrypoint (causing an infinite recursion). To do this `export TRACE_LIBGL=/usr/lib/libGL.so.1`.
 3. Link ANGLE against libGL instead of dlsyming the symbols at runtime; otherwise ANGLE won't use the replaced driver entry points. This can be done by adding `-D angle_link_glx=1`.

If you follow these steps, apitrace will work correctly aside from a few minor bugs like not being able to figure out what the default framebuffer size is if there is no glViewport command.

For example, to trace a run of `hello_triangle`, assuming you are using the ninja gyp generator and the apitrace executables are in `$PATH`:

```
./build/gyp_angle -D angle_link_glx=1 -D angle_gl_library_type=static_library
ninja -C out/Debug
export TRACE_LIBGL="/usr/lib/libGL.so.1"
apitrace trace -o mytrace ./out/Debug/hello_triangle
qapitrace mytrace
```
