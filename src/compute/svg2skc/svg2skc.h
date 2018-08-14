/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#pragma once

//
//
//

#include <skc/skc.h>
#include <svg/svg_doc.h>

//
//
//

skc_path_t *
svg_doc_paths_decode(struct svg_doc * const sd, skc_path_builder_t pb);

//
//
//

skc_raster_t *
svg_doc_rasters_decode(struct svg_doc             * const sd, 
                       struct skc_transform_stack * const ts,
                       skc_path_t const           * const paths, 
                       skc_raster_builder_t               rb);

//
//
//

void
svg_doc_paths_release(struct svg_doc * const sd, 
                      skc_path_t     * const paths,
                      skc_context_t          context);

void
svg_doc_rasters_release(struct svg_doc * const sd, 
                        skc_raster_t   * const rasters,
                        skc_context_t          context);

//
// kickstarts pipeline -- useful for debugging
//

void
svg_doc_paths_flush(struct svg_doc * const sd, 
                    skc_path_t     * const paths,
                    skc_context_t          context);

void
svg_doc_rasters_flush(struct svg_doc * const sd, 
                      skc_raster_t   * const rasters,
                      skc_context_t          context);

//
//
//

void
svg_doc_layers_decode(struct svg_doc     * const sd,
                      skc_raster_t const * const rasters, 
                      skc_composition_t          composition, 
                      skc_styling_t              styling,
                      bool const                 is_srgb);

//
//
//
