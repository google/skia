#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#

INCLUDES	= svg_elems.h svg_attribs.h svg_transforms.h svg_color_names.h

all:		$(INCLUDES)

clean:
		-rm -f $(INCLUDES)

$(INCLUDES):	%.h: %.gperf
		gperf --output-file=$@ $<
