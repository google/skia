
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


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
 *  The default 32bit pixel config for texture upload is GL_RGBA on all
 *  platforms except on Windows where it is GL_BGRA. If your bitmaps map to a
 *  different GL enum, specify that with this define. For portability use
 *  GR_BGRA rather than GL_BGRA for platforms where this format is an
 *  extension.
 */
//#define GR_GL_32BPP_COLOR_FORMAT  GL_RGBA

/*
 *  To diagnose texture cache performance, define this to 1 if you want to see
 *  a log statement everytime we upload an image to create a texture.
 */
//#define GR_DUMP_TEXTURE_UPLOAD    1

/*
 * To log all GL calls define this. Can be turned on and off at runtime by
 * gPrintGL global variable.
 */
//#define GR_GL_LOG_CALLS 1

/*
 * When drawing rects this causes Ganesh to use a vertex buffer containing
 * a unit square that is positioned by a matrix. Enable on systems where
 * emitting per-rect-draw verts is more expensive than constant/matrix
 * updates. Defaults to 0.
 */
//#define GR_STATIC_RECT_VB 1

/*
 * This causes more aggressive shader optimization. May hurt performance if
 * switching shaders is expensive.
 */
//#define GR_AGGRESSIVE_SHADER_OPTS 1

/*
 * This gives a threshold in bytes of when to lock a GrGeometryBuffer vs using
 * updateData. (Note the depending on the underlying 3D API the update functions
 * may always be implemented using a lock)
 */
//#define GR_GEOM_BUFFER_LOCK_THRESHOLD (1<<15)

///////////////////////////////////////////////////////////////////////////////
/*
 *  temporary flags (may go away soon)
 */

///////////////////////////////////////////////////////////////////////////////
// Decide Ganesh types

#define GR_SCALAR_IS_FIXED          0
#define GR_SCALAR_IS_FLOAT          1

#define GR_TEXT_SCALAR_IS_USHORT    0
#define GR_TEXT_SCALAR_IS_FIXED     0
#define GR_TEXT_SCALAR_IS_FLOAT     1

#endif


