# Introduction

This page details the extensions that are supported by ANGLE.

Specifications for GLES extensions can be found in the [Khronos OpenGL ES API
Registry](http://www.khronos.org/registry/gles/)

Specifications for EGL extensions can be found in the [Khronos EGL API Registry]
(http://www.khronos.org/registry/egl/)

Specifications for ANGLE-specific extensions can be found in the [ANGLE
extension registry]
(https://code.google.com/p/angleproject/source/browse/?name=master#git%2Fextensions)

# Details

## GLES extensions

*   GL\_OES\_element\_index\_uint (fn1)
*   GL\_OES\_get\_program\_binary
*   GL\_OES\_packed\_depth\_stencil
*   GL\_OES\_rgb8\_rgba8
*   GL\_OES\_standard\_derivatives
*   GL\_OES\_texture\_half\_float (fn1)
*   GL\_OES\_texture\_half\_float\_linear (fn1)
*   GL\_OES\_texture\_float (fn1)
*   GL\_OES\_texture\_float\_linear (fn1)
*   GL\_OES\_texture\_npot (fn1)
*   GL\_EXT\_occlusion\_query\_boolean (fn1)
*   GL\_EXT\_read\_format\_bgra
*   GL\_EXT\_robustness
    *   reset notifications and sized queries only, no robust buffer access
*   GL\_EXT\_texture\_compression\_dxt1 (fn1)
*   GL\_EXT\_texture\_filter\_anisotropic (fn1)
*   GL\_EXT\_texture\_format\_BGRA8888
*   GL\_EXT\_texture\_storage
*   GL\_ANGLE\_depth\_texture
    *   requires support for INTZ and NULL surfaces in D3D9 (see
        http://aras-p.info/texts/D3D9GPUHacks.html)
*   GL\_ANGLE\_framebuffer\_blit
*   GL\_ANGLE\_framebuffer\_multisample (fn1)
*   GL\_ANGLE\_instanced\_arrays
    *   requires SM3 support
*   GL\_ANGLE\_pack\_reverse\_row\_order
*   GL\_ANGLE\_texture\_compression\_dxt3 (fn1)
*   GL\_ANGLE\_texture\_compression\_dxt5 (fn1)
*   GL\_ANGLE\_texture\_usage
*   GL\_ANGLE\_translated\_shader\_source
*   GL\_NV\_fence (fn1)

## EGL Extensions

*   EGL\_EXT\_create\_context\_robustness
    *   only reset notifications supported
*   EGL\_ANGLE\_d3d\_share\_handle\_client\_buffer (fn2)
*   EGL\_ANGLE\_query\_surface\_pointer
*   EGL\_ANGLE\_software\_display (fn3)
*   EGL\_ANGLE\_surface\_d3d\_texture\_2d\_share\_handle (fn2)
*   EGL\_NV\_post\_sub\_buffer

### Notes

*   fn1: extensions are only exposed if underlying D3D9 device has support for
    the required features
*   fn2: extensions are only exposed when running on D3D9Ex (ie Win Vista/7)
*   fn3: extension is only exposed when swiftshader is present
