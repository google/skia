# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Visualize bitmaps in gdb.

(gdb) source <path to this file>
(gdb) sk_bitmap <symbol>

This should pop up a window with the bitmap displayed.
Right clicking should bring up a menu, allowing the
bitmap to be saved to a file.
"""

import gdb
from enum import Enum
try:
  from PIL import Image
except ImportError:
  import Image

class ColorType(Enum):
    unknown = 0
    alpha_8 = 1
    rgb_565 = 2
    argb_4444 = 3
    rgba_8888 = 4
    bgra_8888 = 5
    gray_8 = 6
    rgba_F16 = 7

class AlphaType(Enum):
    unknown = 0
    opaque = 1
    premul = 2
    unpremul = 3

class sk_bitmap(gdb.Command):
  """Displays the content of an SkBitmap image."""

  def __init__(self):
      super(sk_bitmap, self).__init__('sk_bitmap',
                                      gdb.COMMAND_SUPPORT,
                                      gdb.COMPLETE_FILENAME)

  def invoke(self, arg, from_tty):
    frame = gdb.selected_frame()
    val = frame.read_var(arg)
    if str(val.type.strip_typedefs()) == 'SkBitmap':
      pixels = val['fPixels']
      row_bytes = val['fRowBytes']
      info = val['fInfo']
      width = info['fWidth']
      height = info['fHeight']
      color_type = info['fColorType']
      alpha_type = info['fAlphaType']

      process = gdb.selected_inferior()
      memory_data = process.read_memory(pixels, row_bytes * height)
      size = (width, height)
      image = None
      # See Unpack.c for the values understood after the "raw" parameter.
      if color_type == ColorType.bgra_8888.value:
        if alpha_type == AlphaType.unpremul.value:
          image = Image.frombytes("RGBA", size, memory_data.tobytes(),
                                  "raw", "BGRA", row_bytes, 1)
        elif alpha_type == AlphaType.premul.value:
          # RGBA instead of RGBa, because Image.show() doesn't work with RGBa.
          image = Image.frombytes("RGBA", size, memory_data.tobytes(),
                                  "raw", "BGRa", row_bytes, 1)

      if image:
        # Fails on premultiplied alpha, it cannot convert to RGB.
        image.show()
      else:
        print ("Need to add support for %s %s." % (
               str(ColorType(int(color_type))),
               str(AlphaType(int(alpha_type)))
        ))

sk_bitmap()
