The various GPU wait calls on GrDirectContext, SkSurface, and GrVkSecondaryCBContext which take
a client supplied semaphore, now only guarantee to block the gpu fragment stages instead of all
gpu commands. This shouldn't affect any client since client provided gpu resources (e.g. textures)
are only ever used by Skia in the fragment stages.
