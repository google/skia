#ifndef GrUserConfig_DEFINED
#define GrUserConfig_DEFINED

#if defined(GR_USER_CONFIG_FILE)
    #error "default user config pulled in but GR_USER_CONFIG_FILE is defined."
#endif

#if 0
    #undef GR_RELEASE
    #undef GR_DEBUG
    #define GR_RELEASE  0
    #define GR_DEBUG    1
#endif

/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

//#define GR_FORCE_GLCHECKERR   1

/*
 *  The default 32bit pixel config for texture upload is GL_RGBA. If your
 *  bitmaps map to a different GL enum, specify that with this define.
 */
//#define SK_GL_32BPP_COLOR_FORMAT  GL_RGBA

/*
 *  To diagnose texture cache performance, define this to 1 if you want to see
 *  a log statement everytime we upload an image to create a texture.
 */
//#define GR_DUMP_TEXTURE_UPLOAD    1

////////////////////////////////////////////////////////////////////////////////
// Decide Ganesh types

#define GR_SCALAR_IS_FIXED          0
#define GR_SCALAR_IS_FLOAT          1

#define GR_TEXT_SCALAR_IS_USHORT    0
#define GR_TEXT_SCALAR_IS_FIXED     0
#define GR_TEXT_SCALAR_IS_FLOAT     1

#endif


