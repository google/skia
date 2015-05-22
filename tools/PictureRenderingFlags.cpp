/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PictureRenderingFlags.h"

#include "CopyTilesRenderer.h"
#if SK_SUPPORT_GPU
#include "GrContextOptions.h"
#endif
#include "PictureRenderer.h"
#include "picture_utils.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageDecoder.h"
#include "SkString.h"

// Alphabetized list of flags used by this file or bench_ and render_pictures.
DEFINE_string(bbh, "none", "bbhType [width height]: Set the bounding box hierarchy type to "
              "be used. Accepted values are: none, rtree, grid. "
              "Not compatible with --pipe. With value "
              "'grid', width and height must be specified. 'grid' can "
              "only be used with modes tile, record, and "
              "playbackCreation.");


#if SK_SUPPORT_GPU
static const char kGpuAPINameGL[] = "gl";
static const char kGpuAPINameGLES[] = "gles";
#define GPU_CONFIG_STRING "|gpu|msaa4|msaa16|nvprmsaa4|nvprmsaa16|gpudft"
#else
#define GPU_CONFIG_STRING ""
#endif
#if SK_ANGLE
#define ANGLE_CONFIG_STRING "|angle"
#else
#define ANGLE_CONFIG_STRING ""
#endif
#if SK_MESA
#define MESA_CONFIG_STRING "|mesa"
#else
#define MESA_CONFIG_STRING ""
#endif

// Although this config does not support all the same options as gm, the names should be kept
// consistent.
DEFINE_string(config, "8888", "["
              "8888" GPU_CONFIG_STRING ANGLE_CONFIG_STRING MESA_CONFIG_STRING
              "]: Use the corresponding config.");

DEFINE_bool(deferImageDecoding, false, "Defer decoding until drawing images. "
            "Has no effect if the provided skp does not have its images encoded.");
DEFINE_string(mode, "simple", "Run in the corresponding mode:\n"
              "simple: Simple rendering.\n"
              "tile width height: Use tiles with the given dimensions or percentages.\n"
              "pow2tile minWidth height: Use tiles with widths that are all a power\n"
              "\tof two such that they minimize the amount of wasted tile space.\n"
              "\tminWidth must be a power of two.\n"
              "copyTile width height: Draw the picture, then copy into tiles. If the\n"
              "\tpicture is large enough, it is broken into larger tiles to avoid\n"
              "\tcreating a large canvas.\n"
// TODO: If bench_pictures and render_pictures were two separate targets, we could use build flags
// to determine which modes to display.
              "record: (Only in bench_pictures) Time recording from a picture to a new\n"
              "\tpicture.\n"
              "playbackCreation: (Only in bench_pictures) Time creation of the \n"
              "\tSkPicturePlayback.\n"
              "rerecord: (Only in render_pictures) Record the picture as a new skp,\n"
              "\twith the bitmaps PNG encoded.\n");
DEFINE_bool(pipe, false, "Use SkGPipe rendering. Currently incompatible with \"mode\".");
DEFINE_string2(readPath, r, "", "skp files or directories of skp files to process.");
DEFINE_double(scale, 1, "Set the scale factor.");
DEFINE_string(tiles, "", "Used with --mode copyTile to specify number of tiles per larger tile "
              "in the x and y directions.");
DEFINE_string(viewport, "", "width height: Set the viewport.");
#if SK_SUPPORT_GPU
DEFINE_string(gpuAPI, "", "Force use of specific gpu API.  Using \"gl\" "
              "forces OpenGL API. Using \"gles\" forces OpenGL ES API. "
              "Defaults to empty string, which selects the API native to the "
              "system.");
DEFINE_bool(gpuCompressAlphaMasks, false, "Compress masks generated from falling back to "
                                          "software path rendering.");
#endif

