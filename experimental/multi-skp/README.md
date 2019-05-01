Multi SKP
=========

Multi skp is an experimental format for representing animations of display lists and resources.
Basically multiple frames, each serialized as one SKP file, written in a single file with some
metadata at the beginning of each frame.

## Format (draft)
A file header with
   an 8-byte magic word "multiskp"
   how many frames are in the file.
Many frames, each with
  A frame header with
    An indication of the age of the hardware buffer that skia was asked to draw into for this frame
    A number of bytes after the frame header to read containing the hardware buffer skia was asked
    to draw into for this frame. (currently unused and always zero)
  hardware buffer bytes if indicated
  A serialized SkPicture (in the exact same format as a traditional SKP file)


## Writer class

`SkMultiSkpWriter` is meant to be instantiated at the beginning of an animation capture (in Android)
and provided with a filename to write to.

Call `startFrame` at the beginning of each frame, and write to the returned canvas.
Call `finishFrame` when no more commands will be written to the canvas.

A background thread listens for data from finished frames and writes them out to the file.
this seems to take about 0.5 seconds per frame.