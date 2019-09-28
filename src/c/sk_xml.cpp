/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/xml/SkXMLWriter.h"

#include "include/c/sk_xml.h"

#include "src/c/sk_types_priv.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_xmlstreamwriter_t* sk_xmlstreamwriter_new(sk_wstream_t* stream) {
    return ToXMLStreamWriter(new SkXMLStreamWriter(AsWStream(stream)));
}

void sk_xmlstreamwriter_delete(sk_xmlstreamwriter_t* writer) {
    delete AsXMLStreamWriter(writer);
}
