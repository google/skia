/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PICTURE_RENDERING_FLAGS
#define PICTURE_RENDERING_FLAGS

class SkString;

namespace sk_tools {
    class PictureRenderer;
}

enum PictureTool {
    kBench_PictureTool,
    kRender_PictureTool,
};

/**
 *  Uses SkCommandLineFlags to parse the command line, and returns a PictureRenderer
 *  reflecting the flags used. Assumes that SkCommandLineFlags::Parse has
 *  been called.
 *  @param error If there is an error or warning, it will be stored in error.
 *  @param tool Which tool is being used.
 *  @return PictureRenderer A PictureRenderer with the settings specified
 *          on the command line, or NULL if the command line is invalid.
 */
sk_tools::PictureRenderer* parseRenderer(SkString& error, PictureTool tool);

#endif // PICTURE_RENDERING_FLAGS
