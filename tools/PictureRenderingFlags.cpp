/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PictureRenderingFlags.h"

#include "CopyTilesRenderer.h"
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
// Although this config does not support all the same options as gm, the names should be kept
// consistent.
#if SK_ANGLE
// ANGLE assumes GPU
DEFINE_string(config, "8888", "[8888|gpu|msaa4|msaa16|angle]: Use the corresponding config.");
#elif SK_SUPPORT_GPU
DEFINE_string(config, "8888", "[8888|gpu|msaa4|msaa16]: Use the corresponding config.");
#else
DEFINE_string(config, "8888", "[8888]: Use the corresponding config.");
#endif

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
DEFINE_int32(multi, 1, "Set the number of threads for multi threaded drawing. "
             "If > 1, requires tiled rendering.");
DEFINE_bool(pipe, false, "Use SkGPipe rendering. Currently incompatible with \"mode\".");
DEFINE_string2(readPath, r, "", "skp files or directories of skp files to process.");
DEFINE_double(scale, 1, "Set the scale factor.");
DEFINE_string(tiles, "", "Used with --mode copyTile to specify number of tiles per larger tile "
              "in the x and y directions.");
DEFINE_string(viewport, "", "width height: Set the viewport.");

sk_tools::PictureRenderer* parseRenderer(SkString& error, PictureTool tool) {
    error.reset();

    if (FLAGS_multi <= 0) {
        error.printf("--multi must be > 0, was %i", FLAGS_multi);
        return NULL;
    }

    bool useTiles = false;
    const char* widthString = NULL;
    const char* heightString = NULL;
    bool isPowerOf2Mode = false;
    bool isCopyMode = false;
    const char* mode = NULL;
    bool gridSupported = false;

    SkAutoTUnref<sk_tools::PictureRenderer> renderer;
    if (FLAGS_mode.count() >= 1) {
        mode = FLAGS_mode[0];
        if (0 == strcmp(mode, "record")) {
            renderer.reset(SkNEW(sk_tools::RecordPictureRenderer));
            gridSupported = true;
        // undocumented
        } else if (0 == strcmp(mode, "clone")) {
            renderer.reset(sk_tools::CreatePictureCloneRenderer());
        } else if (0 == strcmp(mode, "tile") || 0 == strcmp(mode, "pow2tile")
                   || 0 == strcmp(mode, "copyTile")) {
            useTiles = true;

            if (0 == strcmp(mode, "pow2tile")) {
                isPowerOf2Mode = true;
            } else if (0 == strcmp(mode, "copyTile")) {
                isCopyMode = true;
            } else {
                gridSupported = true;
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
            renderer.reset(SkNEW(sk_tools::PlaybackCreationRenderer));
            gridSupported = true;
        // undocumented
        } else if (0 == strcmp(mode, "gatherPixelRefs") && kBench_PictureTool == tool) {
            renderer.reset(sk_tools::CreateGatherPixelRefsRenderer());
        } else if (0 == strcmp(mode, "rerecord") && kRender_PictureTool == tool) {
            renderer.reset(SkNEW(sk_tools::RecordPictureRenderer));
        // Allow 'mode' to be set to 'simple', but do not create a renderer, so we can
        // ensure that pipe does not override a mode besides simple. The renderer will
        // be created below.
        } else if (0 != strcmp(mode, "simple")) {
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
            tiledRenderer.reset(SkNEW_ARGS(sk_tools::CopyTilesRenderer, (x, y)));
        } else if (FLAGS_multi > 1) {
            tiledRenderer.reset(SkNEW_ARGS(sk_tools::MultiCorePictureRenderer,
                                           (FLAGS_multi)));
        } else {
            tiledRenderer.reset(SkNEW(sk_tools::TiledPictureRenderer));
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
        if (FLAGS_multi > 1) {
            error.printf("Multithreaded drawing requires tiled rendering.\n");
            return NULL;
        }
        if (FLAGS_pipe) {
            if (renderer != NULL) {
                error.printf("Pipe is incompatible with other modes.\n");
                return NULL;
            }
            renderer.reset(SkNEW(sk_tools::PipePictureRenderer));
        }
    }

    if (NULL == renderer) {
        renderer.reset(SkNEW(sk_tools::SimplePictureRenderer));
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
    int sampleCount = 0;
#endif
    if (FLAGS_config.count() > 0) {
        if (0 == strcmp(FLAGS_config[0], "8888")) {
            deviceType = sk_tools::PictureRenderer::kBitmap_DeviceType;
        }
#if SK_SUPPORT_GPU
        else if (0 == strcmp(FLAGS_config[0], "gpu")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            if (FLAGS_multi > 1) {
                error.printf("GPU not compatible with multithreaded tiling.\n");
                return NULL;
            }
        }
        else if (0 == strcmp(FLAGS_config[0], "msaa4")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            if (FLAGS_multi > 1) {
                error.printf("GPU not compatible with multithreaded tiling.\n");
                return NULL;
            }
            sampleCount = 4;
        }
        else if (0 == strcmp(FLAGS_config[0], "msaa16")) {
            deviceType = sk_tools::PictureRenderer::kGPU_DeviceType;
            if (FLAGS_multi > 1) {
                error.printf("GPU not compatible with multithreaded tiling.\n");
                return NULL;
            }
            sampleCount = 16;
        }
#if SK_ANGLE
        else if (0 == strcmp(FLAGS_config[0], "angle")) {
            deviceType = sk_tools::PictureRenderer::kAngle_DeviceType;
            if (FLAGS_multi > 1) {
                error.printf("Angle not compatible with multithreaded tiling.\n");
                return NULL;
            }
        }
#endif
#endif
        else {
            error.printf("%s is not a valid mode for --config\n", FLAGS_config[0]);
            return NULL;
        }
        renderer->setDeviceType(deviceType);
#if SK_SUPPORT_GPU
        renderer->setSampleCount(sampleCount);
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
        } else if (0 == strcmp(type, "grid")) {
            if (!gridSupported) {
                error.printf("'--bbh grid' is not compatible with --mode=%s.\n", mode);
                return NULL;
            }
            bbhType = sk_tools::PictureRenderer::kTileGrid_BBoxHierarchyType;
            if (FLAGS_bbh.count() != 3) {
                error.printf("--bbh grid requires a width and a height.\n");
                return NULL;
            }
            int gridWidth = atoi(FLAGS_bbh[1]);
            int gridHeight = atoi(FLAGS_bbh[2]);
            renderer->setGridSize(gridWidth, gridHeight);

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
