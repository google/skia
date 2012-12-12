# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


""" Helper functions to be used in bench_pictures.cfg. """


def Config(**kwargs):
  config = {}
  for key in kwargs:
    config[key] = kwargs[key]
  return config


def BitmapConfig(**kwargs):
  return Config(device='bitmap', **kwargs)


def GPUConfig(**kwargs):
  return Config(device='gpu', **kwargs)


def TiledBitmapConfig(tile_x, tile_y, **kwargs):
  return BitmapConfig(mode=['tile', str(tile_x), str(tile_y)], **kwargs)


def TiledGPUConfig(tile_x, tile_y, **kwargs):
  return GPUConfig(mode=['tile', str(tile_x), str(tile_y)], **kwargs)


def CopyTilesConfig(tile_x, tile_y, **kwargs):
  return BitmapConfig(mode=['copyTile', str(tile_x), str(tile_y)], **kwargs)


def RecordConfig(**kwargs):
  return BitmapConfig(mode='record', **kwargs)


def PlaybackCreationConfig(**kwargs):
  return BitmapConfig(mode='playbackCreation', **kwargs)


def MultiThreadTileConfig(threads, tile_x, tile_y, **kwargs):
  return TiledBitmapConfig(multi=threads, tile_x=tile_x, tile_y=tile_y,
                           **kwargs)