# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


""" Helper functions to be used in bench_pictures.cfg. """


def Config(**kwargs):
  config = {}
  for key in kwargs:
    config[key] = kwargs[key]
  return config


def TileArgs(tile_x, tile_y, timeIndividualTiles=True):
  config = {'mode': ['tile', str(tile_x), str(tile_y)]}
  if timeIndividualTiles:
    config['timeIndividualTiles'] = True
  return config


def BitmapConfig(**kwargs):
  return Config(config='8888', **kwargs)


def GPUConfig(**kwargs):
  return Config(config='gpu', **kwargs)


def TiledBitmapConfig(tile_x, tile_y, timeIndividualTiles=True, **kwargs):
  return BitmapConfig(**dict(TileArgs(tile_x, tile_y,
      timeIndividualTiles=timeIndividualTiles).items() + kwargs.items()))


def TiledGPUConfig(tile_x, tile_y, **kwargs):
  return GPUConfig(**dict(TileArgs(tile_x, tile_y).items() + kwargs.items()))


def TiledConfig(tile_x, tile_y, timeIndividualTiles=True, **kwargs):
  return Config(**dict(TileArgs(tile_x, tile_y,
      timeIndividualTiles=timeIndividualTiles).items() + kwargs.items()))


def ViewportBitmapConfig(viewport_x, viewport_y, **kwargs):
  return BitmapConfig(viewport=[str(viewport_x), str(viewport_y)], **kwargs)


def ViewportGPUConfig(viewport_x, viewport_y, **kwargs):
  return GPUConfig(viewport=[str(viewport_x), str(viewport_y)], **kwargs)


def ViewportRTreeConfig(viewport_x, viewport_y, **kwargs):
  return RTreeConfig(mode='simple', viewport=[str(viewport_x), str(viewport_y)],
                     **kwargs)


def ViewportGridConfig(viewport_x, viewport_y, **kwargs):
  return GridConfig(viewport_x, viewport_y, mode='simple',
                    viewport=[str(viewport_x), str(viewport_y)], **kwargs)


def CopyTilesConfig(tile_x, tile_y, **kwargs):
  return BitmapConfig(mode=['copyTile', str(tile_x), str(tile_y)], **kwargs)


def RecordConfig(**kwargs):
  return BitmapConfig(mode='record', **kwargs)


def PlaybackCreationConfig(**kwargs):
  return BitmapConfig(mode='playbackCreation', **kwargs)


def MultiThreadTileConfig(threads, tile_x, tile_y, **kwargs):
  return TiledBitmapConfig(tile_x=tile_x, tile_y=tile_y,
                           timeIndividualTiles=False, multi=str(threads),
                           **kwargs)


def RTreeConfig(**kwargs):
  return BitmapConfig(bbh='rtree', **kwargs)


def GridConfig(tile_x, tile_y, mode, **kwargs):
  return BitmapConfig(mode=mode, bbh=['grid', str(tile_x), str(tile_y)],
                      **kwargs)


def RecordRTreeConfig(**kwargs):
  return RTreeConfig(mode='record', **kwargs)


def PlaybackCreationRTreeConfig(**kwargs):
  return RTreeConfig(mode='playbackCreation', **kwargs)


def TileRTreeConfig(tile_x, tile_y, **kwargs):
  return RTreeConfig(**dict(TileArgs(tile_x, tile_y).items() + kwargs.items()))


def RecordGridConfig(tile_x, tile_y, **kwargs):
  return GridConfig(tile_x=tile_x, tile_y=tile_y, mode='record', **kwargs)


def PlaybackCreationGridConfig(tile_x, tile_y, **kwargs):
  return GridConfig(tile_x, tile_y, mode='playbackCreation')
