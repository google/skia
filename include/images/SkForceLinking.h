/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 *  This function's sole purpose is to trick the linker into not discarding
 *  SkImageDecoder subclasses just because we do not directly call them.
 *  This is necessary in applications that will create image decoders from
 *  a stream.
 *  Call this function with an expression that evaluates to false to ensure
 *  that the linker includes the subclasses.
 *  Passing true will result in leaked objects.
 */
int SkForceLinking(bool doNotPassTrue);

#define __SK_FORCE_IMAGE_DECODER_LINKING       \
SK_UNUSED static int linking_forced = SkForceLinking(false)
