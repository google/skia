/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkError_DEFINED
#define SkError_DEFINED


/** \file SkError.h
*/

enum SkError {
    /** All is well
     */
    kNoError_SkError=0,

    /** User argument passed to Skia function was invalid: NULL when that’s
     *  not allowed, out of numeric range, bad enum, or violating some
     *  other general precondition.
     */
    kInvalidArgument_SkError,

    /** User tried to perform some operation in a state when the operation
     *  was not legal, or the operands make no sense (e.g., asking for
     *  pixels from an SkPictureCanvas).  Other examples might be
     *  inset()’ing a rectangle to make it degenerate (negative width/height).
     */
    kInvalidOperation_SkError,

    /** Probably not needed right now, but in the future we could have opaque
     *  handles for SkPictures floating around, and it would be a good idea
     *  to anticipate this kind of issue.
     */
    kInvalidHandle_SkError,

    /** This is probably not possible because paint surely has defaults for
     *  everything, but perhaps a paint can get into a bad state somehow.
     */
    kInvalidPaint_SkError,

    /** Skia was unable to allocate memory to perform some task.
     */
    kOutOfMemory_SkError,

    /** Skia failed while trying to consume some external resource.
     */
    kParseError_SkError,

    /** Something went wrong internally; could be resource exhaustion but
      * will often be a bug.
     */
    kInternalError_SkError
};

/** Return the current per-thread error code.  Error codes are "sticky"; they
 *  are not not reset by subsequent successful operations.
 */
SkError SkGetLastError();

/** Clear the current per-thread error code back to kNoError_SkError.
 */
void SkClearLastError();

/** Type for callback functions to be invoked whenever an error is registered.
 *  Callback functions take the error code being set, as well as a context
 *  argument that is provided when the callback is registered.
 */
typedef void (*SkErrorCallbackFunction)(SkError, void *);

/** Set the current per-thread error callback.
 *
 *  @param cb The callback function to be invoked.  Passing NULL
 *            for cb will revert to the default error callback which
 *            does nothing on release builds, but on debug builds will
 *            print an informative error message to the screen.
 *  @param context An arbitrary pointer that will be passed to
 *                 the provided callback function.
 */
void SkSetErrorCallback(SkErrorCallbackFunction cb, void *context);

/** Get a human-readable description of the last (per-thread) error that
 *  occurred.  The returned error message will include not only a human
 *  readable version of the error code, but also information about the
 *  conditions that led to the error itself.
 */
const char *SkGetLastErrorString();

#endif /* SkError_DEFINED */
