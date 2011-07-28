
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBML_XMLParser_DEFINED
#define SkBML_XMLParser_DEFINED

class SkStream;
class SkWStream;
class SkXMLParser;
class SkXMLWriter;

class BML_XMLParser {
public:
    /** Read the byte XML stream and write the decompressed XML.
    */
    static void Read(SkStream& s, SkXMLWriter& writer);
    /** Read the byte XML stream and write the decompressed XML into a writable stream.
    */
    static void Read(SkStream& s, SkWStream& output);
    /** Read the byte XML stream and write the decompressed XML into an XML parser.
    */
    static void Read(SkStream& s, SkXMLParser& output);
};

#endif // SkBML_XMLParser_DEFINED

