#ANGLE
The goal of ANGLE is to allow Windows users to seamlessly run WebGL and other OpenGL ES content by translating OpenGL ES API calls to DirectX 9 or DirectX 11 API calls.

ANGLE is a conformant implementation of the OpenGL ES 2.0 specification that is hardware‐accelerated via Direct3D. ANGLE v1.0.772 was certified compliant by passing the ES 2.0.3 conformance tests in October 2011. ANGLE also provides an implementation of the EGL 1.4 specification. Work on ANGLE's OpenGL ES 3.0 implementation is currently in progress, but should not be considered stable.

ANGLE is used as the default WebGL backend for both Google Chrome and Mozilla Firefox on Windows platforms. Chrome uses ANGLE for all graphics rendering on Windows, including the accelerated Canvas2D implementation and the Native Client sandbox environment.

Portions of the ANGLE shader compiler are used as a shader validator and translator by WebGL implementations across multiple platforms. It is used on Mac OS X, Linux, and in mobile variants of the browsers. Having one shader validator helps to ensure that a consistent set of GLSL ES shaders are accepted across browsers and platforms. The shader translator can be used to translate shaders to other shading languages, and to optionally apply shader modifications to work around bugs or quirks in the native graphics drivers. The translator targets Desktop GLSL, Direct3D HLSL, and even ESSL for native GLES2 platforms.

##Building
View the [Dev setup instructions](doc/DevSetup.md).

##Contributing
* Join our [Google group](https://groups.google.com/group/angleproject) to keep up to date.
* Join us on IRC in the #ANGLEproject channel on FreeNode.
* Read about ANGLE development in our [documentation](doc).
* Become a [code contributor](doc/ContributingCode.md).
* File bugs in the [issue tracker](http://code.google.com/p/angleproject/issues/list) (preferably with an isolated test-case).
* Read about WebGL on the [Khronos WebGL Wiki](http://khronos.org/webgl/wiki/Main_Page).
* Learn about implementation details in the [OpenGL Insights chapter on ANGLE](http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-ANGLE.pdf) and this [ANGLE presentation](https://code.google.com/p/angleproject/downloads/detail?name=ANGLE%20and%20Cross-Platform%20WebGL%20Support.pdf&can=2&q=).
* Learn about the past, present, and future of the ANGLE implementation in [this recent presentation](https://docs.google.com/presentation/d/1CucIsdGVDmdTWRUbg68IxLE5jXwCb2y1E9YVhQo0thg/pub?start=false&loop=false).
* If you use ANGLE in your own project, we'd love to hear about it!