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

/**
 * This gives a threshold in bytes of when to lock a GrGeometryBuffer vs using
 * updateData. (Note the depending on the underlying 3D API the update functions
 * may always be implemented using a lock)
 */
//#define GR_GEOM_BUFFER_LOCK_THRESHOLD (1<<15)

/**
 * This gives a threshold in megabytes for the maximum size of the texture cache
 * in vram. The value is only a default and can be overridden at runtime.
 */
//#define GR_DEFAULT_RESOURCE_CACHE_MB_LIMIT 96

/**
 * This specifies the maximum number of textures the texture cache can hold
 * in vram. The value is only a default and can be overridden at runtime.
 */
//#define GR_DEFAULT_RESOURCE_CACHE_COUNT_LIMIT 2048

#endif