sk_tools::PictureRenderer* parseRenderer(SkString& error, PictureTool tool) {
    error.reset();

    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    bool isPowerOf2Mode = false;
    bool isCopyMode = false;
    const char* mode = NULL;

#if SK_SUPPORT_GPU
    GrContextOptions grContextOpts;
    grContextOpts.fDrawPathToCompressedTexture = FLAGS_gpuCompressAlphaMasks;
  #define RENDERER_ARGS (grContextOpts)
#else
  #define RENDERER_ARGS ()
#endif

    SkAutoTUnref<sk_tools::PictureRenderer> renderer;
    if (FLAGS_mode.count() >= 1) {
        mode = FLAGS_mode[0];
        if (0 == strcmp(mode, "record")) {
            renderer.reset(SkNEW_ARGS(sk_tools::RecordPictureRenderer, RENDERER_ARGS));
        } else if (0 == strcmp(mode, "tile") || 0 == strcmp(mode, "pow2tile")
                   || 0 == strcmp(mode, "copyTile")) {
            useTiles = true;

            if (0 == strcmp(mode, "pow2tile")) {
                isPowerOf2Mode = true;
            } else if (0 == strcmp(mode, "copyTile")) {
                isCopyMode = true;
            }

            if (FLAGS_mode.count() < 2) {
                error.printf("Missing width for --mode %s\n", mode);
                return NULL;
            }

            widthString = FLAGS_mode[1];
            if (FLAGS_mode.count() < 3) {
                error.printf("Missing height for --mode %s\n", mode);
                return NULL;
            }

            heightString = FLAGS_mode[2];
        } else if (0 == strcmp(mode, "playbackCreation") && kBench_PictureTool == tool) {
            renderer.reset(SkNEW_ARGS(sk_tools::PlaybackCreationRenderer, RENDERER_ARGS));
        // undocumented
        } else if (0 == strcmp(mode, "rerecord") && kRender_PictureTool == tool) {
            renderer.reset(SkNEW_ARGS(sk_tools::RecordPictureRenderer, RENDERER_ARGS));
        } else if (0 == strcmp(mode, "simple")) {
            // Allow 'mode' to be set to 'simple', but do not create a renderer, so we can
            // ensure that pipe does not override a mode besides simple. The renderer will
            // be created below.
        } else {
            error.printf("%s is not a valid mode for --mode\n", mode);
            return NULL;
        }
    }

    if (useTiles) {
        SkASSERT(NULL == renderer);
        SkAutoTUnref<sk_tools::TiledPictureRenderer> tiledRenderer;
        if (isCopyMode) {
            int xTiles = -1;
            int yTiles = -1;
            if (FLAGS_tiles.count() > 0) {
                if (FLAGS_tiles.count() != 2) {
                    error.printf("--tiles requires an x value and a y value.\n");
                    return NULL;
                }
                xTiles = atoi(FLAGS_tiles[0]);
                yTiles = atoi(FLAGS_tiles[1]);
            }

            int x, y;
            if (xTiles != -1 && yTiles != -1) {
                x = xTiles;
                y = yTiles;
                if (x <= 0 || y <= 0) {
                    error.printf("--tiles must be given values > 0\n");
                    return NULL;
                }
            } else {
                x = y = 4;
            }
#if SK_SUPPORT_GPU
            tiledRenderer.reset(SkNEW_ARGS(sk_tools::CopyTilesRenderer, (grContextOpts, x, y)));
#else
            tiledRenderer.reset(SkNEW_ARGS(sk_tools::CopyTilesRenderer, (x, y)));
#endif
        } else {
            tiledRenderer.reset(SkNEW_ARGS(sk_tools::TiledPictureRenderer, RENDERER_ARGS));
        }

        if (isPowerOf2Mode) {
            int minWidth = atoi(widthString);
            if (!SkIsPow2(minWidth) || minWidth < 0) {
                SkString err;
                error.printf("-mode %s must be given a width"
                             " value that is a power of two\n", mode);
                return NULL;
            }
            tiledRenderer->setTileMinPowerOf2Width(minWidth);
        } else if (sk_tools::is_percentage(widthString)) {
            if (isCopyMode) {
                error.printf("--mode %s does not support percentages.\n", mode);
                return NULL;
            }
            tiledRenderer->setTileWidthPercentage(atof(widthString));
            if (!(tiledRenderer->getTileWidthPercentage() > 0)) {
                error.printf("--mode %s must be given a width percentage > 0\n", mode);
                return NULL;
            }
        } else {
            tiledRenderer->setTileWidth(atoi(widthString));
            if (!(tiledRenderer->getTileWidth() > 0)) {
                error.printf("--mode %s must be given a width > 0\n", mode);
                return NULL;
            }
        }

        if (sk_tools::is_percentage(heightString)) {
            if (isCopyMode) {
                error.printf("--mode %s does not support percentages.\n", mode);
                return NULL;
            }
            tiledRenderer->setTileHeightPercentage(atof(heightString));
            if (!(tiledRenderer->getTileHeightPercentage() > 0)) {
                error.printf("--mode %s must be given a height percentage > 0\n", mode);
                return NULL;
            }
        } else {
            tiledRenderer->setTileHeight(atoi(heightString));
            if (!(tiledRenderer->getTileHeight() > 0)) {
                SkString err;
                error.printf("--mode %s must be given a height > 0\n", mode);
                return NULL;
            }
        }

        renderer.reset(tiledRenderer.detach());
        if (FLAGS_pipe) {
            error.printf("Pipe rendering is currently not compatible with tiling.\n"
                         "Turning off pipe.\n");
        }

    } else { // useTiles
        if (FLAGS_pipe) {
            if (renderer != NULL) {
                error.printf("Pipe is incompatible with other modes.\n");
                return NULL;
            }
            renderer.reset(SkNEW_ARGS(sk_tools::PipePictureRenderer, RENDERER_ARGS));
        }
    }

    if (NULL == renderer) {
        renderer.reset(SkNEW_ARGS(sk_tools::SimplePictureRenderer, RENDERER_ARGS));
    }

    if (FLAGS_viewport.count() > 0) {
        if (FLAGS_viewport.count() != 2) {
            error.printf("--viewport requires a width and a height.\n");
            return NULL;
        }
        SkISize viewport;
        viewport.fWidth = atoi(FLAGS_viewport[0]);
        viewport.fHeight = atoi(FLAGS_viewport[1]);
        renderer->setViewport(viewport);
    }

    sk_tools::PictureRenderer::SkDeviceTypes deviceType =
        sk_tools::PictureRenderer::kBitmap_DeviceType;
#if SK_SUPPORT_GPU
    GrGLStandard gpuAPI = kNone_GrGLStandard;
    if (1 == FLAGS_gpuAPI.count()) {
        if (FLAGS_gpuAPI.contains(kGpuAPINameGL)) {
            gpuAPI = kGL_GrGLStandard;
        } else if (FLAGS_gpuAPI.contains(kGpuAPINameGLES)) {
            gpuAPI = kGLES_GrGLStandard;
        } else {
            error.printf("--gpuAPI invalid api value.\n");
            return NULL;
        }
    } else if (FLAGS_gpuAPI.count() > 1) {
        error.printf("--gpuAPI invalid api value.\n");
        return NULL;
    }

    int sampleCount = 0;
    bool useDFText = false;
#endif
    if (FLAGS_config.count() > 0) {
        if (0 == strcmp(FLAGS_config[0], "8888")) {
            deviceType = sk_tools::PictureRenderer::kBitmap_DeviceType;
        }
#if SK_SUPPORT_GPU
        else if (0 == strcmp(FLAGS_config[0], "gpu")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
        }
        else if (0 == strcmp(FLAGS_config[0], "msaa4")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            sampleCount = 4;
        }
        else if (0 == strcmp(FLAGS_config[0], "msaa16")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            sampleCount = 16;
        }
        else if (0 == strcmp(FLAGS_config[0], "nvprmsaa4")) {
            deviceType = sk_tools::PictureRenderer::kNVPR_DeviceType;
            sampleCount = 4;
        }
        else if (0 == strcmp(FLAGS_config[0], "nvprmsaa16")) {
            deviceType = sk_tools::PictureRenderer::kNVPR_DeviceType;
            sampleCount = 16;
        }
        else if (0 == strcmp(FLAGS_config[0], "gpudft")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            useDFText = true;
        }
#if SK_ANGLE
        else if (0 == strcmp(FLAGS_config[0], "angle")) {
            deviceType = sk_tools::PictureRenderer::kAngle_DeviceType;
        }
#endif
#if SK_MESA
        else if (0 == strcmp(FLAGS_config[0], "mesa")) {
            deviceType = sk_tools::PictureRenderer::kMesa_DeviceType;
        }
#endif
#endif
        else {
            error.printf("%s is not a valid mode for --config\n", FLAGS_config[0]);
            return NULL;
        }
#if SK_SUPPORT_GPU
        if (!renderer->setDeviceType(deviceType, gpuAPI)) {
#else
        if (!renderer->setDeviceType(deviceType)) {
#endif
            error.printf("Could not create backend for --config %s\n", FLAGS_config[0]);
            return NULL;
        }
#if SK_SUPPORT_GPU
        renderer->setSampleCount(sampleCount);
        renderer->setUseDFText(useDFText);
#endif
    }


    sk_tools::PictureRenderer::BBoxHierarchyType bbhType
            = sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
    if (FLAGS_bbh.count() > 0) {
        const char* type = FLAGS_bbh[0];
        if (0 == strcmp(type, "none")) {
            bbhType = sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
        } else if (0 == strcmp(type, "rtree")) {
            bbhType = sk_tools::PictureRenderer::kRTree_BBoxHierarchyType;
        } else {
            error.printf("%s is not a valid value for --bbhType\n", type);
            return NULL;
        }
        if (FLAGS_pipe && sk_tools::PictureRenderer::kNone_BBoxHierarchyType != bbhType) {
            error.printf("--pipe and --bbh cannot be used together\n");
            return NULL;
        }
    }
    renderer->setBBoxHierarchyType(bbhType);
    renderer->setScaleFactor(SkDoubleToScalar(FLAGS_scale));

    return renderer.detach();
}
