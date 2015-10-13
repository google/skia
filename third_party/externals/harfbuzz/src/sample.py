#!/usr/bin/python

import sys
from gi.repository import HarfBuzz as hb

def nothing():
	pass

fontdata = file (sys.argv[1]).read ()
blob = hb.blob_create (fontdata, hb.memory_mode_t.WRITABLE, None, nothing)
print blob
buffer = hb.buffer_create ()

