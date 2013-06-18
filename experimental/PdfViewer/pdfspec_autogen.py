import datatypes

def buildPdfSpec(pdfspec):
  pdfspec.addClass('StreamCommonDictionary', 'Dictionary', 'Entries common to all stream dictionaries')\
      .required('NULL')\
          .field('Length')\
          .name('Length')\
          .type('integer')\
          .comment('(Required) The number of bytes from the beginning of the line fol-\nlowing the keyword stream to the last byte just before the keyword\nendstream. (There may be an additional EOL marker, preceding\nendstream, that is not included in the count and is not logically part\nof the stream data.) See "Stream Extent," above, for further discus-\nsion.')\
          .done().done()\
      .optional()\
          .field('Filter')\
          .name('Filter')\
          .type('name or array')\
          .comment('(Optional) The name of a filter to be applied in processing the stream\ndata found between the keywords stream and endstream, or an array\nof such names. Multiple filters should be specified in the order in\nwhich they are to be applied.')\
          .done().done()\
      .optional()\
          .field('DecodeParms')\
          .name('DecodeParms')\
          .type('dictionary or array')\
          .comment('(Optional) A parameter dictionary, or an array of such dictionaries,\nused by the filters specified by Filter. If there is only one filter and that\nfilter has parameters, DecodeParms must be set to the filter\'s parame-\nter dictionary unless all the filter\'s parameters have their default\nvalues, in which case the DecodeParms entry may be omitted. If there\nare multiple filters and any of the filters has parameters set to non-\ndefault values, DecodeParms must be an array with one entry for\neach filter: either the parameter dictionary for that filter, or the null\nobject if that filter has no parameters (or if all of its parameters have\ntheir default values). If none of the filters have parameters, or if all\ntheir parameters have default values, the DecodeParms entry may be\nomitted. (See implementation note 7 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Optional; PDF 1.2) The file containing the stream data. If this entry\nis present, the bytes between stream and endstream are ignored, the\nfilters are specified by FFilter rather than Filter, and the filter parame-\nters are specified by FDecodeParms rather than DecodeParms. How-\never, the Length entry should still specify the number of those bytes.\n(Usually there are no bytes and Length is 0.)')\
          .done().done()\
      .optional()\
          .field('FFilter')\
          .name('FFilter')\
          .type('name or array')\
          .comment('(Optional; PDF 1.2) The name of a filter to be applied in processing\nthe data found in the stream\'s external file, or an array of such names.\nThe same rules apply as for Filter.')\
          .done().done()\
      .optional()\
          .field('FDecodeParms')\
          .name('FDecodeParms')\
          .type('dictionary or array')\
          .comment('(Optional; PDF 1.2) A parameter dictionary, or an array of such dic-\ntionaries, used by the filters specified by FFilter. The same rules apply\nas for DecodeParms.')\
          .done().done()\
      .done()

  pdfspec.addClass('LzwdecodeAndFlatedecodeFiltersDictionary', 'Dictionary', 'Optional parameters for LZWDecode and FlateDecode filters')\
      .optional()\
          .field('Predictor')\
          .name('Predictor')\
          .type('integer')\
          .comment('()A code that selects the predictor algorithm, if any. If the value of this entry\nis 1, the filter assumes that the normal algorithm was used to encode the data,\nwithout prediction. If the value is greater than 1, the filter assumes that the\ndata was differenced before being encoded, and Predictor selects the predic-\ntor algorithm. For more information regarding Predictor values greater\nthan 1, see "LZW and Flate Predictor Functions," below. Default value: 1.')\
          .done().done()\
      .optional()\
          .field('Colors')\
          .name('Colors')\
          .type('integer')\
          .comment('(Used only if Predictor is greater than 1) The number of interleaved color com-\nponents per sample. Valid values are 1 to 4 in PDF 1.2 or earlier, and 1 or\ngreater in PDF 1.3 or later. Default value: 1.')\
          .done().done()\
      .optional()\
          .field('BitsPerComponent')\
          .name('BitsPerComponent')\
          .type('integer')\
          .comment('(Used only if Predictor is greater than 1) The number of bits used to represent\neach color component in a sample. Valid values are 1, 2, 4, and 8. Default\nvalue: 8.')\
          .done().done()\
      .optional()\
          .field('Columns')\
          .name('Columns')\
          .type('integer')\
          .comment('(Used only if Predictor is greater than 1) The number of samples in each row.\nDefault value: 1.')\
          .done().done()\
      .optional()\
          .field('EarlyChange')\
          .name('EarlyChange')\
          .type('integer')\
          .comment('(LZWDecode only) An indication of when to increase the code length. If the\nvalue of this entry is 0, code length increases are postponed as long as pos-\nsible. If it is 1, they occur one code early. This parameter is included because\nLZW sample code distributed by some vendors increases the code length one\ncode earlier than necessary. Default value: 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('CcittfaxdecodeFilterDictionary', 'Dictionary', 'Optional parameters for the CCITTFaxDecode filter')\
      .optional()\
          .field('K')\
          .name('K')\
          .type('integer')\
          .comment('()A code identifying the encoding scheme used:\n  <0    Pure two-dimensional encoding (Group 4)\n    0   Pure one-dimensional encoding (Group 3, 1-D)\n  >0    Mixed one- and two-dimensional encoding (Group 3,\n        2-D), in which a line encoded one-dimensionally can be\n        followed by at most K - 1 lines encoded two-dimensionally\nThe filter distinguishes among negative, zero, and positive values of\nK to determine how to interpret the encoded data; however, it does\nnot distinguish between different positive K values. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('EndOfLine')\
          .name('EndOfLine')\
          .type('boolean')\
          .comment('()A flag indicating whether end-of-line bit patterns are required to be\npresent in the encoding. The CCITTFaxDecode filter always accepts\nend-of-line bit patterns, but requires them only if EndOfLine is true.\nDefault value: false.')\
          .done().done()\
      .optional()\
          .field('EncodedByteAlign')\
          .name('EncodedByteAlign')\
          .type('boolean')\
          .comment('()A flag indicating whether the filter expects extra 0 bits before each\nencoded line so that the line begins on a byte boundary. If true, the\nfilter skips over encoded bits to begin decoding each line at a byte\nboundary. If false, the filter does not expect extra bits in the encod-\ned representation. Default value: false.')\
          .done().done()\
      .optional()\
          .field('Columns')\
          .name('Columns')\
          .type('integer')\
          .comment('()The width of the image in pixels. If the value is not a multiple of 8,\nthe filter adjusts the width of the unencoded image to the next mul-\ntiple of 8, so that each line starts on a byte boundary. Default value:\n1728.')\
          .done().done()\
      .optional()\
          .field('Rows')\
          .name('Rows')\
          .type('integer')\
          .comment('()The height of the image in scan lines. If the value is 0 or absent, the\nimage\'s height is not predetermined, and the encoded data must be\nterminated by an end-of-block bit pattern or by the end of the fil-\nter\'s data. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('EndOfBlock')\
          .name('EndOfBlock')\
          .type('boolean')\
          .comment('()A flag indicating whether the filter expects the encoded data to be\nterminated by an end-of-block pattern, overriding the Rows pa-\nrameter. If false, the filter stops when it has decoded the number of\nlines indicated by Rows or when its data has been exhausted, which-\never occurs first. The end-of-block pattern is the CCITT end-of-\nfacsimile-block (EOFB) or return-to-control (RTC) appropriate for\nthe K parameter. Default value: true.')\
          .done().done()\
      .optional()\
          .field('BlackIs1')\
          .name('BlackIs1')\
          .type('boolean')\
          .comment('()A flag indicating whether 1 bits are to be interpreted as black pixels\nand 0 bits as white pixels, the reverse of the normal PDF convention\nfor image data. Default value: false.')\
          .done().done()\
      .optional()\
          .field('DamagedRowsBeforeError')\
          .name('DamagedRowsBeforeError')\
          .type('integer')\
          .comment('()The number of damaged rows of data to be tolerated before an\nerror occurs. This entry applies only if EndOfLine is true and K is\nnonnegative. Tolerating a damaged row means locating its end in\nthe encoded data by searching for an EndOfLine pattern and then\nsubstituting decoded data from the previous row if the previous\nrow was not damaged, or a white scan line if the previous row was\nalso damaged. Default value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('Jbig2DecodeFilterDictionary', 'Dictionary', 'Optional parameter for the JBIG2Decode filter')\
      .optional()\
          .field('JBIG2Globals')\
          .name('JBIG2Globals')\
          .type('stream')\
          .comment('()A stream containing the JBIG2 global (page 0) segments. Global segments\nmust be placed in this stream even if only a single JBIG2 image XObject refers\nto it.')\
          .done().done()\
      .done()

  pdfspec.addClass('DctdecodeFilterDictionary', 'Dictionary', 'Optional parameter for the DCTDecode filter')\
      .optional()\
          .field('ColorTransform')\
          .name('ColorTransform')\
          .type('integer')\
          .comment('()A code specifying the transformation to be performed on the sample values:\n    0    No transformation.\n    1    If the image has three color components, transform RGB values to\n         YUV before encoding and from YUV to RGB after decoding. If the\n         image has four components, transform CMYK values to YUVK be-\n         fore encoding and from YUVK to CMYK after decoding. This option\n         is ignored if the image has one or two color components.\nNote: The RGB and YUV used here have nothing to do with the color spaces de-\nfined as part of the Adobe imaging model. The purpose of converting from RGB\nto YUV is to separate luminance and chrominance information (see below).\nThe default value of ColorTransform is 1 if the image has three components\nand 0 otherwise. In other words, conversion between RGB and YUV is per-\nformed for all three-component images unless explicitly disabled by setting\nColorTransform to 0. Additionally, the encoding algorithm inserts an Adobe-\ndefined marker code in the encoded data indicating the ColorTransform value\nused. If present, this marker code overrides the ColorTransform value given to\nDCTDecode. Thus it is necessary to specify ColorTransform only when decod-\ning data that does not contain the Adobe-defined marker code.')\
          .done().done()\
      .done()

  pdfspec.addClass('FileTrailerDictionary', 'Dictionary', 'Entries in the file trailer dictionary')\
      .required('NULL')\
          .field('Size')\
          .name('Size')\
          .type('integer')\
          .comment('(Required) The total number of entries in the file\'s cross-reference table, as defined\nby the combination of the original section and all update sections. Equivalently, this\nvalue is 1 greater than the highest object number used in the file.')\
          .done().done()\
      .optional()\
          .field('Prev')\
          .name('Prev')\
          .type('integer')\
          .comment('(Present only if the file has more than one cross-reference section) The byte offset from\nthe beginning of the file to the beginning of the previous cross-reference section.')\
          .done().done()\
      .optional()\
          .field('Root')\
          .name('Root')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The catalog dictionary for the PDF docu-\nment contained in the file (see Section 3.6.1, "Document Catalog").')\
          .done().done()\
      .optional()\
          .field('Encrypt')\
          .name('Encrypt')\
          .type('dictionary')\
          .comment('(Required if document is encrypted; PDF 1.1) The document\'s encryption dictionary\n(see Section 3.5, "Encryption").')\
          .done().done()\
      .optional()\
          .field('Info')\
          .name('Info')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) The document\'s information dictionary\n(see Section 9.2.1, "Document Information Dictionary").')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('array')\
          .comment('(Optional; PDF 1.1) An array of two strings constituting a file identifier (see Section\n9.3, "File Identifiers") for the file.')\
          .done().done()\
      .done()

  pdfspec.addClass('EncryptionCommonDictionary', 'Dictionary', 'Entries common to all encryption dictionaries')\
      .required('NULL')\
          .field('Filter')\
          .name('Filter')\
          .type('name')\
          .comment('(Required) The name of the security handler for this document; see below. Default value:\nStandard, for the built-in security handler. (Names for other security handlers can be\nregistered using the procedure described in Appendix E.)')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('number')\
          .comment('(Optional but strongly recommended) A code specifying the algorithm to be used in en-\ncrypting and decrypting the document:\n   0     An algorithm that is undocumented and no longer supported, and whose use is\n         strongly discouraged.\n   1     Algorithm 3.1 on page 73, with an encryption key length of 40 bits; see below.\n   2     (PDF 1.4) Algorithm 3.1 on page 73, but allowing encryption key lengths greater\n         than 40 bits.\n   3     (PDF 1.4) An unpublished algorithm allowing encryption key lengths ranging\n         from 40 to 128 bits. (This algorithm is unpublished as an export requirement of\n         the U.S. Department of Commerce.)\nThe default value if this entry is omitted is 0, but a value of 1 or greater is strongly rec-\nommended. (See implementation note 15 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('Length')\
          .name('Length')\
          .type('integer')\
          .comment('(Optional; PDF 1.4; only if V is 2 or 3) The length of the encryption key, in bits. The value\nmust be a multiple of 8, in the range 40 to 128. Default value: 40.')\
          .done().done()\
      .done()

  pdfspec.addClass('StandardSecurityHandlerDictionary', 'Dictionary', 'Additional encryption dictionary entries for the standard security handler')\
      .required('NULL')\
          .field('R')\
          .name('R')\
          .type('number')\
          .comment('(Required) A number specifying which revision of the standard security handler should\nbe used to interpret this dictionary. The revision number should be 2 if the document is\nencrypted with a V value less than 2 (see Table 3.13) and does not have any of the access\npermissions set (via the P entry, below) that are designated "Revision 3" in Table 3.15;\notherwise (that is, if the document is encrypted with a V value greater than 2 or has any\n"Revision 3" access permissions set), this value should be 3.')\
          .done().done()\
      .required('NULL')\
          .field('O')\
          .name('O')\
          .type('string')\
          .comment('(Required) A 32-byte string, based on both the owner and user passwords, that is used in\ncomputing the encryption key and in determining whether a valid owner password was\nentered. For more information, see "Encryption Key Algorithm" on page 78 and "Pass-\nword Algorithms" on page 79.')\
          .done().done()\
      .required('NULL')\
          .field('U')\
          .name('U')\
          .type('string')\
          .comment('(Required) A 32-byte string, based on the user password, that is used in determining\nwhether to prompt the user for a password and, if so, whether a valid user or owner pass-\nword was entered. For more information, see "Password Algorithms" on page 79.')\
          .done().done()\
      .required('NULL')\
          .field('P')\
          .name('P')\
          .type('integer')\
          .comment('(Required) A set of flags specifying which operations are permitted when the document is\nopened with user access (see Table 3.15).')\
          .done().done()\
      .done()

  pdfspec.addClass('CatalogDictionary', 'Dictionary', 'Entries in the catalog dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must\nbe Catalog for the catalog dictionary.')\
          .done().done()\
      .optional()\
          .field('Version')\
          .name('Version')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The version of the PDF specification to which the\ndocument conforms (for example, 1.4), if later than the version specified\nin the file\'s header (see Section 3.4.1, "File Header"). If the header speci-\nfies a later version, or if this entry is absent, the document conforms to\nthe version specified in the header. This entry enables a PDF producer\napplication to update the version using an incremental update; see Sec-\ntion 3.4.5, "Incremental Updates." (See implementation note 18 in Ap-\npendix H.)\nNote: The value of this entry is a name object, not a number, and so must\nbe preceded by a slash character (/) when written in the PDF file (for ex-\nample, /1.4).')\
          .done().done()\
      .optional()\
          .field('Pages')\
          .name('Pages')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The page tree node that is the\nroot of the document\'s page tree (see Section 3.6.2, "Page Tree").')\
          .done().done()\
      .optional()\
          .field('PageLabels')\
          .name('PageLabels')\
          .type('number tree')\
          .comment('(Optional; PDF 1.3) A number tree (see Section 3.8.5, "Number Trees")\ndefining the page labeling for the document. The keys in this tree are\npage indices; the corresponding values are page label dictionaries (see\nSection 8.3.1, "Page Labels"). Each page index denotes the first page in a\nlabeling range to which the specified page label dictionary applies. The\ntree must include a value for page index 0.')\
          .done().done()\
      .optional()\
          .field('Names')\
          .name('Names')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) The document\'s name dictionary (see Section 3.6.3,\n"Name Dictionary").')\
          .done().done()\
      .optional()\
          .field('Dests')\
          .name('Dests')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.1; must be an indirect reference) A dictionary of names\nand corresponding destinations (see "Named Destinations" on page\n476).')\
          .done().done()\
      .optional()\
          .field('ViewerPreferences')\
          .name('ViewerPreferences')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) A viewer preferences dictionary (see Section 8.1,\n"Viewer Preferences") specifying the way the document is to be dis-\nplayed on the screen. If this entry is absent, viewer applications should\nuse their own current user preference settings.')\
          .done().done()\
      .optional()\
          .field('PageLayout')\
          .name('PageLayout')\
          .type('name')\
          .comment('(Optional) A name object specifying the page layout to be used when the\ndocument is opened:\n    SinglePage           Display one page at a time.\n    OneColumn            Display the pages in one column.\n    TwoColumnLeft        Display the pages in two columns, with odd-\n                         numbered pages on the left.\n    TwoColumnRight       Display the pages in two columns, with odd-\n                         numbered pages on the right.\n(See implementation note 19 in Appendix H.) Default value: SinglePage.')\
          .done().done()\
      .optional()\
          .field('PageMode')\
          .name('PageMode')\
          .type('name')\
          .comment('(Optional) A name object specifying how the document should be dis-\nplayed when opened:\n    UseNone              Neither document outline nor thumbnail im-\n                         ages visible\n    UseOutlines          Document outline visible\n    UseThumbs            Thumbnail images visible\n    FullScreen           Full-screen mode, with no menu bar, window\n                         controls, or any other window visible\nDefault value: UseNone.')\
          .done().done()\
      .optional()\
          .field('Outlines')\
          .name('Outlines')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) The outline dictionary that is the\nroot of the document\'s outline hierarchy (see Section 8.2.2, "Document\nOutline").')\
          .done().done()\
      .optional()\
          .field('Threads')\
          .name('Threads')\
          .type('array')\
          .comment('(Optional; PDF 1.1; must be an indirect reference) An array of thread\ndictionaries representing the document\'s article threads (see Section\n8.3.2, "Articles").')\
          .done().done()\
      .optional()\
          .field('OpenAction')\
          .name('OpenAction')\
          .type('array or dictionary')\
          .comment('(Optional; PDF 1.1) A value specifying a destination to be displayed or\nan action to be performed when the document is opened. The value is\neither an array defining a destination (see Section 8.2.1, "Destinations")\nor an action dictionary representing an action (Section 8.5, "Actions"). If\nthis entry is absent, the document should be opened to the top of the\nfirst page at the default magnification factor.')\
          .done().done()\
      .optional()\
          .field('AA')\
          .name('AA')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) An additional-actions dictionary defining the actions\nto be taken in response to various trigger events affecting the document\nas a whole (see "Trigger Events" on page 514). (See also implementation\nnote 20 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('URI')\
          .name('URI')\
          .type('dictionary')\
          .comment('(Optional) A URI dictionary containing document-level information for\nURI (uniform resource identifier) actions (see "URI Actions" on page\n523).')\
          .done().done()\
      .optional()\
          .field('AcroForm')\
          .name('AcroForm')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) The document\'s interactive form (AcroForm) dic-\ntionary (see Section 8.6.1, "Interactive Form Dictionary").')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4; must be an indirect reference) A metadata stream\ncontaining metadata for the document (see Section 9.2.2, "Metadata\nStreams").')\
          .done().done()\
      .optional()\
          .field('StructTreeRoot')\
          .name('StructTreeRoot')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) The document\'s structure tree root dictionary (see\nSection 9.6.1, "Structure Hierarchy").')\
          .done().done()\
      .optional()\
          .field('MarkInfo')\
          .name('MarkInfo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A mark information dictionary containing informa-\ntion about the document\'s usage of Tagged PDF conventions (see Sec-\ntion 9.7.1, "Mark Information Dictionary").')\
          .done().done()\
      .optional()\
          .field('Lang')\
          .name('Lang')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) A language identifier specifying the natural language\nfor all text in the document except where overridden by language speci-\nfications for structure elements or marked content (see Section 9.8.1,\n"Natural Language Specification"). If this entry is absent, the language is\nconsidered unknown.')\
          .done().done()\
      .optional()\
          .field('SpiderInfo')\
          .name('SpiderInfo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A Web Capture information dictionary containing\nstate information used by the Acrobat Web Capture (AcroSpider) plug-\nin extension (see Section 9.9.1, "Web Capture Information Dictionary").')\
          .done().done()\
      .optional()\
          .field('OutputIntents')\
          .name('OutputIntents')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of output intent dictionaries describing the\ncolor characteristics of output devices on which the document might be\nrendered (see "Output Intents" on page 684).')\
          .done().done()\
      .done()

  pdfspec.addClass('PageTreeNodeDictionary', 'Dictionary', 'Required entries in a page tree node')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be Pages for\na page tree node.')\
          .done().done()\
      .optional()\
          .field('Parent')\
          .name('Parent')\
          .type('dictionary')\
          .comment('(Required except in root node; must be an indirect reference) The page tree node that\nis the immediate parent of this one.')\
          .done().done()\
      .required('NULL')\
          .field('Kids')\
          .name('Kids')\
          .type('array')\
          .comment('(Required) An array of indirect references to the immediate children of this node.\nThe children may be page objects or other page tree nodes.')\
          .done().done()\
      .required('NULL')\
          .field('Count')\
          .name('Count')\
          .type('integer')\
          .comment('(Required) The number of leaf nodes (page objects) that are descendants of this\nnode within the page tree.')\
          .done().done()\
      .done()

  pdfspec.addClass('PageObjectDictionary', 'Dictionary', 'Entries in a page object')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nPage for a page object.')\
          .done().done()\
      .optional()\
          .field('Parent')\
          .name('Parent')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The page tree node that is the im-\nmediate parent of this page object.')\
          .done().done()\
      .optional()\
          .field('LastModified')\
          .name('LastModified')\
          .type('date')\
          .comment('(Required if PieceInfo is present; optional otherwise; PDF 1.3) The date and\ntime (see Section 3.8.2, "Dates") when the page\'s contents were most re-\ncently modified. If a page-piece dictionary (PieceInfo) is present, the\nmodification date is used to ascertain which of the application data dic-\ntionaries that it contains correspond to the current content of the page\n(see Section 9.4, "Page-Piece Dictionaries").')\
          .done().done()\
      .required('NULL')\
          .field('Resources')\
          .name('Resources')\
          .type('dictionary')\
          .comment('(Required; inheritable) A dictionary containing any resources required by\nthe page (see Section 3.7.2, "Resource Dictionaries"). If the page requires\nno resources, the value of this entry should be an empty dictionary; omit-\nting the entry entirely indicates that the resources are to be inherited from\nan ancestor node in the page tree.')\
          .done().done()\
      .required('NULL')\
          .field('MediaBox')\
          .name('MediaBox')\
          .type('rectangle')\
          .comment('(Required; inheritable) A rectangle (see Section 3.8.3, "Rectangles"), ex-\npressed in default user space units, defining the boundaries of the physical\nmedium on which the page is intended to be displayed or printed (see\nSection 9.10.1, "Page Boundaries").')\
          .done().done()\
      .optional()\
          .field('CropBox')\
          .name('CropBox')\
          .type('rectangle')\
          .comment('(Optional; inheritable) A rectangle, expressed in default user space units,\ndefining the visible region of default user space. When the page is dis-\nplayed or printed, its contents are to be clipped (cropped) to this rectangle\nand then imposed on the output medium in some implementation-\ndefined manner (see Section 9.10.1, "Page Boundaries"). Default value:\nthe value of MediaBox.')\
          .done().done()\
      .optional()\
          .field('BleedBox')\
          .name('BleedBox')\
          .type('rectangle')\
          .comment('(Optional; PDF 1.3) A rectangle, expressed in default user space units, de-\nfining the region to which the contents of the page should be clipped\nwhen output in a production environment (see Section 9.10.1, "Page\nBoundaries"). Default value: the value of CropBox.')\
          .done().done()\
      .optional()\
          .field('TrimBox')\
          .name('TrimBox')\
          .type('rectangle')\
          .comment('(Optional; PDF 1.3) A rectangle, expressed in default user space units, de-\nfining the intended dimensions of the finished page after trimming (see\nSection 9.10.1, "Page Boundaries"). Default value: the value of CropBox.')\
          .done().done()\
      .optional()\
          .field('ArtBox')\
          .name('ArtBox')\
          .type('rectangle')\
          .comment('(Optional; PDF 1.3) A rectangle, expressed in default user space units, de-\nfining the extent of the page\'s meaningful content (including potential\nwhite space) as intended by the page\'s creator (see Section 9.10.1, "Page\nBoundaries"). Default value: the value of CropBox.')\
          .done().done()\
      .optional()\
          .field('BoxColorInfo')\
          .name('BoxColorInfo')\
          .type('dictionary')\
          .comment('(Optional) A box color information dictionary specifying the colors and\nother visual characteristics to be used in displaying guidelines on the\nscreen for the various page boundaries (see "Display of Page Boundaries"\non page 679). If this entry is absent, the viewer application should use its\nown current default settings.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('stream or array')\
          .comment('(Optional) A content stream (see Section 3.7.1, "Content Streams") de-\nscribing the contents of this page. If this entry is absent, the page is empty.\nThe value may be either a single stream or an array of streams. If it is an\narray, the effect is as if all of the streams in the array were concatenated, in\norder, to form a single stream. This allows a program generating a PDF\nfile to create image objects and other resources as they occur, even though\nthey interrupt the content stream. The division between streams may\noccur only at the boundaries between lexical tokens (see Section 3.1, "Lex-\nical Conventions"), but is unrelated to the page\'s logical content or orga-\nnization. Applications that consume or produce PDF files are not required\nto preserve the existing structure of the Contents array. (See implementa-\ntion note 22 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('Rotate')\
          .name('Rotate')\
          .type('integer')\
          .comment('(Optional; inheritable) The number of degrees by which the page should\nbe rotated clockwise when displayed or printed. The value must be a mul-\ntiple of 90. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('Group')\
          .name('Group')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A group attributes dictionary specifying the attributes\nof the page\'s page group for use in the transparent imaging model (see\nSections 7.3.6, "Page Group," and 7.5.5, "Transparency Group XObjects").')\
          .done().done()\
      .optional()\
          .field('Thumb')\
          .name('Thumb')\
          .type('stream')\
          .comment('(Optional) A stream object defining the page\'s thumbnail image (see Sec-\ntion 8.2.3, "Thumbnail Images").')\
          .done().done()\
      .optional()\
          .field('B')\
          .name('B')\
          .type('array')\
          .comment('(Optional; PDF 1.1; recommended if the page contains article beads) An ar-\nray of indirect references to article beads appearing on the page (see Sec-\ntion 8.3.2, "Articles"; see also implementation note 23 in Appendix H).\nThe beads are listed in the array in natural reading order.')\
          .done().done()\
      .optional()\
          .field('Dur')\
          .name('Dur')\
          .type('number')\
          .comment('(Optional; PDF 1.1) The page\'s display duration (also called its advance\ntiming): the maximum length of time, in seconds, that the page will be\ndisplayed during presentations before the viewer application automati-\ncally advances to the next page (see Section 8.3.3, "Presentations"). By\ndefault, the viewer does not advance automatically.')\
          .done().done()\
      .optional()\
          .field('Trans')\
          .name('Trans')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.1) A transition dictionary describing the transition effect\nto be used when displaying the page during presentations (see Section\n8.3.3, "Presentations").')\
          .done().done()\
      .optional()\
          .field('Annots')\
          .name('Annots')\
          .type('array')\
          .comment('(Optional) An array of annotation dictionaries representing annotations\nassociated with the page (see Section 8.4, "Annotations").')\
          .done().done()\
      .optional()\
          .field('AA')\
          .name('AA')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An additional-actions dictionary defining actions to\nbe performed when the page is opened or closed (see Section 8.5.2, "Trig-\nger Events"; see also implementation note 24 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the page\n(see Section 9.2.2, "Metadata Streams").')\
          .done().done()\
      .optional()\
          .field('PieceInfo')\
          .name('PieceInfo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A page-piece dictionary associated with the page (see\nSection 9.4, "Page-Piece Dictionaries").')\
          .done().done()\
      .optional()\
          .field('StructParents')\
          .name('StructParents')\
          .type('integer')\
          .comment('(Required if the page contains structural content items; PDF 1.3) The inte-\nger key of the page\'s entry in the structural parent tree (see "Finding Struc-\nture Elements from Content Items" on page 600).')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('string')\
          .comment('(Optional; PDF 1.3; indirect reference preferred) The digital identifier of the\npage\'s parent Web Capture content set (see Section 9.9.5, "Object At-\ntributes Related to Web Capture").')\
          .done().done()\
      .optional()\
          .field('PZ')\
          .name('PZ')\
          .type('number')\
          .comment('(Optional; PDF 1.3) The page\'s preferred zoom (magnification) factor: the\nfactor by which it should be scaled to achieve the "natural" display magni-\nfication (see Section 9.9.5, "Object Attributes Related to Web Capture").')\
          .done().done()\
      .optional()\
          .field('SeparationInfo')\
          .name('SeparationInfo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A separation dictionary containing information need-\ned to generate color separations for the page (see Section 9.10.3, "Separa-\ntion Dictionaries").')\
          .done().done()\
      .done()

  pdfspec.addClass('NameDictionary', 'Dictionary', 'Entries in the name dictionary')\
      .optional()\
          .field('Dests')\
          .name('Dests')\
          .type('name tree')\
          .comment('(Optional; PDF 1.2) A name tree mapping name strings to destinations (see\n"Named Destinations" on page 476).')\
          .done().done()\
      .optional()\
          .field('AP')\
          .name('AP')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping name strings to annotation\nappearance streams (see Section 8.4.4, "Appearance Streams").')\
          .done().done()\
      .optional()\
          .field('JavaScript')\
          .name('JavaScript')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping name strings to document-level\nJavaScript(R) actions (see "JavaScript Actions" on page 556).')\
          .done().done()\
      .optional()\
          .field('Pages')\
          .name('Pages')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping name strings to visible pages for\nuse in interactive forms (see Section 8.6.5, "Named Pages").')\
          .done().done()\
      .optional()\
          .field('Templates')\
          .name('Templates')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping name strings to invisible (tem-\nplate) pages for use in interactive forms (see Section 8.6.5, "Named Pages").')\
          .done().done()\
      .optional()\
          .field('IDS')\
          .name('IDS')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping digital identifiers to Web Capture\ncontent sets (see Section 9.9.3, "Content Sets").')\
          .done().done()\
      .optional()\
          .field('URLS')\
          .name('URLS')\
          .type('name tree')\
          .comment('(Optional; PDF 1.3) A name tree mapping uniform resource locators (URLs)\nto Web Capture content sets (see Section 9.9.3, "Content Sets").')\
          .done().done()\
      .optional()\
          .field('EmbeddedFiles')\
          .name('EmbeddedFiles')\
          .type('name tree')\
          .comment('(Optional; PDF 1.4) A name tree mapping name strings to embedded file\nstreams (see Section 3.10.3, "Embedded File Streams").')\
          .done().done()\
      .done()

  pdfspec.addClass('ResourceDictionary', 'Dictionary', 'Entries in a resource dictionary')\
      .optional()\
          .field('ExtGState')\
          .name('ExtGState')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping resource names to graphics state parameter\ndictionaries (see Section 4.3.4, "Graphics State Parameter Dictionaries").')\
          .done().done()\
      .optional()\
          .field('ColorSpace')\
          .name('ColorSpace')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping each resource name to either the name of a\ndevice-dependent color space or an array describing a color space (see Sec-\ntion 4.5, "Color Spaces").')\
          .done().done()\
      .optional()\
          .field('Pattern')\
          .name('Pattern')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping resource names to pattern objects (see Sec-\ntion 4.6, "Patterns").')\
          .done().done()\
      .optional()\
          .field('Shading')\
          .name('Shading')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A dictionary mapping resource names to shading dic-\ntionaries (see "Shading Dictionaries" on page 233).')\
          .done().done()\
      .optional()\
          .field('XObject')\
          .name('XObject')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping resource names to external objects (see Sec-\ntion 4.7, "External Objects").')\
          .done().done()\
      .optional()\
          .field('Font')\
          .name('Font')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping resource names to font dictionaries (see\nChapter 5).')\
          .done().done()\
      .optional()\
          .field('ProcSet')\
          .name('ProcSet')\
          .type('array')\
          .comment('(Optional) An array of predefined procedure set names (see Section 9.1,\n"Procedure Sets").')\
          .done().done()\
      .optional()\
          .field('Properties')\
          .name('Properties')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) A dictionary mapping resource names to property list\ndictionaries for marked content (see Section 9.5.1, "Property Lists").')\
          .done().done()\
      .done()

  pdfspec.addClass('NameTreeNodeDictionary', 'Dictionary', 'Entries in a name tree node dictionary')\
      .optional()\
          .field('Kids')\
          .name('Kids')\
          .type('array')\
          .comment('(Root and intermediate nodes only; required in intermediate nodes; present in the root node\nif and only if Names is not present) An array of indirect references to the immediate chil-\ndren of this node. The children may be intermediate or leaf nodes.')\
          .done().done()\
      .optional()\
          .field('Names')\
          .name('Names')\
          .type('array')\
          .comment('(Root and leaf nodes only; required in leaf nodes; present in the root node if and only if Kids\nis not present) An array of the form\n    [key1 value1 key2 value2 ... keyn valuen ]\nwhere each keyi is a string and the corresponding valuei is an indirect reference to the\nobject associated with that key. The keys are sorted in lexical order, as described below.')\
          .done().done()\
      .optional()\
          .field('Limits')\
          .name('Limits')\
          .type('array')\
          .comment('(Intermediate and leaf nodes only; required) An array of two strings, specifying the (lexi-\ncally) least and greatest keys included in the Names array of a leaf node or in the Names\narrays of any leaf nodes that are descendants of an intermediate node.')\
          .done().done()\
      .done()

  pdfspec.addClass('NumberTreeNodeDictionary', 'Dictionary', 'Entries in a number tree node dictionary')\
      .optional()\
          .field('Kids')\
          .name('Kids')\
          .type('array')\
          .comment('(Root and intermediate nodes only; required in intermediate nodes; present in the root node\nif and only if Nums is not present) An array of indirect references to the immediate chil-\ndren of this node. The children may be intermediate or leaf nodes.')\
          .done().done()\
      .optional()\
          .field('Nums')\
          .name('Nums')\
          .type('array')\
          .comment('(Root and leaf nodes only; required in leaf nodes; present in the root node if and only if Kids\nis not present) An array of the form\n    [key1 value1 key2 value2 ... keyn valuen ]\nwhere each keyi is an integer and the corresponding valuei is an indirect reference to the\nobject associated with that key. The keys are sorted in numerical order, analogously to\nthe arrangement of keys in a name tree as described in Section 3.8.4, "Name Trees."')\
          .done().done()\
      .optional()\
          .field('Limits')\
          .name('Limits')\
          .type('array')\
          .comment('(Intermediate and leaf nodes only; required) An array of two integers, specifying the\n(numerically) least and greatest keys included in the Nums array of a leaf node or in the\nNums arrays of any leaf nodes that are descendants of an intermediate node.')\
          .done().done()\
      .done()

  pdfspec.addClass('FunctionCommonDictionary', 'Dictionary', 'Entries common to all function dictionaries')\
      .required('NULL')\
          .field('FunctionType')\
          .name('FunctionType')\
          .type('integer')\
          .comment('(Required) The function type:\n    0    Sampled function\n    2    Exponential interpolation function\n    3    Stitching function\n    4    PostScript calculator function')\
          .done().done()\
      .required('NULL')\
          .field('Domain')\
          .name('Domain')\
          .type('array')\
          .comment('(Required) An array of 2 x m numbers, where m is the number of input\nvalues. For each i from 0 to m - 1, Domain2i must be less than or equal to\nDomain2i+1 , and the ith input value, xi , must lie in the interval\nDomain2i <= xi <= Domain2i+1 . Input values outside the declared domain are\nclipped to the nearest boundary value.')\
          .done().done()\
      .optional()\
          .field('Range')\
          .name('Range')\
          .type('array')\
          .comment('(Required for type 0 and type 4 functions, optional otherwise; see below) An\narray of 2 x n numbers, where n is the number of output values. For each j\nfrom 0 to n - 1, Range2j must be less than or equal to Range2j+1 , and the jth\noutput value, yj , must lie in the interval Range2j <= yj <= Range2j+1 . Output\nvalues outside the declared range are clipped to the nearest boundary value.\nIf this entry is absent, no clipping is done.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type0FunctionDictionary', 'Dictionary', 'Additional entries specific to a type 0 function dictionary')\
      .required('NULL')\
          .field('Size')\
          .name('Size')\
          .type('array')\
          .comment('(Required) An array of m positive integers specifying the number of samples\nin each input dimension of the sample table.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerSample')\
          .name('BitsPerSample')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each sample. (If the function\nhas multiple output values, each one occupies BitsPerSample bits.) Valid\nvalues are 1, 2, 4, 8, 12, 16, 24, and 32.')\
          .done().done()\
      .optional()\
          .field('Order')\
          .name('Order')\
          .type('integer')\
          .comment('(Optional) The order of interpolation between samples. Valid values are 1\nand 3, specifying linear and cubic spline interpolation, respectively. (See im-\nplementation note 26 in Appendix H.) Default value: 1.')\
          .done().done()\
      .optional()\
          .field('Encode')\
          .name('Encode')\
          .type('array')\
          .comment('(Optional) An array of 2 x m numbers specifying the linear mapping of input\nvalues into the domain of the function\'s sample table. Default value:\n[0 (Size0 - 1) 0 (Size1 - 1) ...].')\
          .done().done()\
      .optional()\
          .field('Decode')\
          .name('Decode')\
          .type('array')\
          .comment('(Optional) An array of 2 x n numbers specifying the linear mapping of sam-\nple values into the range appropriate for the function\'s output values. Default\nvalue: same as the value of Range.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type2FunctionDictionary', 'Dictionary', 'Additional entries specific to a type 2 function dictionary')\
      .optional()\
          .field('C0')\
          .name('C0')\
          .type('array')\
          .comment('(Optional) An array of n numbers defining the function result when x = 0.0 (hence the "0"\nin the name). Default value: [0.0].')\
          .done().done()\
      .optional()\
          .field('C1')\
          .name('C1')\
          .type('array')\
          .comment('(Optional) An array of n numbers defining the function result when x = 1.0 (hence the "1"\nin the name). Default value: [1.0].')\
          .done().done()\
      .required('NULL')\
          .field('N')\
          .name('N')\
          .type('number')\
          .comment('(Required) The interpolation exponent. Each input value x will return n values, given by\nyj = C0j + xN x (C1j - C0j ), for 0 <= j < n.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type3FunctionDictionary', 'Dictionary', 'Additional entries specific to a type 3 function dictionary')\
      .required('NULL')\
          .field('Functions')\
          .name('Functions')\
          .type('array')\
          .comment('(Required) An array of k 1-input functions making up the stitching function. The out-\nput dimensionality of all functions must be the same, and compatible with the value of\nRange if Range is present.')\
          .done().done()\
      .required('NULL')\
          .field('Bounds')\
          .name('Bounds')\
          .type('array')\
          .comment('(Required) An array of k - 1 numbers that, in combination with Domain, define the\nintervals to which each function from the Functions array applies. Bounds elements\nmust be in order of increasing value, and each value must be within the domain\ndefined by Domain.')\
          .done().done()\
      .required('NULL')\
          .field('Encode')\
          .name('Encode')\
          .type('array')\
          .comment('(Required) An array of 2 x k numbers that, taken in pairs, map each subset of the do-\nmain defined by Domain and the Bounds array to the domain of the corresponding\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('FileSpecificationDictionary', 'Dictionary', 'Entries in a file specification dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required if an EF or RF entry is present; recommended always) The type of PDF object\nthat this dictionary describes; must be Filespec for a file specification dictionary.')\
          .done().done()\
      .optional()\
          .field('FS')\
          .name('FS')\
          .type('name')\
          .comment('(Optional) The name of the file system to be used to interpret this file specification. If\nthis entry is present, all other entries in the dictionary are interpreted by the desig-\nnated file system. PDF defines only one standard file system, URL (see Section 3.10.4,\n"URL Specifications"); a viewer application or plug-in extension can register a differ-\nent one (see Appendix E). Note that this entry is independent of the F, DOS, Mac, and\nUnix entries.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('string')\
          .comment('(Required if the DOS, Mac, and Unix entries are all absent) A file specification string of\nthe form described in Section 3.10.1, "File Specification Strings," or (if the file system\nis URL) a uniform resource locator, as described in Section 3.10.4, "URL Specifica-\ntions."')\
          .done().done()\
      .optional()\
          .field('DOS')\
          .name('DOS')\
          .type('string')\
          .comment('(Optional) A file specification string (see Section 3.10.1, "File Specification Strings")\nrepresenting a DOS file name.')\
          .done().done()\
      .optional()\
          .field('Mac')\
          .name('Mac')\
          .type('string')\
          .comment('(Optional) A file specification string (see Section 3.10.1, "File Specification Strings")\nrepresenting a Mac OS file name.')\
          .done().done()\
      .optional()\
          .field('Unix')\
          .name('Unix')\
          .type('string')\
          .comment('(Optional) A file specification string (see Section 3.10.1, "File Specification Strings")\nrepresenting a UNIX file name.')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('array')\
          .comment('(Optional) An array of two strings constituting a file identifier (see Section 9.3, "File\nIdentifiers") that is also included in the referenced file. The use of this entry improves\na viewer application\'s chances of finding the intended file and allows it to warn the\nuser if the file has changed since the link was made.')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('boolean')\
          .comment('(Optional; PDF 1.2) A flag indicating whether the file referenced by the file specifica-\ntion is volatile (changes frequently with time). If the value is true, viewer applications\nshould never cache a copy of the file. For example, a movie annotation referencing a\nURL to a live video camera could set this flag to true, notifying the application that it\nshould reacquire the movie each time it is played. Default value: false.')\
          .done().done()\
      .optional()\
          .field('EF')\
          .name('EF')\
          .type('dictionary')\
          .comment('(Required if RF is present; PDF 1.3) A dictionary containing a subset of the keys F,\nDOS, Mac, and Unix, corresponding to the entries by those names in the file specifica-\ntion dictionary. The value of each such key is an embedded file stream (see Section\n3.10.3, "Embedded File Streams") containing the corresponding file. If this entry is\npresent, the Type entry is required and the file specification dictionary must be indi-\nrectly referenced.')\
          .done().done()\
      .optional()\
          .field('RF')\
          .name('RF')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A dictionary with the same structure as the EF dictionary, which\nmust also be present. Each key in the RF dictionary must also be present in the EF dic-\ntionary. Each value is a related files array (see "Related Files Arrays" on page 125)\nidentifying files that are related to the corresponding file in the EF dictionary. If this\nentry is present, the Type entry is required and the file specification dictionary must\nbe indirectly referenced.')\
          .done().done()\
      .done()

  pdfspec.addClass('EmbeddedFileStreamDictionary', 'Dictionary', 'Additional entries in an embedded file stream dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be EmbeddedFile for an embedded file stream.')\
          .done().done()\
      .optional()\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Optional) The subtype of the embedded file. The value of this entry must be\na first-class name, as defined in Appendix E. Names without a registered pre-\nfix must conform to the MIME media type names defined in Internet RFC\n2046, Multipurpose Internet Mail Extensions (MIME), Part Two: Media Types\n(see the Bibliography), with the provision that characters not allowed in\nnames must use the 2-character hexadecimal code format described in Sec-\ntion 3.2.4, "Name Objects."')\
          .done().done()\
      .optional()\
          .field('Params')\
          .name('Params')\
          .type('dictionary')\
          .comment('(Optional) An embedded file parameter dictionary containing additional, file-\nspecific information (see Table 3.34).')\
          .done().done()\
      .done()

  pdfspec.addClass('EmbeddedFileParameterDictionary', 'Dictionary', 'Entries in an embedded file parameter dictionary')\
      .optional()\
          .field('Size')\
          .name('Size')\
          .type('integer')\
          .comment('(Optional) The size of the embedded file, in bytes.')\
          .done().done()\
      .optional()\
          .field('CreationDate')\
          .name('CreationDate')\
          .type('date')\
          .comment('(Optional) The date and time when the embedded file was created.')\
          .done().done()\
      .optional()\
          .field('ModDate')\
          .name('ModDate')\
          .type('date')\
          .comment('(Optional) The date and time when the embedded file was last modified.')\
          .done().done()\
      .optional()\
          .field('Mac')\
          .name('Mac')\
          .type('dictionary')\
          .comment('(Optional) A subdictionary containing additional information specific to\nMac OS files (see Table 3.35).')\
          .done().done()\
      .optional()\
          .field('CheckSum')\
          .name('CheckSum')\
          .type('string')\
          .comment('(Optional) A 16-byte string that is the checksum of the bytes of the uncom-\npressed embedded file. The checksum is calculated by applying the standard\nMD5 message-digest algorithm (described in Internet RFC 1321, The MD5\nMessage-Digest Algorithm; see the Bibliography) to the bytes of the embedded\nfile stream.')\
          .done().done()\
      .done()

  pdfspec.addClass('MacOsFileInformationDictionary', 'Dictionary', 'Entries in a Mac OS file information dictionary')\
      .optional()\
          .field('Subtype')\
          .name('Subtype')\
          .type('string')\
          .comment('(Optional) The embedded file\'s file type.')\
          .done().done()\
      .optional()\
          .field('Creator')\
          .name('Creator')\
          .type('string')\
          .comment('(Optional) The embedded file\'s creator signature.')\
          .done().done()\
      .optional()\
          .field('ResFork')\
          .name('ResFork')\
          .type('stream')\
          .comment('(Optional) The binary contents of the embedded file\'s resource fork.')\
          .done().done()\
      .done()

  pdfspec.addClass('GraphicsStateDictionary', 'Dictionary', 'Entries in a graphics state parameter dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; must be\nExtGState for a graphics state parameter dictionary.')\
          .done().done()\
      .optional()\
          .field('LW')\
          .name('LW')\
          .type('number')\
          .comment('(Optional; PDF 1.3) The line width (see "Line Width" on page 152).')\
          .done().done()\
      .optional()\
          .field('LC')\
          .name('LC')\
          .type('integer')\
          .comment('(Optional; PDF 1.3) The line cap style (see "Line Cap Style" on page 153).')\
          .done().done()\
      .optional()\
          .field('LJ')\
          .name('LJ')\
          .type('integer')\
          .comment('(Optional; PDF 1.3) The line join style (see "Line Join Style" on page 153).')\
          .done().done()\
      .optional()\
          .field('ML')\
          .name('ML')\
          .type('number')\
          .comment('(Optional; PDF 1.3) The miter limit (see "Miter Limit" on page 153).')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('array')\
          .comment('(Optional; PDF 1.3) The line dash pattern, expressed as an array of the form\n[dashArray dashPhase], where dashArray is itself an array and dashPhase is an\ninteger (see "Line Dash Pattern" on page 155).')\
          .done().done()\
      .optional()\
          .field('RI')\
          .name('RI')\
          .type('name')\
          .comment('(Optional; PDF 1.3) The name of the rendering intent (see "Rendering\nIntents" on page 197).')\
          .done().done()\
      .optional()\
          .field('OP')\
          .name('OP')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to apply overprint (see Section 4.5.6,\n"Overprint Control"). In PDF 1.2 and earlier, there is a single overprint\nparameter that applies to all painting operations. Beginning with PDF 1.3,\nthere are two separate overprint parameters: one for stroking and one for all\nother painting operations. Specifying an OP entry sets both parameters un-\nless there is also an op entry in the same graphics state parameter dictionary,\nin which case the OP entry sets only the overprint parameter for stroking.')\
          .done().done()\
      .optional()\
          .field('op')\
          .name('op')\
          .type('boolean')\
          .comment('(Optional; PDF 1.3) A flag specifying whether to apply overprint (see Section\n4.5.6, "Overprint Control") for painting operations other than stroking. If\nthis entry is absent, the OP entry, if any, sets this parameter.')\
          .done().done()\
      .optional()\
          .field('OPM')\
          .name('OPM')\
          .type('integer')\
          .comment('(Optional; PDF 1.3) The overprint mode (see Section 4.5.6, "Overprint Con-\ntrol").')\
          .done().done()\
      .optional()\
          .field('Font')\
          .name('Font')\
          .type('array')\
          .comment('(Optional; PDF 1.3) An array of the form [font size], where font is an indirect\nreference to a font dictionary and size is a number expressed in text space\nunits. These two objects correspond to the operands of the Tf operator (see\nSection 5.2, "Text State Parameters and Operators"); however, the first oper-\nand is an indirect object reference instead of a resource name.')\
          .done().done()\
      .optional()\
          .field('BG')\
          .name('BG')\
          .type('function')\
          .comment('(Optional) The black-generation function, which maps the interval [0.0 1.0]\nto the interval [0.0 1.0] (see Section 6.2.3, "Conversion from DeviceRGB to\nDeviceCMYK").')\
          .done().done()\
      .optional()\
          .field('BG2')\
          .name('BG2')\
          .type('function or name')\
          .comment('(Optional; PDF 1.3) Same as BG except that the value may also be the name\nDefault, denoting the black-generation function that was in effect at the start\nof the page. If both BG and BG2 are present in the same graphics state param-\neter dictionary, BG2 takes precedence.')\
          .done().done()\
      .optional()\
          .field('UCR')\
          .name('UCR')\
          .type('function')\
          .comment('(Optional) The undercolor-removal function, which maps the interval\n[0.0 1.0] to the interval [-1.0 1.0] (see Section 6.2.3, "Conversion from\nDeviceRGB to DeviceCMYK").')\
          .done().done()\
      .optional()\
          .field('UCR2')\
          .name('UCR2')\
          .type('function or name')\
          .comment('(Optional; PDF 1.3) Same as UCR except that the value may also be the name\nDefault, denoting the undercolor-removal function that was in effect at the\nstart of the page. If both UCR and UCR2 are present in the same graphics state\nparameter dictionary, UCR2 takes precedence.')\
          .done().done()\
      .optional()\
          .field('TR')\
          .name('TR')\
          .type('function, array, or name')\
          .comment('(Optional) The transfer function, which maps the interval [0.0 1.0] to the\ninterval [0.0 1.0] (see Section 6.3, "Transfer Functions"). The value is either\na single function (which applies to all process colorants) or an array of four\nfunctions (which apply to the process colorants individually). The name\nIdentity may be used to represent the identity function.')\
          .done().done()\
      .optional()\
          .field('TR2')\
          .name('TR2')\
          .type('function, array, or name')\
          .comment('(Optional; PDF 1.3) Same as TR except that the value may also be the name\nDefault, denoting the transfer function that was in effect at the start of the\npage. If both TR and TR2 are present in the same graphics state parameter dic-\ntionary, TR2 takes precedence.')\
          .done().done()\
      .optional()\
          .field('HT')\
          .name('HT')\
          .type('dictionary, stream, or name')\
          .comment('(Optional) The halftone dictionary or stream (see Section 6.4, "Halftones")\nor the name Default, denoting the halftone that was in effect at the start of the\npage.')\
          .done().done()\
      .optional()\
          .field('FL')\
          .name('FL')\
          .type('number')\
          .comment('(Optional; PDF 1.3) The flatness tolerance (see Section 6.5.1, "Flatness Toler-\nance").')\
          .done().done()\
      .optional()\
          .field('SM')\
          .name('SM')\
          .type('number')\
          .comment('(Optional; PDF 1.3) The smoothness tolerance (see Section 6.5.2, "Smooth-\nness Tolerance").')\
          .done().done()\
      .optional()\
          .field('SA')\
          .name('SA')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to apply automatic stroke adjustment\n(see Section 6.5.4, "Automatic Stroke Adjustment").')\
          .done().done()\
      .optional()\
          .field('BM')\
          .name('BM')\
          .type('name or array')\
          .comment('(Optional; PDF 1.4) The current blend mode to be used in the transparent\nimaging model (see Sections 7.2.4, "Blend Mode," and 7.5.2, "Specifying\nBlending Color Space and Blend Mode").')\
          .done().done()\
      .optional()\
          .field('SMask')\
          .name('SMask')\
          .type('dictionary or name')\
          .comment('(Optional; PDF 1.4) The current soft mask, specifying the mask shape or\nmask opacity values to be used in the transparent imaging model (see\n"Source Shape and Opacity" on page 421 and "Mask Shape and Opacity" on\npage 443).\nNote: Although the current soft mask is sometimes referred to as a "soft clip,"\naltering it with the gs operator completely replaces the old value with the new\none, rather than intersecting the two as is done with the current clipping path\nparameter (see Section 4.4.3, "Clipping Path Operators").')\
          .done().done()\
      .optional()\
          .field('CA')\
          .name('CA')\
          .type('number')\
          .comment('(Optional; PDF 1.4) The current stroking alpha constant, specifying the con-\nstant shape or constant opacity value to be used for stroking operations in the\ntransparent imaging model (see "Source Shape and Opacity" on page 421\nand "Constant Shape and Opacity" on page 444).')\
          .done().done()\
      .optional()\
          .field('ca')\
          .name('ca')\
          .type('number')\
          .comment('(Optional; PDF 1.4) Same as CA, but for nonstroking operations.')\
          .done().done()\
      .optional()\
          .field('AIS')\
          .name('AIS')\
          .type('boolean')\
          .comment('(Optional; PDF 1.4) The alpha source flag ("alpha is shape"), specifying\nwhether the current soft mask and alpha constant are to be interpreted as\nshape values (true) or opacity values (false).')\
          .done().done()\
      .optional()\
          .field('TK')\
          .name('TK')\
          .type('boolean')\
          .comment('(Optional; PDF 1.4) The text knockout flag, which determines the behavior\nof overlapping glyphs within a text object in the transparent imaging model\n(see Section 5.2.7, "Text Knockout").')\
          .done().done()\
      .done()

  pdfspec.addClass('CalgrayColorSpaceDictionary', 'Dictionary', 'Entries in a CalGray color space dictionary')\
      .required('NULL')\
          .field('WhitePoint')\
          .name('WhitePoint')\
          .type('array')\
          .comment('(Required) An array of three numbers [XW YW ZW ] specifying the tri-\nstimulus value, in the CIE 1931 XYZ space, of the diffuse white point; see\n"CalRGB Color Spaces," below, for further discussion. The numbers XW and\nZW must be positive, and YW must be equal to 1.0.')\
          .done().done()\
      .optional()\
          .field('BlackPoint')\
          .name('BlackPoint')\
          .type('array')\
          .comment('(Optional) An array of three numbers [ XB YB ZB ] specifying the tristimulus\nvalue, in the CIE 1931 XYZ space, of the diffuse black point; see "CalRGB\nColor Spaces," below, for further discussion. All three of these numbers must\nbe nonnegative. Default value: [0.0 0.0 0.0].')\
          .done().done()\
      .optional()\
          .field('Gamma')\
          .name('Gamma')\
          .type('number')\
          .comment('(Optional) A number G defining the gamma for the gray (A) component. G\nmust be positive and will generally be greater than or equal to 1. Default\nvalue: 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('CalrgbColorSpaceDictionary', 'Dictionary', 'Entries in a CalRGB color space dictionary')\
      .required('NULL')\
          .field('WhitePoint')\
          .name('WhitePoint')\
          .type('array')\
          .comment('(Required) An array of three numbers [ XW YW ZW ] specifying the tristimulus value,\nin the CIE 1931 XYZ space, of the diffuse white point; see below for further discus-\nsion. The numbers XW and ZW must be positive, and YW must be equal to 1.0.')\
          .done().done()\
      .optional()\
          .field('BlackPoint')\
          .name('BlackPoint')\
          .type('array')\
          .comment('(Optional) An array of three numbers [ XB YB ZB ] specifying the tristimulus value, in\nthe CIE 1931 XYZ space, of the diffuse black point; see below for further discussion.\nAll three of these numbers must be nonnegative. Default value: [0.0 0.0 0.0].')\
          .done().done()\
      .optional()\
          .field('Gamma')\
          .name('Gamma')\
          .type('array')\
          .comment('(Optional) An array of three numbers [ GR GG GB ] specifying the gamma for the red,\ngreen, and blue (A, B, and C) components of the color space. Default value:\n[1.0 1.0 1.0].')\
          .done().done()\
      .optional()\
          .field('Matrix')\
          .name('Matrix')\
          .type('array')\
          .comment('(Optional) An array of nine numbers [ XA YA ZA XB YB ZB XC YC ZC ] specifying\nthe linear interpretation of the decoded A, B, and C components of the color space\nwith respect to the final XYZ representation. Default value: the identity matrix\n[1 0 0 0 1 0 0 0 1].')\
          .done().done()\
      .done()

  pdfspec.addClass('LabColorSpaceDictionary', 'Dictionary', 'Entries in a Lab color space dictionary')\
      .required('NULL')\
          .field('WhitePoint')\
          .name('WhitePoint')\
          .type('array')\
          .comment('(Required) An array of three numbers [ XW YW ZW ] specifying the tristimulus value,\nin the CIE 1931 XYZ space, of the diffuse white point; see "CalRGB Color Spaces" on\npage 184 for further discussion. The numbers XW and ZW must be positive, and YW\nmust be equal to 1.0.')\
          .done().done()\
      .optional()\
          .field('BlackPoint')\
          .name('BlackPoint')\
          .type('array')\
          .comment('(Optional) An array of three numbers [ XB YB ZB ] specifying the tristimulus value, in\nthe CIE 1931 XYZ space, of the diffuse black point; see "CalRGB Color Spaces" on\npage 184 for further discussion. All three of these numbers must be nonnegative.\nDefault value: [0.0 0.0 0.0].')\
          .done().done()\
      .optional()\
          .field('Range')\
          .name('Range')\
          .type('array')\
          .comment('(Optional) An array of four numbers [ amin amax bmin bmax ] specifying the range of\nvalid values for the a* and b* (B and C) components of the color space-that is,\n      a min <= a* <= a max\nand\n      b min <= b* <= b max\nComponent values falling outside the specified range will be adjusted to the nearest\nvalid value without error indication. Default value: [ -100 100 -100 100].')\
          .done().done()\
      .done()

  pdfspec.addClass('IccProfileStreamDictionary', 'Dictionary', 'Additional entries specific to an ICC profile stream dictionary')\
      .required('NULL')\
          .field('N')\
          .name('N')\
          .type('integer')\
          .comment('(Required) The number of color components in the color space described by the ICC\nprofile data. This number must match the number of components actually in the ICC\nprofile. As of PDF 1.4, N must be 1, 3, or 4.')\
          .done().done()\
      .optional()\
          .field('Alternate')\
          .name('Alternate')\
          .type('array or name')\
          .comment('(Optional) An alternate color space to be used in case the one specified in the stream\ndata is not supported (for example, by viewer applications designed for earlier\nversions of PDF). The alternate space may be any valid color space (except a Pattern\ncolor space) that has the number of components specified by N. If this entry is omit-\nted and the viewer application does not understand the ICC profile data, the color\nspace used will be DeviceGray, DeviceRGB, or DeviceCMYK, depending on whether\nthe value of N is 1, 3, or 4, respectively.\nNote: Note that there is no conversion of source color values, such as a tint transforma-\ntion, when using the alternate color space. Color values that are within the range of the\nICCBased color space might not be within the range of the alternate color space. In this\ncase, the nearest values within the range of the alternate space will be substituted.')\
          .done().done()\
      .optional()\
          .field('Range')\
          .name('Range')\
          .type('array')\
          .comment('(Optional) An array of 2 x N numbers [min0 max0 min1 max1 ... ] specifying the\nminimum and maximum valid values of the corresponding color components.\nThese values must match the information in the ICC profile. Default value:\n[0.0 1.0 0.0 1.0 ...].')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the color space (see\nSection 9.2.2, "Metadata Streams").')\
          .done().done()\
      .done()

  pdfspec.addClass('DeviceNColorSpaceDictionary', 'Dictionary', 'Entry in a DeviceN color space attributes dictionary')\
      .optional()\
          .field('Colorants')\
          .name('Colorants')\
          .type('dictionary')\
          .comment('(Optional) A dictionary describing the individual colorants used in the DeviceN\ncolor space. For each entry in this dictionary, the key is a colorant name and the\nvalue is an array defining a Separation color space for that colorant (see "Separa-\ntion Color Spaces" on page 201). The key must match the colorant name given in\nthat color space. The dictionary need not list all colorants used in the DeviceN\ncolor space and may list additional colorants.\nThis dictionary has no effect on the operation of the DeviceN color space itself or\nthe appearance that it produces. However, it provides information about the indi-\nvidual colorants that may be useful to some applications. In particular, the alter-\nnate color space and tint transformation function of a Separation color space\ndescribe the appearance of that colorant alone, whereas those of a DeviceN color\nspace describe only the appearance of its colorants in combination.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type1PatternDictionary', 'Dictionary', 'Additional entries specific to a type 1 pattern dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Pattern for a pattern dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('PatternType')\
          .name('PatternType')\
          .type('integer')\
          .comment('(Required) A code identifying the type of pattern that this dictionary de-\nscribes; must be 1 for a tiling pattern.')\
          .done().done()\
      .required('NULL')\
          .field('PaintType')\
          .name('PaintType')\
          .type('integer')\
          .comment('(Required) A code that determines how the color of the pattern cell is to be\nspecified:\n   1     Colored tiling pattern. The pattern\'s content stream itself specifies the\n         colors used to paint the pattern cell. When the content stream begins\n         execution, the current color is the one that was initially in effect in the\n         pattern\'s parent content stream. (This is similar to the definition of the\n         pattern matrix; see Section 4.6.1, "General Properties of Patterns.")\n   2     Uncolored tiling pattern. The pattern\'s content stream does not speci-\n         fy any color information. Instead, the entire pattern cell is painted\n         with a separately specified color each time the pattern is used. Essen-\n         tially, the content stream describes a stencil through which the cur-')\
          .done().done()\
      .done()

  pdfspec.addClass('Type2PatternDictionary', 'Dictionary', 'Entries in a type 2 pattern dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('integer')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Pattern for a pattern dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('PatternType')\
          .name('PatternType')\
          .type('integer')\
          .comment('(Required) A code identifying the type of pattern that this dictionary de-\nscribes; must be 2 for a shading pattern.')\
          .done().done()\
      .required('NULL')\
          .field('Shading')\
          .name('Shading')\
          .type('dictionary or stream')\
          .comment('(Required) A shading object (see below) defining the shading pattern\'s gradient\nfill. The contents of the dictionary consist of the entries in Table 4.25 on page\n234, plus those in one of Tables 4.26 to 4.31 on pages 237 to 253.')\
          .done().done()\
      .optional()\
          .field('Matrix')\
          .name('Matrix')\
          .type('array')\
          .comment('(Optional) An array of six numbers specifying the pattern matrix (see Section\n4.6.1, "General Properties of Patterns"). Default value: the identity matrix\n[1 0 0 1 0 0].')\
          .done().done()\
      .optional()\
          .field('ExtGState')\
          .name('ExtGState')\
          .type('dictionary')\
          .comment('(Optional) A graphics state parameter dictionary (see Section 4.3.4, "Graph-\nics State Parameter Dictionaries") containing graphics state parameters to be\nput into effect temporarily while the shading pattern is painted. Any parame-\nters that are not so specified are inherited from the graphics state that was in\neffect at the beginning of the content stream in which the pattern is defined\nas a resource.')\
          .done().done()\
      .done()

  pdfspec.addClass('ShadingDictionary', 'Dictionary', 'Entries common to all shading dictionaries')\
      .required('NULL')\
          .field('ShadingType')\
          .name('ShadingType')\
          .type('integer')\
          .comment('(Required) The shading type:\n    1    Function-based shading\n    2    Axial shading\n    3    Radial shading\n    4    Free-form Gouraud-shaded triangle mesh\n    5    Lattice-form Gouraud-shaded triangle mesh\n    6    Coons patch mesh\n    7    Tensor-product patch mesh')\
          .done().done()\
      .required('NULL')\
          .field('ColorSpace')\
          .name('ColorSpace')\
          .type('name or array')\
          .comment('(Required) The color space in which color values are expressed. This may be\nany device, CIE-based, or special color space except a Pattern space. See\n"Color Space: Special Considerations," below, for further information.')\
          .done().done()\
      .optional()\
          .field('Background')\
          .name('Background')\
          .type('array')\
          .comment('(Optional) An array of color components appropriate to the color space,\nspecifying a single background color value. If present, this color is used be-\nfore any painting operation involving the shading, to fill those portions of the\narea to be painted that lie outside the bounds of the shading object itself. In\nthe opaque imaging model, the effect is as if the painting operation were\nperformed twice: first with the background color and then again with the\nshading.\nNote: The background color is applied only when the shading is used as part of a\nshading pattern, not when it is painted directly with the sh operator.')\
          .done().done()\
      .optional()\
          .field('BBox')\
          .name('BBox')\
          .type('rectangle')\
          .comment('(Optional) An array of four numbers giving the left, bottom, right, and top\ncoordinates, respectively, of the shading\'s bounding box. The coordinates are\ninterpreted in the shading\'s target coordinate space. If present, this bounding\nbox is applied as a temporary clipping boundary when the shading is painted,\nin addition to the current clipping path and any other clipping boundaries in\neffect at that time.')\
          .done().done()\
      .optional()\
          .field('AntiAlias')\
          .name('AntiAlias')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether to filter the shading function to prevent\naliasing artifacts. The shading operators sample shading functions at a rate\ndetermined by the resolution of the output device. Aliasing can occur if the\nfunction is not smooth-that is, if it has a high spatial frequency relative to\nthe sampling rate. Anti-aliasing can be computationally expensive and is usu-\nally unnecessary, since most shading functions are smooth enough, or are\nsampled at a high enough frequency, to avoid aliasing effects. Anti-aliasing\nmay not be implemented on some output devices, in which case this flag is\nignored. Default value: false.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type1ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 1 shading dictionary')\
      .optional()\
          .field('Domain')\
          .name('Domain')\
          .type('array')\
          .comment('(Optional) An array of four numbers [ xmin xmax ymin ymax ] specifying the rec-\ntangular domain of coordinates over which the color function(s) are defined.\nDefault value: [0.0 1.0 0.0 1.0].')\
          .done().done()\
      .optional()\
          .field('Matrix')\
          .name('Matrix')\
          .type('array')\
          .comment('(Optional) An array of six numbers specifying a transformation matrix mapping\nthe coordinate space specified by the Domain entry into the shading\'s target co-\nordinate space. For example, to map the domain rectangle [0.0 1.0 0.0 1.0] to a\n1-inch square with lower-left corner at coordinates (100, 100) in default user\nspace, the Matrix value would be [72 0 0 72 100 100]. Default value: the iden-\ntity matrix [1 0 0 1 0 0].')\
          .done().done()\
      .required('NULL')\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Required) A 2-in, n-out function or an array of n 2-in, 1-out functions (where n\nis the number of color components in the shading dictionary\'s color space).\nEach function\'s domain must be a superset of that of the shading dictionary. If\nthe value returned by the function for a given color component is out of range, it\nwill be adjusted to the nearest valid value.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type2ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 2 shading dictionary')\
      .required('NULL')\
          .field('Coords')\
          .name('Coords')\
          .type('array')\
          .comment('(Required) An array of four numbers [ x0 y0 x1 y1 ] specifying the starting and\nending coordinates of the axis, expressed in the shading\'s target coordinate\nspace.')\
          .done().done()\
      .optional()\
          .field('Domain')\
          .name('Domain')\
          .type('array')\
          .comment('(Optional) An array of two numbers [ t0 t1 ] specifying the limiting values of a\nparametric variable t. The variable is considered to vary linearly between these\ntwo values as the color gradient varies between the starting and ending points of\nthe axis. The variable t becomes the input argument to the color function(s).\nDefault value: [0.0 1.0].')\
          .done().done()\
      .required('NULL')\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Required) A 1-in, n-out function or an array of n 1-in, 1-out functions (where n\nis the number of color components in the shading dictionary\'s color space). The\nfunction(s) are called with values of the parametric variable t in the domain de-\nfined by the Domain entry. Each function\'s domain must be a superset of that of\nthe shading dictionary. If the value returned by the function for a given color\ncomponent is out of range, it will be adjusted to the nearest valid value.')\
          .done().done()\
      .optional()\
          .field('Extend')\
          .name('Extend')\
          .type('array')\
          .comment('(Optional) An array of two boolean values specifying whether to extend the\nshading beyond the starting and ending points of the axis, respectively. Default\nvalue: [false false].')\
          .done().done()\
      .done()

  pdfspec.addClass('Type3ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 3 shading dictionary')\
      .required('NULL')\
          .field('Coords')\
          .name('Coords')\
          .type('array')\
          .comment('(Required) An array of six numbers [ x0 y0 r0 x1 y1 r1 ] specifying the centers and\nradii of the starting and ending circles, expressed in the shading\'s target coor-\ndinate space. The radii r0 and r1 must both be greater than or equal to 0. If one\nradius is 0, the corresponding circle is treated as a point; if both are 0, nothing is\npainted.')\
          .done().done()\
      .optional()\
          .field('Domain')\
          .name('Domain')\
          .type('array')\
          .comment('(Optional) An array of two numbers [ t0 t1 ] specifying the limiting values of a\nparametric variable t. The variable is considered to vary linearly between these\ntwo values as the color gradient varies between the starting and ending circles.\nThe variable t becomes the input argument to the color function(s). Default\nvalue: [0.0 1.0].')\
          .done().done()\
      .required('NULL')\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Required) A 1-in, n-out function or an array of n 1-in, 1-out functions (where n\nis the number of color components in the shading dictionary\'s color space). The\nfunction(s) are called with values of the parametric variable t in the domain de-\nfined by the shading dictionary\'s Domain entry. Each function\'s domain must be\na superset of that of the shading dictionary. If the value returned by the function\nfor a given color component is out of range, it will be adjusted to the nearest\nvalid value.')\
          .done().done()\
      .optional()\
          .field('Extend')\
          .name('Extend')\
          .type('array')\
          .comment('(Optional) An array of two boolean values specifying whether to extend the\nshading beyond the starting and ending circles, respectively. Default value:\n[false false].')\
          .done().done()\
      .done()

  pdfspec.addClass('Type4ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 4 shading dictionary')\
      .required('NULL')\
          .field('BitsPerCoordinate')\
          .name('BitsPerCoordinate')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each vertex coordinate.\nValid values are 1, 2, 4, 8, 12, 16, 24, and 32.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerComponent')\
          .name('BitsPerComponent')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each color component.\nValid values are 1, 2, 4, 8, 12, and 16.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerFlag')\
          .name('BitsPerFlag')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent the edge flag for each ver-\ntex (see below). Valid values of BitsPerFlag are 2, 4, and 8, but only the\nleast significant 2 bits in each flag value are used. Valid values for the edge\nflag itself are 0, 1, and 2.')\
          .done().done()\
      .required('NULL')\
          .field('Decode')\
          .name('Decode')\
          .type('rectangle')\
          .comment('(Required) An array of numbers specifying how to map vertex coordinates\nand color components into the appropriate ranges of values. The de-\ncoding method is similar to that used in image dictionaries (see "Decode\nArrays" on page 271). The ranges are specified as follows:\n    [ xmin xmax ymin ymax c1,min c1,max ... cn,min cn,max ]\nNote that only one pair of c values should be specified if a Function entry\nis present.')\
          .done().done()\
      .optional()\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Optional) A 1-in, n-out function or an array of n 1-in, 1-out functions\n(where n is the number of color components in the shading dictionary\'s\ncolor space). If this entry is present, the color data for each vertex must be\nspecified by a single parametric variable rather than by n separate color\ncomponents; the designated function(s) will be called with each interpo-\nlated value of the parametric variable to determine the actual color at each\npoint. Each input value will be forced into the range interval specified for\nthe corresponding color component in the shading dictionary\'s Decode\narray. Each function\'s domain must be a superset of that interval. If the\nvalue returned by the function for a given color component is out of\nrange, it will be adjusted to the nearest valid value.\nThis entry may not be used with an Indexed color space.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type5ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 5 shading dictionary')\
      .required('NULL')\
          .field('BitsPerCoordinate')\
          .name('BitsPerCoordinate')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each vertex coordinate.\nValid values are 1, 2, 4, 8, 12, 16, 24, and 32.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerComponent')\
          .name('BitsPerComponent')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each color component.\nValid values are 1, 2, 4, 8, 12, and 16.')\
          .done().done()\
      .required('NULL')\
          .field('VerticesPerRow')\
          .name('VerticesPerRow')\
          .type('integer')\
          .comment('(Required) The number of vertices in each row of the lattice; the value\nmust be greater than or equal to 2. The number of rows need not be\nspecified.')\
          .done().done()\
      .required('NULL')\
          .field('Decode')\
          .name('Decode')\
          .type('array')\
          .comment('(Required) An array of numbers specifying how to map vertex coordinates\nand color components into the appropriate ranges of values. The de-\ncoding method is similar to that used in image dictionaries (see "Decode\nArrays" on page 271). The ranges are specified as follows:\n    [ xmin xmax ymin ymax c1,min c1,max ... cn,min cn,max ]\nNote that only one pair of c values should be specified if a Function entry\nis present.')\
          .done().done()\
      .optional()\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Optional) A 1-in, n-out function or an array of n 1-in, 1-out functions\n(where n is the number of color components in the shading dictionary\'s\ncolor space). If this entry is present, the color data for each vertex must be\nspecified by a single parametric variable rather than by n separate color\ncomponents; the designated function(s) will be called with each interpo-\nlated value of the parametric variable to determine the actual color at each\npoint. Each input value will be forced into the range interval specified for\nthe corresponding color component in the shading dictionary\'s Decode\narray. Each function\'s domain must be a superset of that interval. If the\nvalue returned by the function for a given color component is out of\nrange, it will be adjusted to the nearest valid value.\nThis entry may not be used with an Indexed color space.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type6ShadingDictionary', 'ShadingDictionary', 'Additional entries specific to a type 6 shading dictionary')\
      .required('NULL')\
          .field('BitsPerCoordinate')\
          .name('BitsPerCoordinate')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each geometric coordi-\nnate. Valid values are 1, 2, 4, 8, 12, 16, 24, and 32.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerComponent')\
          .name('BitsPerComponent')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent each color component.\nValid values are 1, 2, 4, 8, 12, and 16.')\
          .done().done()\
      .required('NULL')\
          .field('BitsPerFlag')\
          .name('BitsPerFlag')\
          .type('integer')\
          .comment('(Required) The number of bits used to represent the edge flag for each\npatch (see below). Valid values of BitsPerFlag are 2, 4, and 8, but only the\nleast significant 2 bits in each flag value are used. Valid values for the edge\nflag itself are 0, 1, 2, and 3.')\
          .done().done()\
      .required('NULL')\
          .field('Decode')\
          .name('Decode')\
          .type('array')\
          .comment('(Required) An array of numbers specifying how to map coordinates and\ncolor components into the appropriate ranges of values. The decoding\nmethod is similar to that used in image dictionaries (see "Decode Arrays"\non page 271). The ranges are specified as follows:\n    [ xmin xmax ymin ymax c1,min c1,max ... cn,min cn,max ]\nNote that only one pair of c values should be specified if a Function entry\nis present.')\
          .done().done()\
      .optional()\
          .field('Function')\
          .name('Function')\
          .type('function')\
          .comment('(Optional) A 1-in, n-out function or an array of n 1-in, 1-out functions\n(where n is the number of color components in the shading dictionary\'s\ncolor space). If this entry is present, the color data for each vertex must be\nspecified by a single parametric variable rather than by n separate color\ncomponents; the designated function(s) will be called with each interpo-\nlated value of the parametric variable to determine the actual color at each\npoint. Each input value will be forced into the range interval specified for\nthe corresponding color component in the shading dictionary\'s Decode\narray. Each function\'s domain must be a superset of that interval. If the\nvalue returned by the function for a given color component is out of\nrange, it will be adjusted to the nearest valid value.\nThis entry may not be used with an Indexed color space.')\
          .done().done()\
      .done()

  pdfspec.addClass('ImageDictionary', 'XObjectDictionary', 'Additional entries specific to an image dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be XObject for an image XObject.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of XObject that this dictionary describes; must be\nImage for an image XObject.')\
          .must(datatypes.PdfName('Image'))\
          .done().done()\
      .required('NULL')\
          .field('Width')\
          .name('Width')\
          .type('integer')\
          .comment('(Required) The width of the image, in samples.')\
          .done().done()\
      .required('NULL')\
          .field('Height')\
          .name('Height')\
          .type('integer')\
          .comment('(Required) The height of the image, in samples.')\
          .done().done()\
      .optional()\
          .field('ColorSpace')\
          .name('ColorSpace')\
          .type('name or array')\
          .comment('(Required except for image masks; not allowed for image masks) The color\nspace in which image samples are specified. This may be any type of color\nspace except Pattern.')\
          .done().done()\
      .optional()\
          .field('BitsPerComponent')\
          .name('BitsPerComponent')\
          .type('integer')\
          .comment('(Required except for image masks; optional for image masks) The number of\nbits used to represent each color component. Only a single value may be\nspecified; the number of bits is the same for all color components. Valid\nvalues are 1, 2, 4, and 8. If ImageMask is true, this entry is optional, and if\nspecified, its value must be 1.\nIf the image stream uses a filter, the value of BitsPerComponent must be\nconsistent with the size of the data samples that the filter delivers. In par-\nticular, a CCITTFaxDecode or JBIG2Decode filter always delivers 1-bit sam-\nples, a RunLengthDecode or DCTDecode filter delivers 8-bit samples, and\nan LZWDecode or FlateDecode filter delivers samples of a specified size if\na predictor function is used.')\
          .done().done()\
      .optional()\
          .field('Intent')\
          .name('Intent')\
          .type('name')\
          .comment('(Optional; PDF 1.1) The name of a color rendering intent to be used in\nrendering the image (see "Rendering Intents" on page 197). Default value:\nthe current rendering intent in the graphics state.')\
          .done().done()\
      .optional()\
          .field('ImageMask')\
          .name('ImageMask')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether the image is to be treated as an image\nmask (see Section 4.8.5, "Masked Images"). If this flag is true, the value of\nBitsPerComponent must be 1 and Mask and ColorSpace should not be\nspecified; unmasked areas will be painted using the current nonstroking\ncolor. Default value: false.')\
          .done().done()\
      .optional()\
          .field('Mask')\
          .name('Mask')\
          .type('stream or array')\
          .comment('(Optional except for image masks; not allowed for image masks; PDF 1.3) An\nimage XObject defining an image mask to be applied to this image (see\n"Explicit Masking" on page 277), or an array specifying a range of colors\nto be applied to it as a color key mask (see "Color Key Masking" on page\n277). If ImageMask is true, this entry must not be present. (See\nimplementation note 35 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('SMask')\
          .name('SMask')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A subsidiary image XObject defining a soft-mask\nimage (see "Soft-Mask Images" on page 447) to be used as a source of\nmask shape or mask opacity values in the transparent imaging model. The\nalpha source parameter in the graphics state determines whether the mask\nvalues are interpreted as shape or opacity.\nIf present, this entry overrides the current soft mask in the graphics state,\nas well as the image\'s Mask entry, if any. (However, the other transparency-\nrelated graphics state parameters-blend mode and alpha constant-\nremain in effect.) If SMask is absent, the image has no associated soft mask\n(although the current soft mask in the graphics state may still apply).')\
          .done().done()\
      .optional()\
          .field('Decode')\
          .name('Decode')\
          .type('array')\
          .comment('(Optional) An array of numbers describing how to map image samples\ninto the range of values appropriate for the image\'s color space (see\n"Decode Arrays" on page 271). If ImageMask is true, the array must be\neither [0 1] or [1 0]; otherwise, its length must be twice the number of\ncolor components required by ColorSpace. Default value: see "Decode\nArrays" on page 271.')\
          .done().done()\
      .optional()\
          .field('Interpolate')\
          .name('Interpolate')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether image interpolation is to be per-\nformed (see "Image Interpolation" on page 273). Default value: false.')\
          .done().done()\
      .optional()\
          .field('Alternates')\
          .name('Alternates')\
          .type('array')\
          .comment('(Optional; PDF 1.3) An array of alternate image dictionaries for this image\n(see "Alternate Images" on page 273). The order of elements within the\narray has no significance. This entry may not be present in an image\nXObject that is itself an alternate image.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Required in PDF 1.0; optional otherwise) The name by which this image\nXObject is referenced in the XObject subdictionary of the current resource\ndictionary (see Section 3.7.2, "Resource Dictionaries").\nNote: This entry is obsolescent and its use is no longer recommended. (See\nimplementation note 36 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('StructParent')\
          .name('StructParent')\
          .type('integer')\
          .comment('(Required if the image is a structural content item; PDF 1.3) The integer key\nof the image\'s entry in the structural parent tree (see "Finding Structure\nElements from Content Items" on page 600).')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('string')\
          .comment('(Optional; PDF 1.3; indirect reference preferred) The digital identifier of the\nimage\'s parent Web Capture content set (see Section 9.9.5, "Object At-\ntributes Related to Web Capture").')\
          .done().done()\
      .optional()\
          .field('OPI')\
          .name('OPI')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An OPI version dictionary for the image (see Section\n9.10.6, "Open Prepress Interface (OPI)"). If ImageMask is true, this entry\nis ignored.')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the image\n(see Section 9.2.2, "Metadata Streams").')\
          .done().done()\
      .done()

  pdfspec.addClass('AlternateImageDictionary', 'Dictionary', 'Entries in an alternate image dictionary')\
      .required('NULL')\
          .field('Image')\
          .name('Image')\
          .type('stream')\
          .comment('(Required) The image XObject for the alternate image.')\
          .done().done()\
      .optional()\
          .field('DefaultForPrinting')\
          .name('DefaultForPrinting')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether this alternate image is the default ver-\nsion to be used for printing. At most one alternate for a given base image may\nbe so designated. If no alternate has this entry set to true, the base image itself\nis used for printing.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type1FormDictionary', 'XObjectDictionary', 'Additional entries specific to a type 1 form dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be XObject for a form XObject.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of XObject that this dictionary describes; must be Form\nfor a form XObject.')\
          .must(datatypes.PdfName('Form'))\
          .done().done()\
      .optional()\
          .field('FormType')\
          .name('FormType')\
          .type('integer')\
          .comment('(Optional) A code identifying the type of form XObject that this dictionary\ndescribes. The only valid value defined at the time of publication is 1. Default\nvalue: 1.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Required in PDF 1.0; optional otherwise) The name by which this form\nXObject is referenced in the XObject subdictionary of the current resource\ndictionary (see Section 3.7.2, "Resource Dictionaries").\nNote: This entry is obsolescent and its use is no longer recommended. (See\nimplementation note 38 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('LastModified')\
          .name('LastModified')\
          .type('date')\
          .comment('(Required if PieceInfo is present; optional otherwise; PDF 1.3) The date and\ntime (see Section 3.8.2, "Dates") when the form XObject\'s contents were\nmost recently modified. If a page-piece dictionary (PieceInfo) is present, the\nmodification date is used to ascertain which of the application data diction-\naries it contains correspond to the current content of the form (see Section\n9.4, "Page-Piece Dictionaries").')\
          .done().done()\
      .required('NULL')\
          .field('BBox')\
          .name('BBox')\
          .type('rectangle')\
          .comment('(Required) An array of four numbers in the form coordinate system (see\nbelow), giving the coordinates of the left, bottom, right, and top edges,\nrespectively, of the form XObject\'s bounding box. These boundaries are used\nto clip the form XObject and to determine its size for caching.')\
          .done().done()\
      .optional()\
          .field('Matrix')\
          .name('Matrix')\
          .type('array')\
          .comment('(Optional) An array of six numbers specifying the form matrix, which maps\nform space into user space (see Section 4.2.3, "Transformation Matrices").\nDefault value: the identity matrix [1 0 0 1 0 0].')\
          .done().done()\
      .optional()\
          .field('Resources')\
          .name('Resources')\
          .type('dictionary')\
          .comment('(Optional but strongly recommended; PDF 1.2) A dictionary specifying any\nresources (such as fonts and images) required by the form XObject (see Sec-\ntion 3.7, "Content Streams and Resources").\nIn PDF 1.1 and earlier, all named resources used in the form XObject must be\nincluded in the resource dictionary of each page object on which the form\nXObject appears, whether or not they also appear in the resource dictionary\nof the form XObject itself. It can be useful to specify these resources in the\nform XObject\'s own resource dictionary as well, in order to determine which\nresources are used inside the form XObject. If a resource is included in both\ndictionaries, it should have the same name in both locations.\n     In PDF 1.2 and later versions, form XObjects can be independent of the\n     content streams in which they appear, and this is strongly recommended\n     although not required. In an independent form XObject, the resource dic-\n     tionary of the form XObject is required and contains all named resources\n     used by the form XObject. These resources are not "promoted" to the outer\n     content stream\'s resource dictionary, although that stream\'s resource diction-\n     ary will refer to the form XObject itself.')\
          .done().done()\
      .optional()\
          .field('Group')\
          .name('Group')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A group attributes dictionary indicating that the contents\nof the form XObject are to be treated as a group and specifying the attributes\nof that group (see Section 4.9.2, "Group XObjects").\nNote: If a Ref entry (see below) is present, the group attributes also apply to the\nexternal page imported by that entry. This allows such an imported page to be\ntreated as a group without further modification.')\
          .done().done()\
      .optional()\
          .field('Ref')\
          .name('Ref')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A reference dictionary identifying a page to be imported\nfrom another PDF file, and for which the form XObject serves as a proxy (see\nSection 4.9.3, "Reference XObjects").')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the form\nXObject (see Section 9.2.2, "Metadata Streams").')\
          .done().done()\
      .optional()\
          .field('PieceInfo')\
          .name('PieceInfo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A page-piece dictionary associated with the form\nXObject (see Section 9.4, "Page-Piece Dictionaries").')\
          .done().done()\
      .optional()\
          .field('StructParent')\
          .name('StructParent')\
          .type('integer')\
          .comment('(Required if the form XObject is a structural content item; PDF 1.3) The integer\nkey of the form XObject\'s entry in the structural parent tree (see "Finding\nStructure Elements from Content Items" on page 600).')\
          .done().done()\
      .optional()\
          .field('StructParents')\
          .name('StructParents')\
          .type('integer')\
          .comment('(Required if the form XObject contains marked-content sequences that are struc-\ntural content items; PDF 1.3) The integer key of the form XObject\'s entry in\nthe structural parent tree (see "Finding Structure Elements from Content\nItems" on page 600).\nNote: At most one of the entries StructParent or StructParents may be present. A\nform XObject can be either a content item in its entirety or a container for\nmarked-content sequences that are content items, but not both.')\
          .done().done()\
      .optional()\
          .field('OPI')\
          .name('OPI')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An OPI version dictionary for the form XObject (see\nSection 9.10.6, "Open Prepress Interface (OPI)").')\
          .done().done()\
      .done()

  pdfspec.addClass('GroupAttributesDictionary', 'Dictionary', 'Entries common to all group attributes dictionaries')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must\nbe Group for a group attributes dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The group subtype, which identifies the type of group whose at-\ntributes this dictionary describes and determines the format and meaning of the\ndictionary\'s remaining entries. The only group subtype defined in PDF 1.4 is\nTransparency; see Section 7.5.5, "Transparency Group XObjects," for the re-\nmaining contents of this type of dictionary. Other group subtypes may be added\nin the future.')\
          .done().done()\
      .done()

  pdfspec.addClass('ReferenceDictionary', 'Dictionary', 'Entries in a reference dictionary')\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required) The file containing the target document.')\
          .done().done()\
      .required('NULL')\
          .field('Page')\
          .name('Page')\
          .type('integer or text string')\
          .comment('(Required) A page index or page label (see Section 8.3.1, "Page Labels")\nidentifying the page of the target document containing the content to be\nimported. Note that the reference is a weak one and can be inadvertently in-\nvalidated if the referenced page is changed or replaced in the target document\nafter the reference is created.')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('array')\
          .comment('(Optional) An array of two strings constituting a file identifier (see Section 9.3,\n"File Identifiers") for the file containing the target document. The use of this\nentry improves a viewer application\'s chances of finding the intended file and\nallows it to warn the user if the file has changed since the reference was created.')\
          .done().done()\
      .done()

  pdfspec.addClass('PSXobjectDictionary', 'Dictionary', 'Additional entries specific to a PostScript XObject dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nXObject for a PostScript XObject.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of XObject that this dictionary describes; must be PS for a\nPostScript XObject.')\
          .done().done()\
      .optional()\
          .field('Level1')\
          .name('Level1')\
          .type('stream')\
          .comment('(Optional) A stream whose contents are to be used in place of the PostScript\nXObject\'s stream when the target PostScript interpreter is known to support only\nLanguageLevel 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type1FontDictionary', 'FontDictionary', 'Entries in a Type 1 font dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nFont for a font dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of font; must be Type1 for a Type 1 font.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Required in PDF 1.0; optional otherwise) The name by which this font is ref-\nerenced in the Font subdictionary of the current resource dictionary.\nNote: This entry is obsolescent and its use is no longer recommended. (See\nimplementation note 42 in Appendix H.)')\
          .done().done()\
      .required('NULL')\
          .field('BaseFont')\
          .name('BaseFont')\
          .type('name')\
          .comment('(Required) The PostScript name of the font. For Type 1 fonts, this is usually\nthe value of the FontName entry in the font program; for more information,\nsee Section 5.2 of the PostScript Language Reference, Third Edition. The Post-\nScript name of the font can be used to find the font\'s definition in the viewer\napplication or its environment. It is also the name that will be used when\nprinting to a PostScript output device.')\
          .done().done()\
      .optional()\
          .field('FirstChar')\
          .name('FirstChar')\
          .type('integer')\
          .comment('(Required except for the standard 14 fonts) The first character code defined in\nthe font\'s Widths array.')\
          .done().done()\
      .optional()\
          .field('LastChar')\
          .name('LastChar')\
          .type('integer')\
          .comment('(Required except for the standard 14 fonts) The last character code defined in\nthe font\'s Widths array.')\
          .done().done()\
      .optional()\
          .field('Widths')\
          .name('Widths')\
          .type('array')\
          .comment('(Required except for the standard 14 fonts; indirect reference preferred) An array\nof (LastChar - FirstChar + 1) widths, each element being the glyph width for\nthe character whose code is FirstChar plus the array index. For character\ncodes outside the range FirstChar to LastChar, the value of MissingWidth from\nthe FontDescriptor entry for this font is used. The glyph widths are measured\nin units in which 1000 units corresponds to 1 unit in text space. These widths\nmust be consistent with the actual widths given in the font program itself.\n(See implementation note 43 in Appendix H.) For more information on\nglyph widths and other glyph metrics, see Section 5.1.3, "Glyph Positioning\nand Metrics."')\
          .done().done()\
      .optional()\
          .field('FontDescriptor')\
          .name('FontDescriptor')\
          .type('dictionary')\
          .comment('(Required except for the standard 14 fonts; must be an indirect reference) A font\ndescriptor describing the font\'s metrics other than its glyph widths (see Sec-\ntion 5.7, "Font Descriptors").\n   Note: For the standard 14 fonts, the entries FirstChar, LastChar, Widths, and\n   FontDescriptor must either all be present or all absent. Ordinarily, they are ab-\n   sent; specifying them enables a standard font to be overridden (see "Standard\n   Type 1 Fonts," below).')\
          .done().done()\
      .optional()\
          .field('Encoding')\
          .name('Encoding')\
          .type('name or dictionary')\
          .comment('(Optional) A specification of the font\'s character encoding, if different from\nits built-in encoding. The value of Encoding may be either the name of a pre-\ndefined encoding (MacRomanEncoding, MacExpertEncoding, or WinAnsi-\nEncoding, as described in Appendix D) or an encoding dictionary that\nspecifies differences from the font\'s built-in encoding or from a specified pre-\ndefined encoding (see Section 5.5.5, "Character Encoding").')\
          .done().done()\
      .optional()\
          .field('ToUnicode')\
          .name('ToUnicode')\
          .type('stream')\
          .comment('(Optional; PDF 1.2) A stream containing a CMap file that maps character\ncodes to Unicode values (see Section 5.9, "ToUnicode CMaps").')\
          .done().done()\
      .done()

  pdfspec.addClass('Type3FontDictionary', 'FontDictionary', 'Entries in a Type 3 font dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nFont for a font dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of font; must be Type3 for a Type 3 font.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Required in PDF 1.0; optional otherwise) See Table 5.8 on page 317.')\
          .done().done()\
      .required('NULL')\
          .field('FontBBox')\
          .name('FontBBox')\
          .type('rectangle')\
          .comment('(Required) A rectangle (see Section 3.8.3, "Rectangles"), expressed in the\nglyph coordinate system, specifying the font bounding box. This is the small-\nest rectangle enclosing the shape that would result if all of the glyphs of the\nfont were placed with their origins coincident and then filled.\nIf all four elements of the rectangle are zero, no assumptions are made based\non the font bounding box. If any element is nonzero, it is essential that the\nfont bounding box be accurate; if any glyph\'s marks fall outside this bound-\ning box, incorrect behavior may result.')\
          .done().done()\
      .required('NULL')\
          .field('FontMatrix')\
          .name('FontMatrix')\
          .type('array')\
          .comment('(Required) An array of six numbers specifying the font matrix, mapping\nglyph space to text space (see Section 5.1.3, "Glyph Positioning and\nMetrics"). A common practice is to define glyphs in terms of a 1000-unit')\
          .done().done()\
      .done()

  pdfspec.addClass('EncodingDictionary', 'Dictionary', 'Entries in an encoding dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must\nbe Encoding for an encoding dictionary.')\
          .done().done()\
      .optional()\
          .field('BaseEncoding')\
          .name('BaseEncoding')\
          .type('name')\
          .comment('(Optional) The base encoding-that is, the encoding from which the Differences\nentry (if present) describes differences-specified as the name of a predefined\nencoding MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding (see\nAppendix D).\nIf this entry is absent, the Differences entry describes differences from an im-\nplicit base encoding. For a font program that is embedded in the PDF file, the\nimplicit base encoding is the font program\'s built-in encoding, as described\nabove and further elaborated in the sections on specific font types below. Other-\nwise, for a nonsymbolic font, it is StandardEncoding, and for a symbolic font, it\nis the font\'s built-in encoding.')\
          .done().done()\
      .optional()\
          .field('Differences')\
          .name('Differences')\
          .type('array')\
          .comment('(Optional; not recommended with TrueType fonts) An array describing the differ-\nences from the encoding specified by BaseEncoding or, if BaseEncoding is ab-\nsent, from an implicit base encoding. The Differences array is described above.')\
          .done().done()\
      .done()

  pdfspec.addClass('CIDSystemInfoDictionary', 'Dictionary', 'Entries in a CIDSystemInfo dictionary')\
      .required('NULL')\
          .field('Registry')\
          .name('Registry')\
          .type('string')\
          .comment('(Required) A string identifying the issuer of the character collection-for exam-\nple, Adobe. For information about assigning a registry identifier, consult the ASN\nDeveloper Program Web site or contact the Adobe Solutions Network (see the\nBibliography).')\
          .done().done()\
      .required('NULL')\
          .field('Ordering')\
          .name('Ordering')\
          .type('string')\
          .comment('(Required) A string that uniquely names the character collection within the speci-\nfied registry-for example, Japan1.')\
          .done().done()\
      .required('NULL')\
          .field('Supplement')\
          .name('Supplement')\
          .type('integer')\
          .comment('(Required) The supplement number of the character collection. An original charac-\nter collection has a supplement number of 0. Whenever additional CIDs are\nassigned in a character collection, the supplement number is increased. Supple-\nments do not alter the ordering of existing CIDs in the character collection. This\nvalue is not used in determining compatibility between character collections.')\
          .done().done()\
      .done()

  pdfspec.addClass('CIDFontDictionary', 'FontDictionary', 'Entries in a CIDFont dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nFont for a CIDFont dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of CIDFont; CIDFontType0 or CIDFontType2.')\
          .done().done()\
      .required('NULL')\
          .field('BaseFont')\
          .name('BaseFont')\
          .type('name')\
          .comment('(Required) The PostScript name of the CIDFont. For Type 0 CIDFonts, this\nis usually the value of the CIDFontName entry in the CIDFont program. For\nType 2 CIDFonts, it is derived the same way as for a simple TrueType font;\nsee Section 5.5.2, "TrueType Fonts." In either case, the name can have a sub-\nset prefix if appropriate; see Section 5.5.3, "Font Subsets."')\
          .done().done()\
      .required('NULL')\
          .field('CIDSystemInfo')\
          .name('CIDSystemInfo')\
          .type('dictionary')\
          .comment('(Required) A dictionary containing entries that define the character collec-\ntion of the CIDFont. See Table 5.12 on page 337.')\
          .done().done()\
      .optional()\
          .field('FontDescriptor')\
          .name('FontDescriptor')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) A font descriptor describing the\nCIDFont\'s default metrics other than its glyph widths (see Section 5.7,\n"Font Descriptors").')\
          .done().done()\
      .optional()\
          .field('DW')\
          .name('DW')\
          .type('integer')\
          .comment('(Optional) The default width for glyphs in the CIDFont (see "Glyph Met-\nrics in CIDFonts" on page 340). Default value: 1000.')\
          .done().done()\
      .optional()\
          .field('W')\
          .name('W')\
          .type('array')\
          .comment('(Optional) A description of the widths for the glyphs in the CIDFont. The\narray\'s elements have a variable format that can specify individual widths\nfor consecutive CIDs or one width for a range of CIDs (see "Glyph Metrics\nin CIDFonts" on page 340). Default value: none (the DW value is used for\nall glyphs).')\
          .done().done()\
      .optional()\
          .field('DW2')\
          .name('DW2')\
          .type('array')\
          .comment('(Optional; applies only to CIDFonts used for vertical writing) An array of two\nnumbers specifying the default metrics for vertical writing (see "Glyph\nMetrics in CIDFonts" on page 340). Default value: [880 -1000].')\
          .done().done()\
      .optional()\
          .field('W2')\
          .name('W2')\
          .type('array')\
          .comment('(Optional; applies only to CIDFonts used for vertical writing) A description of\nthe metrics for vertical writing for the glyphs in the CIDFont (see "Glyph\nMetrics in CIDFonts" on page 340). Default value: none (the DW2 value is\nused for all glyphs).')\
          .done().done()\
      .optional()\
          .field('CIDToGIDMap')\
          .name('CIDToGIDMap')\
          .type('stream or name')\
          .comment('(Optional; Type 2 CIDFonts only) A specification of the mapping from CIDs\nto glyph indices. If the value is a stream, the bytes in the stream contain the\nmapping from CIDs to glyph indices: the glyph index for a particular CID\nvalue c is a 2-byte value stored in bytes 2 x c and 2 x c + 1, where the first\nbyte is the high-order byte. If the value of CIDToGIDMap is a name, it must\nbe Identity, indicating that the mapping between CIDs and glyph indices is\nthe identity mapping. Default value: Identity.\nThis entry may appear only in a Type 2 CIDFont whose associated True-\nType font program is embedded in the PDF file (see the next section).')\
          .done().done()\
      .done()

  pdfspec.addClass('CMapDictionary', 'Dictionary', 'Additional entries in a CMap dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nCMap for a CMap dictionary. (Note that although this object is the value of\nan entry named Encoding in a Type 0 font, its type is CMap.)')\
          .done().done()\
      .required('NULL')\
          .field('CMapName')\
          .name('CMapName')\
          .type('name')\
          .comment('(Required) The PostScript name of the CMap. This should be the same as the\nvalue of CMapName in the CMap file itself.')\
          .done().done()\
      .required('NULL')\
          .field('CIDSystemInfo')\
          .name('CIDSystemInfo')\
          .type('dictionary or array')\
          .comment('(Required) A dictionary or array containing entries that define the character\ncollection for the CIDFont or CIDFonts associated with the CMap. If the\nCMap selects only font number 0 and specifies character selectors that are\nCIDs, this entry can be a dictionary identifying the character collection for\nthe associated CIDFont. Otherwise, it is an array indexed by the font num-\nber. If the character selectors for a given font number are CIDs, the corre-\nsponding array element is a dictionary identifying the character collection\nfor the associated CIDFont. If the character selectors are names or codes (to\nbe used with an associated font, not a CIDFont), the array element should\nbe null. For details of the CIDSystemInfo dictionaries, see Section 5.6.2,\n"CIDSystemInfo Dictionaries."\nNote: In all PDF versions up to and including PDF 1.4, CIDSystemInfo must be\neither a dictionary or a one-element array containing a dictionary.\nThe value of this entry should be the same as the value of CIDSystemInfo in\nthe CMap file itself.')\
          .done().done()\
      .optional()\
          .field('WMode')\
          .name('WMode')\
          .type('integer')\
          .comment('(Optional) A code that determines the writing mode for any CIDFont with\nwhich this CMap is combined:\n    0    Horizontal\n    1    Vertical\nDefault value: 0.\nThe value of this entry should be the same as the value of WMode in the\nCMap file itself.')\
          .done().done()\
      .optional()\
          .field('UseCMap')\
          .name('UseCMap')\
          .type('name or stream')\
          .comment('(Optional) The name of a predefined CMap, or a stream containing a CMap,\nthat is to be used as the base for this CMap. This allows the CMap to be de-\nfined differentially, specifying only the character mappings that differ from\nthe base CMap.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type0FontDictionary', 'FontDictionary', 'Entries in a Type 0 font dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nFont for a font dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of font; must be Type0 for a Type 0 font.')\
          .done().done()\
      .required('NULL')\
          .field('BaseFont')\
          .name('BaseFont')\
          .type('name')\
          .comment('(Required) The PostScript name of the font. In principle, this is an arbitrary\nname, since there is no font program associated directly with a Type 0 font\ndictionary. The conventions described here ensure maximum compatibility\nwith existing Acrobat products.\nIf the descendant is a Type 0 CIDFont, this name should be the concatenation\nof the CIDFont\'s BaseFont name, a hyphen, and the CMap name given in the\nEncoding entry (or the CMapName entry in the CMap program itself). If the\ndescendant is a Type 2 CIDFont, this name should be the same as the\nCIDFont\'s BaseFont name.')\
          .done().done()\
      .required('NULL')\
          .field('Encoding')\
          .name('Encoding')\
          .type('name or stream')\
          .comment('(Required) The name of a predefined CMap, or a stream containing a CMap\nprogram, that maps character codes to font numbers and CIDs. If the descen-\ndant is a Type 2 CIDFont whose associated TrueType font program is not em-\nbedded in the PDF file, the Encoding entry must be a predefined CMap name\n(see "Glyph Selection in CIDFonts" on page 339).')\
          .done().done()\
      .required('NULL')\
          .field('DescendantFonts')\
          .name('DescendantFonts')\
          .type('array')\
          .comment('(Required) An array specifying one or more fonts or CIDFonts that are\ndescendants of this composite font. This array is indexed by the font number\nthat is obtained by mapping a character code through the CMap specified in\nthe Encoding entry.\nNote: In all PDF versions up to and including PDF 1.4, DescendantFonts must\nbe a one-element array containing a CIDFont dictionary.')\
          .done().done()\
      .optional()\
          .field('ToUnicode')\
          .name('ToUnicode')\
          .type('stream')\
          .comment('(Optional) A stream containing a CMap file that maps character codes to\nUnicode values (see Section 5.9, "ToUnicode CMaps").')\
          .done().done()\
      .done()

  pdfspec.addClass('FontDescriptorDictionary', 'Dictionary', 'Entries common to all font descriptors')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be\nFontDescriptor for a font descriptor.')\
          .done().done()\
      .required('NULL')\
          .field('FontName')\
          .name('FontName')\
          .type('name')\
          .comment('(Required) The PostScript name of the font. This should be the same as the\nvalue of BaseFont in the font or CIDFont dictionary that refers to this font\ndescriptor.')\
          .done().done()\
      .required('NULL')\
          .field('Flags')\
          .name('Flags')\
          .type('integer')\
          .comment('(Required) A collection of flags defining various characteristics of the font\n(see Section 5.7.1, "Font Descriptor Flags").')\
          .done().done()\
      .required('NULL')\
          .field('FontBBox')\
          .name('FontBBox')\
          .type('rectangle')\
          .comment('(Required) A rectangle (see Section 3.8.3, "Rectangles"), expressed in the\nglyph coordinate system, specifying the font bounding box. This is the small-\nest rectangle enclosing the shape that would result if all of the glyphs of the\nfont were placed with their origins coincident and then filled.')\
          .done().done()\
      .required('NULL')\
          .field('ItalicAngle')\
          .name('ItalicAngle')\
          .type('number')\
          .comment('(Required) The angle, expressed in degrees counterclockwise from the verti-\ncal, of the dominant vertical strokes of the font. (For example, the 9-o\'clock\nposition is 90 degrees, and the 3-o\'clock position is \'90 degrees.) The value is\nnegative for fonts that slope to the right, as almost all italic fonts do.')\
          .done().done()\
      .required('NULL')\
          .field('Ascent')\
          .name('Ascent')\
          .type('number')\
          .comment('(Required) The maximum height above the baseline reached by glyphs in this\nfont, excluding the height of glyphs for accented characters.')\
          .done().done()\
      .required('NULL')\
          .field('Descent')\
          .name('Descent')\
          .type('number')\
          .comment('(Required) The maximum depth below the baseline reached by glyphs in this\nfont. The value is a negative number.')\
          .done().done()\
      .optional()\
          .field('Leading')\
          .name('Leading')\
          .type('number')\
          .comment('(Optional) The desired spacing between baselines of consecutive lines of text.\nDefault value: 0.')\
          .done().done()\
      .required('NULL')\
          .field('CapHeight')\
          .name('CapHeight')\
          .type('number')\
          .comment('(Required) The vertical coordinate of the top of flat capital letters, measured\nfrom the baseline.')\
          .done().done()\
      .optional()\
          .field('XHeight')\
          .name('XHeight')\
          .type('number')\
          .comment('(Optional) The font\'s x height: the vertical coordinate of the top of flat non-\nascending lowercase letters (like the letter x), measured from the baseline.\nDefault value: 0.')\
          .done().done()\
      .required('NULL')\
          .field('StemV')\
          .name('StemV')\
          .type('number')\
          .comment('(Required) The thickness, measured horizontally, of the dominant vertical\nstems of glyphs in the font.')\
          .done().done()\
      .optional()\
          .field('StemH')\
          .name('StemH')\
          .type('number')\
          .comment('(Optional) The thickness, measured invertically, of the dominant horizontal\nstems of glyphs in the font. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('AvgWidth')\
          .name('AvgWidth')\
          .type('number')\
          .comment('(Optional) The average width of glyphs in the font. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('MaxWidth')\
          .name('MaxWidth')\
          .type('number')\
          .comment('(Optional) The maximum width of glyphs in the font. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('MissingWidth')\
          .name('MissingWidth')\
          .type('number')\
          .comment('(Optional) The width to use for character codes whose widths are not speci-\nfied in a font dictionary\'s Widths array. This has a predictable effect only if all\nsuch codes map to glyphs whose actual widths are the same as the Missing-\nWidth value. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('FontFile')\
          .name('FontFile')\
          .type('stream')\
          .comment('(Optional) A stream containing a Type 1 font program (see Section 5.8,\n"Embedded Font Programs").')\
          .done().done()\
      .optional()\
          .field('FontFile2')\
          .name('FontFile2')\
          .type('stream')\
          .comment('(Optional; PDF 1.1) A stream containing a TrueType font program (see Sec-\ntion 5.8, "Embedded Font Programs").')\
          .done().done()\
      .optional()\
          .field('FontFile3')\
          .name('FontFile3')\
          .type('stream')\
          .comment('(Optional; PDF 1.2) A stream containing a font program other than Type 1 or\nTrueType. The format of the font program is specified by the Subtype entry\nin the stream dictionary (see Section 5.8, "Embedded Font Programs," and\nimplementation note 49 in Appendix H).\nAt most, only one of the FontFile, FontFile2, and FontFile3 entries may be\npresent.')\
          .done().done()\
      .optional()\
          .field('CharSet')\
          .name('CharSet')\
          .type('string')\
          .comment('(Optional; meaningful only in Type 1 fonts; PDF 1.1) A string listing the char-\nacter names defined in a font subset. The names in this string must be in PDF\nsyntax-that is, each name preceded by a slash (/). The names can appear in\nany order. The name .notdef should be omitted; it is assumed to exist in the\nfont subset. If this entry is absent, the only indication of a font subset is the\nsubset tag in the FontName entry (see Section 5.5.3, "Font Subsets").')\
          .done().done()\
      .done()

  pdfspec.addClass('CIDFontDescriptorDictionary', 'Dictionary', 'Additional font descriptor entries for CIDFonts')\
      .optional()\
          .field('Style')\
          .name('Style')\
          .type('dictionary')\
          .comment('(Optional) A dictionary containing entries that describe the style of the glyphs in\nthe font (see "Style," above).')\
          .done().done()\
      .optional()\
          .field('Lang')\
          .name('Lang')\
          .type('name')\
          .comment('(Optional) A name specifying the language of the font, used for encodings where\nthe language is not implied by the encoding itself. The possible values are the\n2-character language codes defined by ISO 639-for example, en for English and ja\nfor Japanese. The complete list of these codes be obtained from the International\nOrganization for Standardization (see the Bibliography).')\
          .done().done()\
      .optional()\
          .field('FD')\
          .name('FD')\
          .type('dictionary')\
          .comment('(Optional) A dictionary whose keys identify a class of characters in a CIDFont.\nEach value is a dictionary containing entries that override the corresponding\nvalues in the main font descriptor dictionary for that class of characters (see "FD,"\nbelow).')\
          .done().done()\
      .optional()\
          .field('CIDSet')\
          .name('CIDSet')\
          .type('stream')\
          .comment('(Optional) A stream identifying which CIDs are present in the CIDFont file. If this\nentry is present, the CIDFont contains only a subset of the glyphs in the character\ncollection defined by the CIDSystemInfo dictionary. If it is absent, the only indica-\ntion of a CIDFont subset is the subset tag in the FontName entry (see Section 5.5.3,\n"Font Subsets").\nThe stream\'s data is organized as a table of bits indexed by CID. The bits should be\nstored in bytes with the high-order bit first. Each bit corresponds to a CID. The first\nbit of the first byte corresponds to CID 0, the next bit to CID 1, and so on.')\
          .done().done()\
      .done()

  pdfspec.addClass('EmbeddedFontStreamDictionary', 'Dictionary', 'Additional entries in an embedded font stream dictionary')\
      .optional()\
          .field('Length1')\
          .name('Length1')\
          .type('integer')\
          .comment('(Required for Type 1 and TrueType fonts) The length in bytes of the clear-text portion\nof the Type 1 font program (see below), or the entire TrueType font program, after it\nhas been decoded using the filters specified by the stream\'s Filter entry, if any.')\
          .done().done()\
      .optional()\
          .field('Length2')\
          .name('Length2')\
          .type('integer')\
          .comment('(Required for Type 1 fonts) The length in bytes of the encrypted portion of the Type 1\nfont program (see below) after it has been decoded using the filters specified by the\nstream\'s Filter entry.')\
          .done().done()\
      .optional()\
          .field('Length3')\
          .name('Length3')\
          .type('integer')\
          .comment('(Required for Type 1 fonts) The length in bytes of the fixed-content portion of the\nType 1 font program (see below), after it has been decoded using the filters specified\nby the stream\'s Filter entry. If Length3 is 0, it indicates that the 512 zeros and clearto-\nmark have not been included in the FontFile font program and must be added.')\
          .done().done()\
      .optional()\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required if referenced from FontFile3; PDF 1.2) A name specifying the format of the\nembedded font program. The name must be Type1C for Type 1 compact fonts or CID-\nFontType0C for Type 0 compact CIDFonts. When additional font formats are added\nto PDF, more values will be defined for Subtype.')\
          .done().done()\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the embedded font\nprogram (see Section 9.2.2, "Metadata Streams").')\
          .done().done()\
      .done()

  pdfspec.addClass('Type1HalftoneDictionary', 'Dictionary', 'Entries in a type 1 halftone dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be Halftone for a halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('HalftoneType')\
          .name('HalftoneType')\
          .type('integer')\
          .comment('(Required) A code identifying the halftone type that this dictionary\ndescribes; must be 1 for this type of halftone.')\
          .done().done()\
      .optional()\
          .field('HalftoneName')\
          .name('HalftoneName')\
          .type('string')\
          .comment('(Optional) The name of the halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Frequency')\
          .name('Frequency')\
          .type('number')\
          .comment('(Required) The screen frequency, measured in halftone cells per inch in\ndevice space.')\
          .done().done()\
      .required('NULL')\
          .field('Angle')\
          .name('Angle')\
          .type('number')\
          .comment('(Required) The screen angle, in degrees of rotation counterclockwise\nwith respect to the device coordinate system. (Note that most output\ndevices have left-handed device spaces; on such devices, a counter-\nclockwise angle in device space will correspond to a clockwise angle in\ndefault user space and on the physical medium.)')\
          .done().done()\
      .required('NULL')\
          .field('SpotFunction')\
          .name('SpotFunction')\
          .type('function or name')\
          .comment('(Required) A function object defining the order in which device pixels\nwithin a screen cell are adjusted for different gray levels, or the name of\none of the predefined spot functions (see Table 6.1 on page 385).')\
          .done().done()\
      .optional()\
          .field('AccurateScreens')\
          .name('AccurateScreens')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to invoke a special halftone al-\ngorithm that is extremely precise, but computationally expensive; see\nbelow for further discussion. Default value: false.')\
          .done().done()\
      .optional()\
          .field('TransferFunction')\
          .name('TransferFunction')\
          .type('function or name')\
          .comment('(Optional) A transfer function, which overrides the current transfer\nfunction in the graphics state for the same component. This entry is\nrequired if the dictionary is a component of a type 5 halftone (see\n"Type 5 Halftones" on page 400) and represents either a nonprimary\nor nonstandard primary color component (see Section 6.3, "Transfer\nFunctions"). The name Identity may be used to specify the identity\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type6HalftoneDictionary', 'Dictionary', 'Additional entries specific to a type 6 halftone dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be Halftone for a halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('HalftoneType')\
          .name('HalftoneType')\
          .type('integer')\
          .comment('(Required) A code identifying the halftone type that this dictionary\ndescribes; must be 6 for this type of halftone.')\
          .done().done()\
      .optional()\
          .field('HalftoneName')\
          .name('HalftoneName')\
          .type('string')\
          .comment('(Optional) The name of the halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Width')\
          .name('Width')\
          .type('integer')\
          .comment('(Required) The width of the threshold array, in device pixels.')\
          .done().done()\
      .required('NULL')\
          .field('Height')\
          .name('Height')\
          .type('integer')\
          .comment('(Required) The height of the threshold array, in device pixels.')\
          .done().done()\
      .optional()\
          .field('TransferFunction')\
          .name('TransferFunction')\
          .type('function or name')\
          .comment('(Optional) A transfer function, which overrides the current transfer\nfunction in the graphics state for the same component. This entry is\nrequired if the dictionary is a component of a type 5 halftone (see\n"Type 5 Halftones" on page 400) and represents either a nonprimary\nor nonstandard primary color component (see Section 6.3, "Transfer\nFunctions"). The name Identity may be used to specify the identity\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type10HalftoneDictionary', 'Dictionary', 'Additional entries specific to a type 10 halftone dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be Halftone for a halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('HalftoneType')\
          .name('HalftoneType')\
          .type('integer')\
          .comment('(Required) A code identifying the halftone type that this dictionary\ndescribes; must be 10 for this type of halftone.')\
          .done().done()\
      .optional()\
          .field('HalftoneName')\
          .name('HalftoneName')\
          .type('string')\
          .comment('(Optional) The name of the halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Xsquare')\
          .name('Xsquare')\
          .type('integer')\
          .comment('(Required) The side of square X, in device pixels; see below.')\
          .done().done()\
      .required('NULL')\
          .field('Ysquare')\
          .name('Ysquare')\
          .type('integer')\
          .comment('(Required) The side of square Y, in device pixels; see below.')\
          .done().done()\
      .optional()\
          .field('TransferFunction')\
          .name('TransferFunction')\
          .type('function or name')\
          .comment('(Optional) A transfer function, which overrides the current transfer\nfunction in the graphics state for the same component. This entry is\nrequired if the dictionary is a component of a type 5 halftone (see\n"Type 5 Halftones" on page 400) and represents either a nonprimary\nor nonstandard primary color component (see Section 6.3, "Transfer\nFunctions"). The name Identity may be used to specify the identity\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type16HalftoneDictionary', 'Dictionary', 'Additional entries specific to a type 16 halftone dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be Halftone for a halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('HalftoneType')\
          .name('HalftoneType')\
          .type('integer')\
          .comment('(Required) A code identifying the halftone type that this dictionary\ndescribes; must be 16 for this type of halftone.')\
          .done().done()\
      .optional()\
          .field('HalftoneName')\
          .name('HalftoneName')\
          .type('string')\
          .comment('(Optional) The name of the halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Width')\
          .name('Width')\
          .type('integer')\
          .comment('(Required) The width of the first (or only) rectangle in the threshold\narray, in device pixels.')\
          .done().done()\
      .required('NULL')\
          .field('Height')\
          .name('Height')\
          .type('integer')\
          .comment('(Required) The height of the first (or only) rectangle in the threshold\narray, in device pixels.')\
          .done().done()\
      .optional()\
          .field('Width2')\
          .name('Width2')\
          .type('integer')\
          .comment('(Optional) The width of the optional second rectangle in the threshold\narray, in device pixels. If this entry is present, the Height2 entry must\nbe present as well; if this entry is absent, the Height2 entry must also be\nabsent and the threshold array has only one rectangle.')\
          .done().done()\
      .optional()\
          .field('Height2')\
          .name('Height2')\
          .type('integer')\
          .comment('(Optional) The height of the optional second rectangle in the threshold\narray, in device pixels.')\
          .done().done()\
      .optional()\
          .field('TransferFunction')\
          .name('TransferFunction')\
          .type('function or name')\
          .comment('(Optional) A transfer function, which overrides the current transfer\nfunction in the graphics state for the same component. This entry is\nrequired if the dictionary is a component of a type 5 halftone (see\n"Type 5 Halftones," below) and represents either a nonprimary or\nnonstandard primary color component (see Section 6.3, "Transfer\nFunctions"). The name Identity may be used to specify the identity\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('Type5HalftoneDictionary', 'Dictionary', 'Entries in a type 5 halftone dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Halftone for a halftone dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('HalftoneType')\
          .name('HalftoneType')\
          .type('number')\
          .comment('(Required) A code identifying the halftone type that this dictionary describes;\nmust be 5 for this type of halftone.')\
          .done().done()\
      .optional()\
          .field('HalftoneName')\
          .name('HalftoneName')\
          .type('string')\
          .comment('(Optional) The name of the halftone dictionary.')\
          .done().done()\
      .optional()\
          .field('[any_colorant_name]')\
          .name('[any_colorant_name]')\
          .type('dictionary or stream')\
          .comment('(Required, one per colorant) The halftone corresponding to the colorant or\ncolor component named by the key. The halftone may be of any type other\nthan 5. Note that the key must be a name object; strings are not permitted, as\nthey are in type 5 PostScript halftone dictionaries.')\
          .done().done()\
      .required('NULL')\
          .field('Default')\
          .name('Default')\
          .type('dictionary or stream')\
          .comment('(Required) A halftone to be used for any colorant or color component that\ndoes not have an entry of its own. The value may not be a type 5 halftone. If\nthere are any nonprimary colorants, the default halftone must have a transfer\nfunction.')\
          .done().done()\
      .done()

  pdfspec.addClass('SoftMaskDictionary', 'Dictionary', 'Entries in a soft-mask dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Mask for a soft-mask dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) A subtype specifying the method to be used in deriving the mask\nvalues from the transparency group specified by the G entry:\n   Alpha          Use the group\'s computed alpha, disregarding its color (see\n                  Section 7.4.1, "Deriving a Soft Mask from Group Alpha").\n   Luminosity     Convert the group\'s computed color to a single-component\n                  luminosity value (see Section 7.4.2, "Deriving a Soft Mask\n                  from Group Luminosity").')\
          .done().done()\
      .required('NULL')\
          .field('G')\
          .name('G')\
          .type('stream')\
          .comment('(Required) A transparency group XObject (see Section 7.5.5, "Transparency\nGroup XObjects") to be used as the source of alpha or color values for deriv-\ning the mask. If the subtype S is Luminosity, the group attributes dictionary\nmust contain a CS entry defining the color space in which the compositing\ncomputation is to be performed.')\
          .done().done()\
      .optional()\
          .field('BC')\
          .name('BC')\
          .type('array')\
          .comment('(Optional) An array of component values specifying the color to be used as\nthe backdrop against which to composite the transparency group XObject G.\nThis entry is consulted only if the subtype S is Luminosity. The array consists\nof n numbers, where n is the number of components in the color space speci-\nfied by the CS entry in the group attributes dictionary (see Section 7.5.5,\n"Transparency Group XObjects"). Default value: the color space\'s initial\nvalue, representing black.')\
          .done().done()\
      .optional()\
          .field('TR')\
          .name('TR')\
          .type('function or name')\
          .comment('(Optional) A function object (see Section 3.9, "Functions") specifying the\ntransfer function to be used in deriving the mask values. The function ac-\ncepts one input, the computed group alpha or luminosity (depending on the\nvalue of the subtype S), and returns one output, the resulting mask value.\nBoth the input and output must be in the range 0.0 to 1.0; if the computed\noutput falls outside this range, it is forced to the nearest valid value. The\nname Identity may be specified in place of a function object to designate the\nidentity function. Default value: Identity.')\
          .done().done()\
      .done()

  pdfspec.addClass('SoftMaskImageDictionary', 'Dictionary', 'Additional entry in a soft-mask image dictionary')\
      .optional()\
          .field('Matte')\
          .name('Matte')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of component values specifying the matte color with\nwhich the image data in the parent image has been preblended. The array consists of n\nnumbers, where n is the number of components in the color space specified by the\nColorSpace entry in the parent image\'s image dictionary; the numbers must be valid\ncolor components in that color space. If this entry is absent, the image data is not pre-\nblended.')\
          .done().done()\
      .done()

  pdfspec.addClass('TransparencyGroupDictionary', 'Dictionary', 'Additional entries specific to a transparency group attributes dictionary')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The group subtype, which identifies the type of group whose at-\ntributes this dictionary describes; must be Transparency for a transparency\ngroup.')\
          .done().done()\
      .optional()\
          .field('CS')\
          .name('CS')\
          .type('name or array')\
          .comment('(Sometimes required, as discussed below) The group color space, which is used for\nthe following purposes:\n*  As the color space into which colors are converted when painted into the\n   group\n*  As the blending color space in which objects are composited within the group\n   (see Section 7.2.3, "Blending Color Space")\n*  As the color space of the group as a whole when it in turn is painted as an ob-\n   ject onto its backdrop\nThe group color space may be any device or CIE-based color space that treats its\ncomponents as independent additive or subtractive values in the range 0.0 to\n1.0, subject to the restrictions described in Section 7.2.3, "Blending Color\nSpace." These restrictions exclude Lab and lightness-chromaticity ICCBased\ncolor spaces, as well as the special color spaces Pattern, Indexed, Separation, and\nDeviceN. Device color spaces are subject to remapping according to the Default-\nGray, DefaultRGB, and DefaultCMYK entries in the ColorSpace subdictionary of\nthe current resource dictionary (see "Default Color Spaces" on page 194).\nOrdinarily, the CS entry is allowed only for isolated transparency groups (those\nfor which I, below, is true) and even then it is optional. However, this entry is re-\nquired in the group attributes dictionary for any transparency group XObject\nthat has no parent group or page from which to inherit-in particular, one that\nis the value of the G entry in a soft-mask dictionary of subtype Luminosity (see\n"Soft-Mask Dictionaries" on page 445).\nIn addition, it is always permissible to specify CS in the group attributes diction-\nary associated with a page object, even if I is false or absent. In the normal case in\nwhich the page is imposed directly on the output medium, the page group is\neffectively isolated regardless of the I value, and the specified CS value is there-\nfore honored. But if the page is in turn used as an element of some other page\nand if the group is non-isolated, CS is ignored and the color space is inherited\nfrom the actual backdrop with which the page is composited (see Section 7.3.6,\n"Page Group").')\
          .done().done()\
      .done()

  pdfspec.addClass('ViewerPreferencesDictionary', 'Dictionary', 'Entries in a viewer preferences dictionary')\
      .optional()\
          .field('HideToolbar')\
          .name('HideToolbar')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to hide the viewer application\'s tool\nbars when the document is active. Default value: false.')\
          .done().done()\
      .optional()\
          .field('HideMenubar')\
          .name('HideMenubar')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to hide the viewer application\'s\nmenu bar when the document is active. Default value: false.')\
          .done().done()\
      .optional()\
          .field('HideWindowUI')\
          .name('HideWindowUI')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to hide user interface elements in\nthe document\'s window (such as scroll bars and navigation controls),\nleaving only the document\'s contents displayed. Default value: false.')\
          .done().done()\
      .optional()\
          .field('FitWindow')\
          .name('FitWindow')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to resize the document\'s window to\nfit the size of the first displayed page. Default value: false.')\
          .done().done()\
      .optional()\
          .field('CenterWindow')\
          .name('CenterWindow')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to position the document\'s window\nin the center of the screen. Default value: false.')\
          .done().done()\
      .optional()\
          .field('DisplayDocTitle')\
          .name('DisplayDocTitle')\
          .type('boolean')\
          .comment('(Optional; PDF 1.4) A flag specifying whether the window\'s title bar\nshould display the document title taken from the Title entry of the docu-\nment information dictionary (see Section 9.2.1, "Document Informa-\ntion Dictionary"). If false, the title bar should instead display the name\nof the PDF file containing the document. Default value: false.')\
          .done().done()\
      .optional()\
          .field('NonFullScreenPageMode')\
          .name('NonFullScreenPageMode')\
          .type('name')\
          .comment('(Optional) The document\'s page mode, specifying how to display the\ndocument on exiting full-screen mode:\n     UseNone            Neither document outline nor thumbnail images\n                        visible\n     UseOutlines        Document outline visible\n     UseThumbs          Thumbnail images visible\nThis entry is meaningful only if the value of the PageMode entry in the\ncatalog dictionary (see Section 3.6.1, "Document Catalog") is FullScreen;\nit is ignored otherwise. Default value: UseNone.')\
          .done().done()\
      .optional()\
          .field('Direction')\
          .name('Direction')\
          .type('name')\
          .comment('(Optional; PDF 1.3) The predominant reading order for text:\n     L2R                Left to right\n     R2L                Right to left (including vertical writing systems\n                        such as Chinese, Japanese, and Korean)\nThis entry has no direct effect on the document\'s contents or page num-\nbering, but can be used to determine the relative positioning of pages\nwhen displayed side by side or printed n-up. Default value: L2R.')\
          .done().done()\
      .optional()\
          .field('ViewArea')\
          .name('ViewArea')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The name of the page boundary representing the\narea of a page to be displayed when viewing the document on the screen.\nThe value is the key designating the relevant page boundary in the page\nobject (see "Page Objects" on page 87 and Section 9.10.1, "Page Bound-\naries"). If the specified page boundary is not defined in the page object,\nits default value will be used, as specified in Table 3.18 on page 88.\nDefault value: CropBox.\nNote: This entry is intended primarily for use by prepress applications that\ninterpret or manipulate the page boundaries as described in Section 9.10.1,\n"Page Boundaries." Most PDF consumer applications will disregard it.')\
          .done().done()\
      .optional()\
          .field('ViewClip')\
          .name('ViewClip')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The name of the page boundary to which the con-\ntents of a page are to be clipped when viewing the document on the\nscreen. The value is the key designating the relevant page boundary in\nthe page object (see "Page Objects" on page 87 and Section 9.10.1, "Page\nBoundaries"). If the specified page boundary is not defined in the page\nobject, its default value will be used, as specified in Table 3.18 on page\n88. Default value: CropBox.\nNote: This entry is intended primarily for use by prepress applications that\ninterpret or manipulate the page boundaries as described in Section 9.10.1,\n"Page Boundaries." Most PDF consumer applications will disregard it.')\
          .done().done()\
      .optional()\
          .field('PrintArea')\
          .name('PrintArea')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The name of the page boundary representing the\narea of a page to be rendered when printing the document. The value is\nthe key designating the relevant page boundary in the page object (see\n"Page Objects" on page 87 and Section 9.10.1, "Page Boundaries"). If the\nspecified page boundary is not defined in the page object, its default value\nwill be used, as specified in Table 3.18 on page 88. Default value: CropBox.\nNote: This entry is intended primarily for use by prepress applications that\ninterpret or manipulate the page boundaries as described in Section 9.10.1,\n"Page Boundaries." Most PDF consumer applications will disregard it.')\
          .done().done()\
      .optional()\
          .field('PrintClip')\
          .name('PrintClip')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The name of the page boundary to which the con-\ntents of a page are to be clipped when printing the document. The value\nis the key designating the relevant page boundary in the page object (see\n"Page Objects" on page 87 and Section 9.10.1, "Page Boundaries"). If the\nspecified page boundary is not defined in the page object, its default value\nwill be used, as specified in Table 3.18 on page 88. Default value: CropBox.\nNote: This entry is intended primarily for use by prepress applications that\ninterpret or manipulate the page boundaries as described in Section 9.10.1,\n"Page Boundaries." Most PDF consumer applications will disregard it.')\
          .done().done()\
      .done()

  pdfspec.addClass('OutlineDictionary', 'Dictionary', 'Entries in the outline dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Outlines for an outline dictionary.')\
          .done().done()\
      .optional()\
          .field('First')\
          .name('First')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) An outline item dictionary represent-\ning the first top-level item in the outline.')\
          .done().done()\
      .optional()\
          .field('Last')\
          .name('Last')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) An outline item dictionary represent-\ning the last top-level item in the outline.')\
          .done().done()\
      .optional()\
          .field('Count')\
          .name('Count')\
          .type('integer')\
          .comment('(Required if the document has any open outline entries) The total number of\nopen items at all levels of the outline. This entry should be omitted if there\nare no open outline items.')\
          .done().done()\
      .done()

  pdfspec.addClass('OutlineItemDictionary', 'Dictionary', 'Entries in an outline item dictionary')\
      .required('NULL')\
          .field('Title')\
          .name('Title')\
          .type('text string')\
          .comment('(Required) The text to be displayed on the screen for this item.')\
          .done().done()\
      .optional()\
          .field('Parent')\
          .name('Parent')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The parent of this item in the outline\nhierarchy. The parent of a top-level item is the outline dictionary itself.')\
          .done().done()\
      .optional()\
          .field('Prev')\
          .name('Prev')\
          .type('dictionary')\
          .comment('(Required for all but the first item at each level; must be an indirect reference)\nThe previous item at this outline level.')\
          .done().done()\
      .optional()\
          .field('Next')\
          .name('Next')\
          .type('dictionary')\
          .comment('(Required for all but the last item at each level; must be an indirect reference)\nThe next item at this outline level.')\
          .done().done()\
      .optional()\
          .field('First')\
          .name('First')\
          .type('dictionary')\
          .comment('(Required if the item has any descendants; must be an indirect reference) The\nfirst of this item\'s immediate children in the outline hierarchy.')\
          .done().done()\
      .optional()\
          .field('Last')\
          .name('Last')\
          .type('dictionary')\
          .comment('(Required if the item has any descendants; must be an indirect reference) The\nlast of this item\'s immediate children in the outline hierarchy.')\
          .done().done()\
      .optional()\
          .field('Count')\
          .name('Count')\
          .type('integer')\
          .comment('(Required if the item has any descendants) If the item is open, the total num-\nber of its open descendants at all lower levels of the outline hierarchy. If the\nitem is closed, a negative integer whose absolute value specifies how many\ndescendants would appear if the item were reopened.')\
          .done().done()\
      .optional()\
          .field('Dest')\
          .name('Dest')\
          .type('name, string, or array')\
          .comment('(Optional; not permitted if an A entry is present) The destination to be\ndisplayed when this item is activated (see Section 8.2.1, "Destinations"; see\nalso implementation note 56 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('A')\
          .name('A')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.1; not permitted if a Dest entry is present) The action to be\nperformed when this item is activated (see Section 8.5, "Actions").')\
          .done().done()\
      .optional()\
          .field('SE')\
          .name('SE')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3; must be an indirect reference) The structure element to\nwhich the item refers (see Section 9.6.1, "Structure Hierarchy").\nNote: The ability to associate an outline item with a structure element (such as\nthe beginning of a chapter) is a PDF 1.3 feature. For backward compatibility\nwith earlier PDF versions, such an item should also specify a destination (Dest)\ncorresponding to an area of a page where the contents of the designated structure\nelement are displayed.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of three numbers in the range 0.0 to 1.0, repre-\nsenting the components in the DeviceRGB color space of the color to be used\nfor the outline entry\'s text. Default value: [0.0 0.0 0.0].')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('integer')\
          .comment('(Optional; PDF 1.4) A set of flags specifying style characteristics for display-\ning the outline item\'s text (see Table 8.5). Default value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('PageLabelDictionary', 'Dictionary', 'Entries in a page label dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nPageLabel for a page label dictionary.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Optional) The numbering style to be used for the numeric portion of each page label:\n   D     Decimal arabic numerals\n   R     Uppercase roman numerals\n   r     Lowercase roman numerals\n   A     Uppercase letters (A to Z for the first 26 pages, AA to ZZ for the next 26, and so on)\n   a     Lowercase letters (a to z for the first 26 pages, aa to zz for the next 26, and so on)\nThere is no default numbering style; if no S entry is present, page labels will consist solely\nof a label prefix with no numeric portion. For example, if the P entry (below) specifies the\nlabel prefix Contents, each page will simply be labeled Contents with no page number. (If\nthe P entry is also missing or empty, the page label will be an empty string.)')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('text string')\
          .comment('(Optional) The label prefix for page labels in this range.')\
          .done().done()\
      .optional()\
          .field('St')\
          .name('St')\
          .type('integer')\
          .comment('(Optional) The value of the numeric portion for the first page label in the range. Sub-\nsequent pages will be numbered sequentially from this value, which must be greater than\nor equal to 1. Default value: 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('ThreadDictionary', 'Dictionary', 'Entries in a thread dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nThread for a thread dictionary.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The first bead in the thread.')\
          .done().done()\
      .optional()\
          .field('I')\
          .name('I')\
          .type('dictionary')\
          .comment('(Optional) A thread information dictionary containing information about the\nthread, such as its title, author, and creation date. The contents of this dictionary are\nsimilar to those of the document information dictionary (see Section 9.2.1, "Docu-\nment Information Dictionary").')\
          .done().done()\
      .done()

  pdfspec.addClass('BeadDictionary', 'Dictionary', 'Entries in a bead dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nBead for a bead dictionary.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('dictionary')\
          .comment('(Required for the first bead of a thread; optional for all others; must be an indirect refer-\nence) The thread to which this bead belongs.\nNote: In PDF 1.1, this entry is permitted only for the first bead of a thread. In PDF 1.2\nand higher, it is permitted for any bead but required only for the first.')\
          .done().done()\
      .optional()\
          .field('N')\
          .name('N')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The next bead in the thread. In the last bead,\nthis entry points to the first.')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The previous bead in the thread. In the first\nbead, this entry points to the last.')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The page object representing the page on\nwhich this bead appears.')\
          .done().done()\
      .required('NULL')\
          .field('R')\
          .name('R')\
          .type('rectangle')\
          .comment('(Required) A rectangle specifying the location of this bead on the page.')\
          .done().done()\
      .done()

  pdfspec.addClass('TransitionDictionary', 'Dictionary', 'Entries in a transition dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nTrans for a transition dictionary.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('number')\
          .comment('(Optional) The duration of the transition effect, in seconds. Default value: 1.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Optional) The transition style to use when moving to this page from another during a\npresentation:\n   Split       Two lines sweep across the screen, revealing the new page. The lines may\n               be either horizontal or vertical and may move inward from the edges of\n               the page or outward from the center, as specified by the Dm and M\n               entries, respectively.\n   Blinds      Multiple lines, evenly spaced across the screen, synchronously sweep in\n               the same direction to reveal the new page. The lines may be either hori-\n               zontal or vertical, as specified by the Dm entry. Horizontal lines move\n               downward, vertical lines to the right.\n   Box         A rectangular box sweeps inward from the edges of the page or outward\n               from the center, as specified by the M entry, revealing the new page.\n   Wipe        A single line sweeps across the screen from one edge to the other in the\n               direction specified by the Di entry, revealing the new page.\n   Dissolve    The old page "dissolves" gradually to reveal the new one.\n   Glitter     Similar to Dissolve, except that the effect sweeps across the page in a\n               wide band moving from one side of the screen to the other in the direc-\n               tion specified by the Di entry.\n   R           The new page simply replaces the old one with no special transition ef-\n               fect; the D entry is ignored.\nDefault value: R.')\
          .done().done()\
      .optional()\
          .field('Dm')\
          .name('Dm')\
          .type('name')\
          .comment('(Optional; Split and Blinds transition styles only) The dimension in which the specified\ntransition effect occurs:\n     H         Horizontal\n     V         Vertical\nDefault value: H.')\
          .done().done()\
      .optional()\
          .field('M')\
          .name('M')\
          .type('name')\
          .comment('(Optional; Split and Box transition styles only) The direction of motion for the specified\ntransition effect:\n     I         Inward from the edges of the page\n     O         Outward from the center of the page\nDefault value: I.')\
          .done().done()\
      .optional()\
          .field('Di')\
          .name('Di')\
          .type('number')\
          .comment('(Optional; Wipe and Glitter transition styles only) The direction in which the specified\ntransition effect moves, expressed in degrees counterclockwise starting from a left-to-\nright direction. (Note that this differs from the page object\'s Rotate entry, which is\nmeasured clockwise from the top.) Only the following values are valid:\n        0      Left to right\n       90      Bottom to top (Wipe only)\n     180       Right to left (Wipe only)\n     270       Top to bottom\n     315       Top-left to bottom-right (Glitter only)\nDefault value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('AnnotationDictionary', 'Dictionary', 'Entries common to all annotation dictionaries')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Annot for an annotation dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; see Table\n8.14 on page 499 for specific values.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required or optional, depending on the annotation type) Text to be displayed\nfor the annotation or, if this type of annotation does not display text, an al-\nternate description of the annotation\'s contents in human-readable form. In\neither case, this text is useful when extracting the document\'s contents in\nsupport of accessibility to disabled users or for other purposes (see Section\n9.8.2, "Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3; not used in FDF files) An indirect reference to the page\nobject with which this annotation is associated.')\
          .done().done()\
      .required('NULL')\
          .field('Rect')\
          .name('Rect')\
          .type('rectangle')\
          .comment('(Required) The annotation rectangle, defining the location of the annotation\non the page in default user space units.')\
          .done().done()\
      .optional()\
          .field('NM')\
          .name('NM')\
          .type('text')\
          .comment('(Optional; PDF 1.4) The annotation name, a text string uniquely identifying\nit among all the annotations on its page.')\
          .done().done()\
      .optional()\
          .field('M')\
          .name('M')\
          .type('date or string')\
          .comment('(Optional; PDF 1.1) The date and time when the annotation was most\nrecently modified. The preferred format is a date string as described in Sec-\ntion 3.8.2, "Dates," but viewer applications should be prepared to accept and\ndisplay a string in any format. (See implementation note 59 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('integer')\
          .comment('(Optional; PDF 1.1) A set of flags specifying various characteristics of the an-\nnotation (see Section 8.4.2, "Annotation Flags"). Default value: 0.')\
          .done().done()\
      .optional()\
          .field('BS')\
          .name('BS')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) A border style dictionary specifying the characteristics of\nthe annotation\'s border (see Section 8.4.3, "Border Styles"; see also imple-\nmentation note 60 in Appendix H).\nNote: This entry also specifies the width and dash pattern for the lines drawn by\nline, square, circle, and ink annotations. See the note under Border (below) for\nadditional information.')\
          .done().done()\
      .optional()\
          .field('Border')\
          .name('Border')\
          .type('array')\
          .comment('(Optional) An array specifying the characteristics of the annotation\'s border.\nThe border is specified as a "rounded rectangle."\nIn PDF 1.0, the array consists of three numbers defining the horizontal cor-\nner radius, vertical corner radius, and border width, all in default user space\nunits. If the corner radii are 0, the border has square (not rounded) corners;\nif the border width is 0, no border is drawn. (See implementation note 61 in\nAppendix H.)\n  In PDF 1.1, the array may have a fourth element, an optional dash array\n  defining a pattern of dashes and gaps to be used in drawing the border. The\n  dash array is specified in the same format as in the line dash pattern parame-\n  ter of the graphics state (see "Line Dash Pattern" on page 155). For example,\n  a Border value of [0 0 1 [3 2]] specifies a border 1 unit wide, with square\n  corners, drawn with 3-unit dashes alternating with 2-unit gaps. Note that no\n  dash phase is specified; the phase is assumed to be 0. (See implementation\n  note 62 in Appendix H.)\n  Note: In PDF 1.2 or later, annotations may ignore this entry and use the BS\n  entry (see above) to specify their border styles instead. In PDF 1.2 and 1.3, only\n  widget annotations do so; in PDF 1.4, all of the standard annotation types ex-\n  cept Link (see Table 8.14 on page 499) use BS rather than Border if both are\n  present. For backward compatibility, however, Border is still supported for all\n  annotation types.\n  Default value: [0 0 1].')\
          .done().done()\
      .optional()\
          .field('AP')\
          .name('AP')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An appearance dictionary specifying how the annotation\nis presented visually on the page (see Section 8.4.4, "Appearance Streams";\nsee also implementation note 60 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('AS')\
          .name('AS')\
          .type('name')\
          .comment('(Required if the appearance dictionary AP contains one or more subdictionaries;\nPDF 1.2) The annotation\'s appearance state, which selects the applicable\nappearance stream from an appearance subdictionary (see Section 8.4.4,\n"Appearance Streams"; see also implementation note 60 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('array')\
          .comment('(Optional; PDF 1.1) An array of three numbers in the range 0.0 to 1.0, repre-\nsenting the components of a color in the DeviceRGB color space. This color\nwill be used for the following purposes:\n*  The background of the annotation\'s icon when closed\n*  The title bar of the annotation\'s pop-up window\n*  The border of a link annotation')\
          .done().done()\
      .optional()\
          .field('CA')\
          .name('CA')\
          .type('number')\
          .comment('(Optional; PDF 1.4) The constant opacity value to be used in painting the\nannotation (see Sections 7.1, "Overview of Transparency," and 7.2.6, "Shape\nand Opacity Computations"). This value applies to all visible elements of\nthe annotation in its closed state (including its background and border), but\nnot to the pop-up window that appears when the annotation is opened. The\nspecified value is used as the initial alpha constant (both stroking and non-\nstroking) for interpreting the annotation\'s appearance stream, if any (see\nSection 8.4.4, "Appearance Streams," and "Constant Shape and Opacity" on\npage 444). The implicit blend mode (see Section 7.2.4, "Blend Mode") is\nNormal. Default value: 1.0.\n                  Note: If no explicit appearance stream is defined for the annotation, it will be\n                  painted by implementation-dependent means that do not necessarily conform to\n                  the Adobe imaging model; in this case, the effect of this entry is implementation-\n                  dependent as well.\n                  Note: This entry is recognized by all of the standard annotation types listed in\n                  Table 8.14 on page 499 except Link, Movie, Widget, PrinterMark, and TrapNet.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Optional; PDF 1.1) The text label to be displayed in the title bar of the anno-\ntation\'s pop-up window when open and active.')\
          .done().done()\
      .optional()\
          .field('Popup')\
          .name('Popup')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) An indirect reference to a pop-up annotation for enter-\ning or editing the text associated with this annotation.')\
          .done().done()\
      .optional()\
          .field('A')\
          .name('A')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.1) An action to be performed when the annotation is acti-\nvated (see Section 8.5, "Actions").\nNote: This entry is not permitted in link annotations if a Dest entry is present\n(see "Link Annotations" on page 501). Also note that the A entry in movie anno-\ntations has a different meaning (see "Movie Annotations" on page 510).')\
          .done().done()\
      .optional()\
          .field('AA')\
          .name('AA')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An additional-actions dictionary defining the anno-\ntation\'s behavior in response to various trigger events (see Section 8.5.2,\n"Trigger Events"). At the time of publication, this entry is used only by wid-\nget annotations.')\
          .done().done()\
      .optional()\
          .field('StructParent')\
          .name('StructParent')\
          .type('integer')\
          .comment('(Required if the annotation is a structural content item; PDF 1.3) The integer\nkey of the annotation\'s entry in the structural parent tree (see "Finding Struc-\nture Elements from Content Items" on page 600).')\
          .done().done()\
      .done()

  pdfspec.addClass('BorderStyleDictionary', 'Dictionary', 'Entries in a border style dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nBorder for a border style dictionary.')\
          .done().done()\
      .optional()\
          .field('W')\
          .name('W')\
          .type('number')\
          .comment('(Optional) The border width in points. If this value is 0, no border is drawn. Default\nvalue: 1.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Optional) The border style:\n    S    (Solid) A solid rectangle surrounding the annotation.\n    D    (Dashed) A dashed rectangle surrounding the annotation. The dash pattern\n         is specified by the D entry (see below).\n    B    (Beveled) A simulated embossed rectangle that appears to be raised above the\n         surface of the page.\n    I    (Inset) A simulated engraved rectangle that appears to be recessed below the\n         surface of the page.\n    U    (Underline) A single line along the bottom of the annotation rectangle.\nOther border styles may be defined in the future. (See implementation note 64 in\nAppendix H.) Default value: S.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('array')\
          .comment('(Optional) A dash array defining a pattern of dashes and gaps to be used in drawing a\ndashed border (border style D above). The dash array is specified in the same format\nas in the line dash pattern parameter of the graphics state (see "Line Dash Pattern" on\npage 155). The dash phase is not specified and is assumed to be 0. For example, a D\nentry of [3 2] specifies a border drawn with 3-point dashes alternating with 2-point\ngaps. Default value: [3].')\
          .done().done()\
      .done()

  pdfspec.addClass('AppearanceDictionary', 'Dictionary', 'Entries in an appearance dictionary')\
      .required('NULL')\
          .field('N')\
          .name('N')\
          .type('stream or dictionary')\
          .comment('(Required) The annotation\'s normal appearance.')\
          .done().done()\
      .optional()\
          .field('R')\
          .name('R')\
          .type('stream or dictionary')\
          .comment('(Optional) The annotation\'s rollover appearance. Default value: the value of\nthe N entry.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('stream or dictionary')\
          .comment('(Optional) The annotation\'s down appearance. Default value: the value of the\nN entry.')\
          .done().done()\
      .done()

  pdfspec.addClass('TextAnnotationDictionary', 'Dictionary', 'Additional entries specific to a text annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Text\nfor a text annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annotation\nis opened. Carriage returns may be used to separate the text into paragraphs.')\
          .done().done()\
      .optional()\
          .field('Open')\
          .name('Open')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether the annotation should initially be displayed\nopen. Default value: false (closed).')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Optional) The name of an icon to be used in displaying the annotation. Viewer\napplications should provide predefined icon appearances for at least the follow-\ning standard names:\n    Comment                   Key                      Note\n    Help                      NewParagraph             Paragraph\n    Insert\nAdditional names may be supported as well. Default value: Note.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nName entry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance Streams."')\
          .done().done()\
      .done()

  pdfspec.addClass('ALinkAnnotationDictionary', 'Dictionary', 'Additional entries specific to a link annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Link\nfor a link annotation.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) An alternate representation of the annotation\'s contents in\nhuman-readable form, useful when extracting the document\'s contents in sup-\nport of accessibility to disabled users or for other purposes (see Section 9.8.2,\n"Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('Dest')\
          .name('Dest')\
          .type('array, name, or string')\
          .comment('(Optional; not permitted if an A entry is present) A destination to be displayed\nwhen the annotation is activated (see Section 8.2.1, "Destinations"; see also\nimplementation note 66 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('H')\
          .name('H')\
          .type('name')\
          .comment('(Optional; PDF 1.2) The annotation\'s highlighting mode, the visual effect to be\nused when the mouse button is pressed or held down inside its active area:\n    N    (None) No highlighting.\n    I    (Invert) Invert the contents of the annotation rectangle.\n    O    (Outline) Invert the annotation\'s border.\n    P    (Push) Display the annotation\'s down appearance, if any (see Section\n         8.4.4, "Appearance Streams"). If no down appearance is defined, offset\n         the contents of the annotation rectangle to appear as if it were being\n         "pushed" below the surface of the page.\nA highlighting mode other than P overrides any down appearance defined for\nthe annotation. Default value: I.\nNote: In PDF 1.1, highlighting is always done by inverting colors inside the anno-\ntation rectangle.')\
          .done().done()\
      .optional()\
          .field('PA')\
          .name('PA')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A URI action (see "URI Actions" on page 523) formerly\nassociated with this annotation. When Web Capture (Section 9.9, "Web Cap-\nture") changes an annotation from a URI to a go-to action ("Go-To Actions"\non page 519), it uses this entry to save the data from the original URI action so\nthat it can be changed back in case the target page for the go-to action is subse-\nquently deleted.')\
          .done().done()\
      .done()

  pdfspec.addClass('FreeTextAnnotationDictionary', 'Dictionary', 'Additional entries specific to a free text annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be\nFreeText for a free text annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed.')\
          .done().done()\
      .required('NULL')\
          .field('DA')\
          .name('DA')\
          .type('string')\
          .comment('(Required) The default appearance string to be used in formatting the text (see\n"Variable Text" on page 533).\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the DA\nentry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance Streams."')\
          .done().done()\
      .optional()\
          .field('Q')\
          .name('Q')\
          .type('integer')\
          .comment('(Optional; PDF 1.4) A code specifying the form of quadding (justification) to be\nused in displaying the annotation\'s text:\n   0     Left-justified\n   1     Centered\n   2     Right-justified\nDefault value: 0 (left-justified).')\
          .done().done()\
      .done()

  pdfspec.addClass('LineAnnotationDictionary', 'Dictionary', 'Additional entries specific to a line annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Line\nfor a line annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annotation\nis opened. Carriage returns may be used to separate the text into paragraphs.')\
          .done().done()\
      .required('NULL')\
          .field('L')\
          .name('L')\
          .type('array')\
          .comment('(Required) An array of four numbers, [x1 y1 x2 y2 ], specifying the starting and\nending coordinates of the line in default user space.')\
          .done().done()\
      .optional()\
          .field('BS')\
          .name('BS')\
          .type('dictionary')\
          .comment('(Optional) A border style dictionary (see Table 8.12 on page 495) specifying the\nwidth and dash pattern to be used in drawing the line.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the L\nand BS entries; see Table 8.10 on page 490 and Section 8.4.4, "Appearance Streams."')\
          .done().done()\
      .optional()\
          .field('LE')\
          .name('LE')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of two names specifying the line ending styles to be\nused in drawing the line. The first and second elements of the array specify the\nline ending styles for the endpoints defined, respectively, by the first and second\npairs of coordinates, (x1 , y1 ) and (x2 , y2 ), in the L array. Table 8.19 shows the\npossible values. Default value: [/None /None].')\
          .done().done()\
      .optional()\
          .field('IC')\
          .name('IC')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of three numbers in the range 0.0 to 1.0 specifying\nthe components, in the DeviceRGB color space, of the interior color with which to\nfill the annotation\'s line endings (see Table 8.19). If this entry is absent, the inte-\nriors of the line endings are left transparent.')\
          .done().done()\
      .done()

  pdfspec.addClass('SquareOrCircleAnnotation', 'Dictionary', 'Additional entries specific to a square or circle annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Square\nor Circle for a square or circle annotation, respectively.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annotation\nis opened. Carriage returns may be used to separate the text into paragraphs.')\
          .done().done()\
      .optional()\
          .field('BS')\
          .name('BS')\
          .type('dictionary')\
          .comment('(Optional) A border style dictionary (see Table 8.12 on page 495) specifying the\nline width and dash pattern to be used in drawing the rectangle or ellipse.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nRect and BS entries; see Table 8.10 on page 490 and Section 8.4.4, "Appearance\nStreams."')\
          .done().done()\
      .optional()\
          .field('IC')\
          .name('IC')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of three numbers in the range 0.0 to 1.0 specifying\nthe components, in the DeviceRGB color space, of the interior color with which to\nfill the annotation\'s rectangle or ellipse (see Table 8.19). If this entry is absent,\nthe interior of the annotation is left transparent.')\
          .done().done()\
      .done()

  pdfspec.addClass('MarkupAnnotationsDictionary', 'Dictionary', 'Additional entries specific to markup annotations')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be\nHighlight, Underline, Squiggly, or StrikeOut for a highlight, underline,\nsquiggly-underline, or strikeout annotation, respectively.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annota-\ntion is opened. Carriage returns may be used to separate the text into para-\ngraphs.')\
          .done().done()\
      .required('NULL')\
          .field('QuadPoints')\
          .name('QuadPoints')\
          .type('array')\
          .comment('(Required) An array of 8 x n numbers specifying the coordinates of n quadri-\nlaterals in default user space. Each quadrilateral encompasses a word or\ngroup of contiguous words in the text underlying the annotation. The coor-\ndinates for each quadrilateral are given in the order\n    x1 y1 x2 y2 x3 y3 x4 y4\nspecifying the quadrilateral\'s four vertices in counterclockwise order (see\nFigure 8.5). The text is oriented with respect to the edge connecting points\n(x1 , y1) and (x2 , y2). (See implementation note 67 in Appendix H.)\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nQuadPoints entry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance\nStreams."\n                             (x3 , y3 )\n                            ter\n                                        (x2 , y2 )\n                          pi\n    (x4 , y4 )\n                 Ju\n               (x1 , y1 )\n   FIGURE 8.5 QuadPoints specification')\
          .done().done()\
      .done()

  pdfspec.addClass('RubberStampAnnotationDictionary', 'Dictionary', 'Additional entries specific to a rubber stamp annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Stamp\nfor a rubber stamp annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annotation\nis opened. Carriage returns may be used to separate the text into paragraphs.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Optional) The name of an icon to be used in displaying the annotation. Viewer\napplications should provide predefined icon appearances for at least the follow-\ning standard names:\n    Approved                 Experimental              NotApproved\n    AsIs                     Expired                   NotForPublicRelease\n    Confidential              Final                     Sold\n    Departmental             ForComment                TopSecret\n    Draft                    ForPublicRelease\nAdditional names may be supported as well. Default value: Draft.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nName entry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance Streams."')\
          .done().done()\
      .done()

  pdfspec.addClass('InkAnnotationDictionary', 'Dictionary', 'Additional entries specific to an ink annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Ink for\nan ink annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annotation\nis opened. Carriage returns may be used to separate the text into paragraphs.')\
          .done().done()\
      .required('NULL')\
          .field('InkList')\
          .name('InkList')\
          .type('array')\
          .comment('(Required) An array of n arrays, each representing a stroked path. Each array is a\nseries of alternating horizontal and vertical coordinates in default user space,\nspecifying points along the path. When drawn, the points are connected by\nstraight lines or curves in an implementation-dependent way. (See implementa-\ntion note 68 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('BS')\
          .name('BS')\
          .type('dictionary')\
          .comment('(Optional) A border style dictionary (see Table 8.12 on page 495) specifying the\nline width and dash pattern to be used in drawing the paths.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nInkList and BS entries; see Table 8.10 on page 490 and Section 8.4.4, "Appearance\nStreams."')\
          .done().done()\
      .done()

  pdfspec.addClass('PopUpAnnotationDictionary', 'Dictionary', 'Additional entries specific to a pop-up annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be\nPopup for a pop-up annotation.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) An alternate representation of the annotation\'s contents\nin human-readable form, useful when extracting the document\'s contents in\nsupport of accessibility to disabled users or for other purposes (see Section\n9.8.2, "Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('Parent')\
          .name('Parent')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) The parent annotation with which\nthis pop-up annotation is associated.\nNote: If this entry is present, the parent annotation\'s Contents, M, C, and T\nentries (see Table 8.10 on page 490) override those of the pop-up annotation\nitself.')\
          .done().done()\
      .optional()\
          .field('Open')\
          .name('Open')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether the pop-up annotation should initially\nbe displayed open. Default value: false (closed).')\
          .done().done()\
      .done()

  pdfspec.addClass('FileAttachmentAnnotationDictionary', 'Dictionary', 'Additional entries specific to a file attachment annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be\nFileAttachment for a file attachment annotation.')\
          .done().done()\
      .required('NULL')\
          .field('FS')\
          .name('FS')\
          .type('file specification')\
          .comment('(Required) The file associated with this annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Required) The text to be displayed in the pop-up window when the annota-\ntion is opened. Carriage returns may be used to separate the text into para-\ngraphs.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Optional) The name of an icon to be used in displaying the annotation.\nViewer applications should provide predefined icon appearances for at least\nthe following standard names:\n    Graph                                  PushPin\n    Paperclip                              Tag\nAdditional names may be supported as well. Default value: PushPin.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over\nthe Name entry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance\nStreams."')\
          .done().done()\
      .done()

  pdfspec.addClass('SoundAnnotationDictionary', 'Dictionary', 'Additional entries specific to a sound annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Sound\nfor a sound annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Sound')\
          .name('Sound')\
          .type('stream')\
          .comment('(Required) A sound object defining the sound to be played when the annotation\nis activated (see Section 8.7, "Sounds").')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional) Text to be displayed in a pop-up window for the annotation in place\nof the sound, useful when extracting the document\'s contents in support of\naccessibility to disabled users or for other purposes (see Section 9.8.2, "Alternate\nDescriptions").')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('name')\
          .comment('(Optional) The name of an icon to be used in displaying the annotation. Viewer\napplications should provide predefined icon appearances for at least the stan-\ndard names Speaker and Microphone; additional names may be supported as\nwell. Default value: Speaker.\nNote: The annotation dictionary\'s AP entry, if present, takes precedence over the\nName entry; see Table 8.10 on page 490 and Section 8.4.4, "Appearance Streams."')\
          .done().done()\
      .done()

  pdfspec.addClass('MovieAnnotationDictionary', 'Dictionary', 'Additional entries specific to a movie annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Movie\nfor a movie annotation.')\
          .done().done()\
      .required('NULL')\
          .field('Movie')\
          .name('Movie')\
          .type('dictionary')\
          .comment('(Required) A movie dictionary describing the movie\'s static characteristics (see\nSection 8.8, "Movies").')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) An alternate representation of the annotation\'s contents in\nhuman-readable form, useful when extracting the document\'s contents in sup-\nport of accessibility to disabled users or for other purposes (see Section 9.8.2,\n"Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('A')\
          .name('A')\
          .type('boolean or dictionary')\
          .comment('(Optional) A flag or dictionary specifying whether and how to play the movie\nwhen the annotation is activated. If this value is a dictionary, it is a movie activa-\ntion dictionary (see Section 8.8, "Movies") specifying how to play the movie; if it\nis the boolean value true, the movie should be played using default activation\nparameters; if it is false, the movie should not be played at all. Default value:\ntrue.')\
          .done().done()\
      .done()

  pdfspec.addClass('WidgetAnnotationDictionary', 'Dictionary', 'Additional entries specific to a widget annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Widget\nfor a widget annotation.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) An alternate representation of the annotation\'s contents in\nhuman-readable form, useful when extracting the document\'s contents in sup-\nport of accessibility to disabled users or for other purposes (see Section 9.8.2,\n"Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('H')\
          .name('H')\
          .type('name')\
          .comment('(Optional) The annotation\'s highlighting mode, the visual effect to be used when\nthe mouse button is pressed or held down inside its active area:\n   N    (None) No highlighting.\n   I    (Invert) Invert the contents of the annotation rectangle.\n   O    (Outline) Invert the annotation\'s border.\n   P    (Push) Display the annotation\'s down appearance, if any (see Section\n        8.4.4, "Appearance Streams"). If no down appearance is defined, offset\n        the contents of the annotation rectangle to appear as if it were being\n        "pushed" below the surface of the page.\n   T    (Toggle) Same as P (which is preferred).\nA highlighting mode other than P overrides any down appearance defined for\nthe annotation. Default value: I.')\
          .done().done()\
      .optional()\
          .field('MK')\
          .name('MK')\
          .type('dictionary')\
          .comment('(Optional) An appearance characteristics dictionary to be used in constructing a\ndynamic appearance stream specifying the annotation\'s visual presentation on\nthe page; see "Variable Text" on page 533 for further discussion.\nNote: The name MK for this entry is of historical significance only and has no direct\nmeaning.')\
          .done().done()\
      .done()

  pdfspec.addClass('ActionDictionary', 'Dictionary', 'Entries common to all action dictionaries')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be Action for an action dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; see Table 8.34\non page 518 for specific values.')\
          .done().done()\
      .optional()\
          .field('Next')\
          .name('Next')\
          .type('dictionary or array')\
          .comment('(Optional; PDF 1.2) The next action, or sequence of actions, to be per-\nformed after this one. The value is either a single action dictionary or an\narray of action dictionaries to be performed in order; see below for fur-\nther discussion.')\
          .done().done()\
      .done()

  pdfspec.addClass('AnnotationActionsDictionary', 'Dictionary', 'Entries in an annotation\'s additional-actions dictionary')\
      .optional()\
          .field('E')\
          .name('E')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the cursor enters the annotation\'s\nactive area.')\
          .done().done()\
      .optional()\
          .field('X')\
          .name('X')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the cursor exits the annotation\'s\nactive area.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the mouse button is pressed\ninside the annotation\'s active area. (The name D stands for "down.")')\
          .done().done()\
      .optional()\
          .field('U')\
          .name('U')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the mouse button is released\ninside the annotation\'s active area. (The name U stands for "up.")\nNote: For backward compatibility, the A entry in an annotation dictionary, if present,\ntakes precedence over this entry (see Table 8.10 on page 490).')\
          .done().done()\
      .optional()\
          .field('Fo')\
          .name('Fo')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2; widget annotations only) An action to be performed when the\nannotation receives the input focus.')\
          .done().done()\
      .optional()\
          .field('Bl')\
          .name('Bl')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2; widget annotations only) (Uppercase B, lowercase L) An action to\nbe performed when the annotation loses the input focus. (The name Bl stands for\n"blurred.")')\
          .done().done()\
      .done()

  pdfspec.addClass('PageObjectActionsDictionary', 'Dictionary', 'Entries in a page object\'s additional-actions dictionary')\
      .optional()\
          .field('O')\
          .name('O')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the page is opened (for example,\nwhen the user navigates to it from the next or previous page or via a link annotation or\noutline item). This action is independent of any that may be defined by the Open-\nAction entry in the document catalog (see Section 3.6.1, "Document Catalog"), and is\nexecuted after such an action. (See implementation note 72 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An action to be performed when the page is closed (for example,\nwhen the user navigates to the next or previous page or follows a link annotation or an\noutline item). This action applies to the page being closed, and is executed before any\nother page is opened. (See implementation note 72 in Appendix H.)')\
          .done().done()\
      .done()

  pdfspec.addClass('FormFieldActionsDictionary', 'Dictionary', 'Entries in a form field\'s additional-actions dictionary')\
      .optional()\
          .field('K')\
          .name('K')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A JavaScript action to be performed when the user types a key-\nstroke into a text field or combo box or modifies the selection in a scrollable list box.\nThis allows the keystroke to be checked for validity and rejected or modified.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A JavaScript action to be performed before the field is formatted\nto display its current value. This allows the field\'s value to be modified before format-\nting.')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A JavaScript action to be performed when the field\'s value is\nchanged. This allows the new value to be checked for validity. (The name V stands for\n"validate.")')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A JavaScript action to be performed in order to recalculate the\nvalue of this field when that of another field changes. (The name C stands for\n"calculate.") The order in which the document\'s fields are recalculated is defined by the\nCO entry in the interactive form dictionary (see Section 8.6.1, "Interactive Form\nDictionary").')\
          .done().done()\
      .done()

  pdfspec.addClass('DocumentCatalogActionsDictionary', 'Dictionary', 'Entries in the document catalog\'s additional-actions dictionary')\
      .optional()\
          .field('DC')\
          .name('DC')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript action to be performed before closing a document.\n(The name DC stands for "document close.")')\
          .done().done()\
      .optional()\
          .field('WS')\
          .name('WS')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript action to be performed before saving a document.\n(The name WS stands for "will save.")')\
          .done().done()\
      .optional()\
          .field('DS')\
          .name('DS')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript action to be performed after saving a document. (The\nname DS stands for "did save.")')\
          .done().done()\
      .optional()\
          .field('WP')\
          .name('WP')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript action to be performed before printing a document.\n(The name WP stands for "will print.")')\
          .done().done()\
      .optional()\
          .field('DP')\
          .name('DP')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript action to be performed after printing a document.\n(The name DP stands for "did print.")')\
          .done().done()\
      .done()

  pdfspec.addClass('GoToActionDictionary', 'Dictionary', 'Additional entries specific to a go-to action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be GoTo for a\ngo-to action.')\
          .done().done()\
      .required('NULL')\
          .field('D')\
          .name('D')\
          .type('name, string, or array')\
          .comment('(Required) The destination to jump to (see Section 8.2.1, "Destinations").')\
          .done().done()\
      .done()

  pdfspec.addClass('RemoteGoToActionDictionary', 'Dictionary', 'Additional entries specific to a remote go-to action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be GoToR\nfor a remote go-to action.')\
          .done().done()\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required) The file in which the destination is located.')\
          .done().done()\
      .required('NULL')\
          .field('D')\
          .name('D')\
          .type('name, string, or array')\
          .comment('(Required) The destination to jump to (see Section 8.2.1, "Destinations"). If\nthe value is an array defining an explicit destination (as described under\n"Explicit Destinations" on page 474), its first element must be a page number\nwithin the remote document rather than an indirect reference to a page ob-\nject in the current document. The first page is numbered 0.')\
          .done().done()\
      .optional()\
          .field('NewWindow')\
          .name('NewWindow')\
          .type('boolean')\
          .comment('(Optional; PDF 1.2) A flag specifying whether to open the destination docu-\nment in a new window. If this flag is false, the destination document will\nreplace the current document in the same window. If this entry is absent,\nthe viewer application should behave in accordance with the current user\npreference.')\
          .done().done()\
      .done()

  pdfspec.addClass('LaunchActionDictionary', 'Dictionary', 'Additional entries specific to a launch action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Launch\nfor a launch action.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required if none of the entries Win, Mac, or Unix is present) The application to\nbe launched or the document to be opened or printed. If this entry is absent\nand the viewer application does not understand any of the alternative entries,\nit should do nothing.')\
          .done().done()\
      .optional()\
          .field('Win')\
          .name('Win')\
          .type('dictionary')\
          .comment('(Optional) A dictionary containing Windows-specific launch parameters (see\nthe Table 8.38; see also implementation note 73 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('Mac')\
          .name('Mac')\
          .type('(undefined)')\
          .comment('(Optional) Mac OS\'specific launch parameters; not yet defined.')\
          .done().done()\
      .optional()\
          .field('Unix')\
          .name('Unix')\
          .type('(undefined)')\
          .comment('(Optional) UNIX-specific launch parameters; not yet defined.')\
          .done().done()\
      .optional()\
          .field('NewWindow')\
          .name('NewWindow')\
          .type('boolean')\
          .comment('(Optional; PDF 1.2) A flag specifying whether to open the destination docu-\nment in a new window. If this flag is false, the destination document will\nreplace the current document in the same window. If this entry is absent, the\nviewer application should behave in accordance with the current user prefer-\nence. This entry is ignored if the file designated by the F entry is not a PDF\ndocument.')\
          .done().done()\
      .done()

  pdfspec.addClass('WindowsLaunchActionDictionary', 'Dictionary', 'Entries in a Windows launch parameter dictionary')\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('string')\
          .comment('(Required) The file name of the application to be launched or the document\nto be opened or printed, in standard Windows pathname format. If the name\nstring includes a backslash character (\), the backslash must itself be preceded\nby a backslash.\nNote: This value must be a simple string; it is not a file specification.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('string')\
          .comment('(Optional) A string specifying the default directory in standard DOS syntax.')\
          .done().done()\
      .optional()\
          .field('O')\
          .name('O')\
          .type('string')\
          .comment('(Optional) A string specifying the operation to perform:\n    open      Open a document.\n    print     Print a document.\nIf the F entry designates an application instead of a document, this entry is ig-\nnored and the application is launched. Default value: open.')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('string')\
          .comment('(Optional) A parameter string to be passed to the application designated by\nthe F entry. This entry should be omitted if F designates a document.')\
          .done().done()\
      .done()

  pdfspec.addClass('ThreadActionDictionary', 'Dictionary', 'Additional entries specific to a thread action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Thread\nfor a thread action.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Optional) The file containing the desired thread. If this entry is absent, the\nthread is in the current file.')\
          .done().done()\
      .required('NULL')\
          .field('D')\
          .name('D')\
          .type('dictionary, integer, or text string')\
          .comment('(Required) The desired destination thread, specified in one of the following\nforms:\n*  An indirect reference to a thread dictionary (see Section 8.3.2, "Articles").\n   In this case, the thread must be in the current file.\n*  The index of the thread within the Threads array of its document\'s catalog\n   (see Section 3.6.1, "Document Catalog"). The first thread in the array has\n   index 0.\n*  The title of the thread, as specified in its thread information dictionary (see\n   Table 8.7 on page 484). If two or more threads have the same title, the one\n   appearing first in the document catalog\'s Threads array will be used.')\
          .done().done()\
      .optional()\
          .field('B')\
          .name('B')\
          .type('dictionary or integer')\
          .comment('(Optional) The desired bead in the destination thread, specified in one of the\nfollowing forms:\n*  An indirect reference to a bead dictionary (see Section 8.3.2, "Articles"). In\n   this case, the thread must be in the current file.\n*  The index of the bead within its thread. The first bead in a thread has\n   index 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('URIActionDictionary', 'Dictionary', 'Additional entries specific to a URI action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be URI for a URI\naction.')\
          .done().done()\
      .required('NULL')\
          .field('URI')\
          .name('URI')\
          .type('string')\
          .comment('(Required) The uniform resource identifier to resolve, encoded in 7-bit ASCII.')\
          .done().done()\
      .optional()\
          .field('IsMap')\
          .name('IsMap')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to track the mouse position when the URI is re-\nsolved (see below). Default value: false.\nThis entry applies only to actions triggered by the user\'s clicking an annotation; it is\nignored for actions associated with outline items or with a document\'s OpenAction\nentry.')\
          .done().done()\
      .done()

  pdfspec.addClass('URIDictionary', 'Dictionary', 'Entry in a URI dictionary')\
      .optional()\
          .field('Base')\
          .name('Base')\
          .type('string')\
          .comment('(Optional) The base URI to be used in resolving relative URI references. URI actions\nwithin the document may specify URIs in partial form, to be interpreted relative to\nthis base address. If no base URI is specified, such partial URIs will be interpreted rel-\native to the location of the document itself. The use of this entry is parallel to that of\nthe body element <BASE>, as described in section 2.7.2 of Internet RFC 1866, Hyper-\ntext Markup Language 2.0 Proposed Standard (see the Bibliography).')\
          .done().done()\
      .done()

  pdfspec.addClass('SoundActionDictionary', 'Dictionary', 'Additional entries specific to a sound action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Sound\nfor a sound action.')\
          .done().done()\
      .required('NULL')\
          .field('Sound')\
          .name('Sound')\
          .type('stream')\
          .comment('(Required) A sound object defining the sound to be played (see Section 8.7,\n"Sounds"; see also implementation note 76 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('Volume')\
          .name('Volume')\
          .type('number')\
          .comment('(Optional) The volume at which to play the sound, in the range -1.0 to 1.0.\nHigher values denote greater volume; negative values mute the sound.\nDefault value: 1.0.')\
          .done().done()\
      .optional()\
          .field('Synchronous')\
          .name('Synchronous')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to play the sound synchronously or\nasynchronously. If this flag is true, the viewer application will retain control,\nallowing no further user interaction other than canceling the sound, until the\nsound has been completely played. Default value: false.')\
          .done().done()\
      .optional()\
          .field('Repeat')\
          .name('Repeat')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to repeat the sound indefinitely. If this\nentry is present, the Synchronous entry is ignored. Default value: false.')\
          .done().done()\
      .optional()\
          .field('Mix')\
          .name('Mix')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to mix this sound with any other sound\nalready playing. If this flag is false, any previously playing sound will be\nstopped before starting this sound; this can be used to stop a repeating sound\n(see Repeat, above). Default value: false.')\
          .done().done()\
      .done()

  pdfspec.addClass('MovieActionDictionary', 'Dictionary', 'Additional entries specific to a movie action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Movie\nfor a movie action.')\
          .done().done()\
      .optional()\
          .field('Annot')\
          .name('Annot')\
          .type('dictionary')\
          .comment('(Optional) An indirect reference to a movie annotation identifying the movie\nto be played.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Optional) The title of a movie annotation identifying the movie to be\nplayed.\nNote: The dictionary must include either an Annot or a T entry, but not both.')\
          .done().done()\
      .optional()\
          .field('Operation')\
          .name('Operation')\
          .type('name')\
          .comment('(Optional) The operation to be performed on the movie:\n   Play         Start playing the movie, using the play mode specified by the\n                dictionary\'s Mode entry (see Table 8.79 on page 571). If the\n                movie is currently paused, it is repositioned to the beginning\n                before playing (or to the starting point specified by the dic-\n                tionary\'s Start entry, if present).\n   Stop         Stop playing the movie.\n   Pause        Pause a playing movie.\n   Resume       Resume a paused movie.\nDefault value: Play.')\
          .done().done()\
      .done()

  pdfspec.addClass('HideActionDictionary', 'Dictionary', 'Additional entries specific to a hide action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Hide for a hide\naction.')\
          .done().done()\
      .required('NULL')\
          .field('T')\
          .name('T')\
          .type('dictionary, string, or array')\
          .comment('(Required) The annotation or annotations to be hidden or shown, specified in any\nof the following forms:\n*  An indirect reference to an annotation dictionary\n*  A string giving the fully qualified field name of an interactive form field whose\n   associated widget annotation or annotations are to be affected (see "Field\n   Names" on page 532)\n*  An array of such dictionaries or strings')\
          .done().done()\
      .optional()\
          .field('H')\
          .name('H')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether to hide the annotation (true) or show it (false).\nDefault value: true.')\
          .done().done()\
      .done()

  pdfspec.addClass('NamedActionsDictionary', 'Dictionary', 'Additional entries specific to named actions')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be Named for a named\naction.')\
          .done().done()\
      .required('NULL')\
          .field('N')\
          .name('N')\
          .type('name')\
          .comment('(Required) The name of the action to be performed (see Table 8.45).')\
          .done().done()\
      .done()

  pdfspec.addClass('InteractiveFormDictionary', 'Dictionary', 'Entries in the interactive form dictionary')\
      .required('NULL')\
          .field('Fields')\
          .name('Fields')\
          .type('array')\
          .comment('(Required) An array of references to the document\'s root fields (those with\nno ancestors in the field hierarchy).')\
          .done().done()\
      .optional()\
          .field('NeedAppearances')\
          .name('NeedAppearances')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to construct appearance streams and\nappearance dictionaries for all widget annotations in the document (see\n"Variable Text" on page 533). Default value: false.')\
          .done().done()\
      .optional()\
          .field('SigFlags')\
          .name('SigFlags')\
          .type('integer')\
          .comment('(Optional; PDF 1.3) A set of flags specifying various document-level char-\nacteristics related to signature fields (see Table 8.48, below, and "Signature\nFields" on page 547). Default value: 0.')\
          .done().done()\
      .optional()\
          .field('CO')\
          .name('CO')\
          .type('array')\
          .comment('(Required if any fields in the document have additional-actions dictionaries\ncontaining a C entry; PDF 1.3) An array of indirect references to field dic-\ntionaries with calculation actions, defining the calculation order in which\ntheir values will be recalculated when the value of any field changes (see\nSection 8.5.2, "Trigger Events").')\
          .done().done()\
      .optional()\
          .field('DR')\
          .name('DR')\
          .type('dictionary')\
          .comment('(Optional) A document-wide default value for the DR attribute of variable\ntext fields (see "Variable Text" on page 533).')\
          .done().done()\
      .optional()\
          .field('DA')\
          .name('DA')\
          .type('string')\
          .comment('(Optional) A document-wide default value for the DA attribute of variable\ntext fields (see "Variable Text" on page 533).')\
          .done().done()\
      .optional()\
          .field('Q')\
          .name('Q')\
          .type('integer')\
          .comment('(Optional) A document-wide default value for the Q attribute of variable\ntext fields (see "Variable Text" on page 533).')\
          .done().done()\
      .done()

  pdfspec.addClass('FieldDictionary', 'Dictionary', 'Entries common to all field dictionaries')\
      .optional()\
          .field('FT')\
          .name('FT')\
          .type('name')\
          .comment('(Required for terminal fields; inheritable) The type of field that this dictionary\ndescribes:\n     Btn       Button (see "Button Fields" on page 538)\n     Tx        Text (see "Text Fields" on page 543)\n     Ch        Choice (see "Choice Fields" on page 545)\n     Sig       (PDF 1.3) Signature (see "Signature Fields" on page 547)\nNote: This entry may be present in a nonterminal field (one whose descendants\nare themselves fields) in order to provide an inheritable FT value. However, a\nnonterminal field does not logically have a type of its own; it is merely a contain-\ner for inheritable attributes that are intended for descendant terminal fields of\nany type.')\
          .done().done()\
      .optional()\
          .field('Parent')\
          .name('Parent')\
          .type('dictionary')\
          .comment('(Required if this field is the child of another in the field hierarchy; absent other-\nwise) The field that is the immediate parent of this one (the field, if any,\nwhose Kids array includes this field). A field can have at most one parent; that\nis, it can be included in the Kids array of at most one other field.')\
          .done().done()\
      .optional()\
          .field('Kids')\
          .name('Kids')\
          .type('array')\
          .comment('(Optional) An array of indirect references to the immediate children of this\nfield.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Optional) The partial field name (see "Field Names," below; see also imple-\nmentation notes 82 and 83 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('TU')\
          .name('TU')\
          .type('text string')\
          .comment('(Optional; PDF 1.3) An alternate field name, to be used in place of the actual\nfield name wherever the field must be identified in the user interface (such as\nin error or status messages referring to the field). This text is also useful\nwhen extracting the document\'s contents in support of accessibility to dis-\nabled users or for other purposes (see Section 9.8.2, "Alternate Descrip-\ntions").')\
          .done().done()\
      .optional()\
          .field('TM')\
          .name('TM')\
          .type('text string')\
          .comment('(Optional; PDF 1.3) The mapping name to be used when exporting inter-\nactive form field data from the document.')\
          .done().done()\
      .optional()\
          .field('Ff')\
          .name('Ff')\
          .type('integer')\
          .comment('(Optional; inheritable) A set of flags specifying various characteristics of the\nfield (see Table 8.50). Default value: 0.')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('(various)')\
          .comment('(Optional; inheritable) The field\'s value, whose format varies depending on\nthe field type; see the descriptions of individual field types for further infor-\nmation.')\
          .done().done()\
      .optional()\
          .field('DV')\
          .name('DV')\
          .type('(various)')\
          .comment('(Optional; inheritable) The default value to which the field reverts when a\nreset-form action is executed (see "Reset-Form Actions" on page 554). The\nformat of this value is the same as that of V.')\
          .done().done()\
      .optional()\
          .field('AA')\
          .name('AA')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.2) An additional-actions dictionary defining the field\'s\nbehavior in response to various trigger events (see Section 8.5.2, "Trigger\nEvents"). This entry has exactly the same meaning as the AA entry in an\nannotation dictionary (see Section 8.4.1, "Annotation Dictionaries").')\
          .done().done()\
      .done()

  pdfspec.addClass('VariableTextFieldDictionary', 'Dictionary', 'Additional entries common to all fields containing variable text')\
      .required('NULL')\
          .field('DR')\
          .name('DR')\
          .type('dictionary')\
          .comment('(Required; inheritable) A resource dictionary (see Section 3.7.2, "Resource Diction-\naries") containing default resources (such as fonts, patterns, or color spaces) to be used\nby the appearance stream. At a minimum, this dictionary must contain a Font entry\nspecifying the resource name and font dictionary of the default font for displaying the\nfield\'s text. (See implementation note 84 in Appendix H.)')\
          .done().done()\
      .required('NULL')\
          .field('DA')\
          .name('DA')\
          .type('string')\
          .comment('(Required; inheritable) The default appearance string, containing a sequence of valid\npage-content graphics or text state operators defining such properties as the field\'s text\nsize and color.')\
          .done().done()\
      .optional()\
          .field('Q')\
          .name('Q')\
          .type('integer')\
          .comment('(Optional; inheritable) A code specifying the form of quadding (justification) to be\nused in displaying the text:\n    0    Left-justified\n    1    Centered\n    2    Right-justified\nDefault value: 0 (left-justified).')\
          .done().done()\
      .done()

  pdfspec.addClass('AppearanceCharacteristicsDictionary', 'Dictionary', 'Entries in an appearance characteristics dictionary')\
      .optional()\
          .field('R')\
          .name('R')\
          .type('integer')\
          .comment('(Optional) The number of degrees by which the widget annotation is rotated\ncounterclockwise relative to the page. The value must be a multiple of 90.\nDefault value: 0.')\
          .done().done()\
      .optional()\
          .field('BC')\
          .name('BC')\
          .type('array')\
          .comment('(Optional) An array of numbers in the range 0.0 to 1.0 specifying the color of the\nwidget annotation\'s border. The number of array elements determines the color\nspace in which the color is defined:\n   0    No color; transparent\n   1    DeviceGray\n   3    DeviceRGB\n   4    DeviceCMYK')\
          .done().done()\
      .optional()\
          .field('BG')\
          .name('BG')\
          .type('array')\
          .comment('(Optional) An array of numbers in the range 0.0 to 1.0 specifying the color of the\nwidget annotation\'s background. The number of array elements determines the\ncolor space, as described above for BC.')\
          .done().done()\
      .optional()\
          .field('CA')\
          .name('CA')\
          .type('text string')\
          .comment('(Optional; button fields only) The widget annotation\'s normal caption, displayed\nwhen it is not interacting with the user.\nNote: Unlike the remaining entries listed below, which apply only to widget annota-\ntions associated with pushbutton fields (see "Pushbuttons" on page 539), the CA\nentry can be used with any type of button field, including checkboxes ("Checkboxes"\non page 539) and radio buttons ("Radio Buttons" on page 540).')\
          .done().done()\
      .optional()\
          .field('RC')\
          .name('RC')\
          .type('text string')\
          .comment('(Optional; pushbutton fields only) The widget annotation\'s rollover caption, dis-\nplayed when the user rolls the cursor into its active area without pressing the\nmouse button.')\
          .done().done()\
      .optional()\
          .field('AC')\
          .name('AC')\
          .type('text string')\
          .comment('(Optional; pushbutton fields only) The widget annotation\'s alternate (down)\ncaption, displayed when the mouse button is pressed within its active area.')\
          .done().done()\
      .optional()\
          .field('I')\
          .name('I')\
          .type('stream')\
          .comment('(Optional; pushbutton fields only; must be an indirect reference) A form XObject\ndefining the widget annotation\'s normal icon, displayed when it is not inter-\nacting with the user.')\
          .done().done()\
      .optional()\
          .field('RI')\
          .name('RI')\
          .type('stream')\
          .comment('(Optional; pushbutton fields only; must be an indirect reference) A form XObject\ndefining the widget annotation\'s rollover icon, displayed when the user rolls the\ncursor into its active area without pressing the mouse button.')\
          .done().done()\
      .optional()\
          .field('IX')\
          .name('IX')\
          .type('stream')\
          .comment('(Optional; pushbutton fields only; must be an indirect reference) A form XObject\ndefining the widget annotation\'s alternate (down) icon, displayed when the\nmouse button is pressed within its active area.')\
          .done().done()\
      .optional()\
          .field('IF')\
          .name('IF')\
          .type('dictionary')\
          .comment('(Optional; pushbutton fields only) An icon fit dictionary (see Table 8.73 on page\n566) specifying how to display the widget annotation\'s icon within its\nannotation rectangle. If present, the icon fit dictionary applies to all of the anno-\ntation\'s icons (normal, rollover, and alternate).')\
          .done().done()\
      .optional()\
          .field('TP')\
          .name('TP')\
          .type('integer')\
          .comment('(Optional; pushbutton fields only) A code indicating where to position the text of\nthe widget annotation\'s caption relative to its icon:\n    0    No icon; caption only\n    1    No caption; icon only\n    2    Caption below the icon\n    3    Caption above the icon\n    4    Caption to the right of the icon\n    5    Caption to the left of the icon\n    6    Caption overlaid directly on the icon\nDefault value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('CheckboxFieldDictionary', 'Dictionary', 'Additional entry specific to a checkbox field')\
      .optional()\
          .field('Opt')\
          .name('Opt')\
          .type('text string')\
          .comment('(Optional; inheritable; PDF 1.4) A text string to be used in place of the V entry for the\nvalue of the field.')\
          .done().done()\
      .done()

  pdfspec.addClass('RadioButtonFieldDictionary', 'Dictionary', 'Additional entry specific to a radio button field')\
      .optional()\
          .field('Opt')\
          .name('Opt')\
          .type('array')\
          .comment('(Optional; inheritable; PDF 1.4) An array of text strings to be used in\nplace of the V entries for the values of the widget annotations repre-\nsenting the individual radio buttons. Each element in the array repre-\nsents the export value of the corresponding widget annotation in the\nKids array of the radio button field.')\
          .done().done()\
      .done()

  pdfspec.addClass('TextFieldDictionary', 'Dictionary', 'Additional entry specific to a text field')\
      .optional()\
          .field('MaxLen')\
          .name('MaxLen')\
          .type('integer')\
          .comment('(Optional; inheritable) The maximum length of the field\'s text, in characters.')\
          .done().done()\
      .done()

  pdfspec.addClass('ChoiceFieldDictionary', 'Dictionary', 'Additional entries specific to a choice field')\
      .required('NULL')\
          .field('Opt')\
          .name('Opt')\
          .type('array')\
          .comment('(Required; inheritable) An array of options to be presented to the user. Each element of\nthe array is either a text string representing one of the available options or a two-element\narray consisting of a text string together with a default appearance string for construct-\ning the item\'s appearance dynamically at viewing time (see "Variable Text" on page 533;\nsee also implementation note 85 in Appendix H).')\
          .done().done()\
      .optional()\
          .field('TI')\
          .name('TI')\
          .type('integer')\
          .comment('(Optional; inheritable) For scrollable list boxes, the top index (the index in the Opt array\nof the first option visible in the list).')\
          .done().done()\
      .optional()\
          .field('I')\
          .name('I')\
          .type('array')\
          .comment('(Sometimes required, otherwise optional; inheritable; PDF 1.4) For choice fields that allow\nmultiple selection (MultiSelect flag set), an array of integers, sorted in ascending order,\nrepresenting the zero-based indices in the Opt array of the currently selected option\nitems. This entry is required when two or more elements in the Opt array have different\nnames but the same export value, or when the value of the choice field is an array; in\nother cases, it is permitted but not required. If the items identified by this entry differ\nfrom those in the V entry of the field dictionary (see below), the V entry takes precedence.')\
          .done().done()\
      .done()

  pdfspec.addClass('SignatureDictionary', 'Dictionary', 'Entries in a signature dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present,\nmust be Sig for a signature dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('Filter')\
          .name('Filter')\
          .type('name')\
          .comment('(Required; inheritable) The name of the signature handler to be used for\nauthenticating the field\'s contents, such as Adobe.PPKLite, Entrust.PPKEF,\nCICI.SignIt, or VeriSign.PPKVS.')\
          .done().done()\
      .optional()\
          .field('SubFilter')\
          .name('SubFilter')\
          .type('name')\
          .comment('(Optional) The name of a specific submethod of the specified handler.')\
          .done().done()\
      .required('NULL')\
          .field('ByteRange')\
          .name('ByteRange')\
          .type('array')\
          .comment('(Required) An array of pairs of integers (starting byte offset, length in bytes)\ndescribing the exact byte range for the digest calculation. Multiple discontig-\nuous byte ranges may be used to describe a digest that does not include the\nsignature token itself.')\
          .done().done()\
      .required('NULL')\
          .field('Contents')\
          .name('Contents')\
          .type('string')\
          .comment('(Required) The encrypted signature token.')\
          .done().done()\
      .optional()\
          .field('Name')\
          .name('Name')\
          .type('text string')\
          .comment('(Optional) The name of the person or authority signing the document.')\
          .done().done()\
      .optional()\
          .field('M')\
          .name('M')\
          .type('date')\
          .comment('(Optional) The time of signing. Depending on the signature handler, this\nmay be a normal unverified computer time or a time generated in a verifiable\nway from a secure time server.')\
          .done().done()\
      .optional()\
          .field('Location')\
          .name('Location')\
          .type('text string')\
          .comment('(Optional) The CPU host name or physical location of the signing.')\
          .done().done()\
      .optional()\
          .field('Reason')\
          .name('Reason')\
          .type('text string')\
          .comment('(Optional) The reason for the signing, such as (I agree...).')\
          .done().done()\
      .done()

  pdfspec.addClass('SubmitFormActionDictionary', 'Dictionary', 'Additional entries specific to a submit-form action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must\nbe SubmitForm for a submit-form action.')\
          .done().done()\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required) A URL file specification (see Section 3.10.4, "URL Speci-\nfications") giving the uniform resource locator (URL) of the script\nat the Web server that will process the submission.')\
          .done().done()\
      .optional()\
          .field('Fields')\
          .name('Fields')\
          .type('array')\
          .comment('(Optional) An array identifying which fields to include in the sub-\nmission or which to exclude, depending on the setting of the\nInclude/Exclude flag in the Flags entry (see Table 8.62). Each ele-\nment of the array is either an indirect reference to a field dictionary\nor (PDF 1.3) a string representing the fully qualified name of a field.\nElements of both kinds may be mixed in the same array.\nIf this entry is omitted, the Include/Exclude flag is ignored; all fields\nin the document\'s interactive form are submitted except those\nwhose NoExport flag (see Table 8.50 on page 532) is set. (Fields\nwith no values may also be excluded, depending on the setting of\nthe IncludeNoValueFields flag; see Table 8.62.) See the text follow-\ning Table 8.62 for further discussion.')\
          .done().done()\
      .optional()\
          .field('Flags')\
          .name('Flags')\
          .type('integer')\
          .comment('(Optional; inheritable) A set of flags specifying various characteris-\ntics of the action (see Table 8.62). Default value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('ResetFormActionDictionary', 'Dictionary', 'Additional entries specific to a reset-form action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be\nResetForm for a reset-form action.')\
          .done().done()\
      .optional()\
          .field('Fields')\
          .name('Fields')\
          .type('array')\
          .comment('(Optional) An array identifying which fields to reset or which to exclude\nfrom resetting, depending on the setting of the Include/Exclude flag in\nthe Flags entry (see Table 8.64). Each element of the array is either an in-\ndirect reference to a field dictionary or (PDF 1.3) a string representing\nthe fully qualified name of a field. Elements of both kinds may be mixed\nin the same array.\nIf this entry is omitted, the Include/Exclude flag is ignored; all fields in\nthe document\'s interactive form are reset.')\
          .done().done()\
      .optional()\
          .field('Flags')\
          .name('Flags')\
          .type('integer')\
          .comment('(Optional; inheritable) A set of flags specifying various characteristics of\nthe action (see Table 8.64). Default value: 0.')\
          .done().done()\
      .done()

  pdfspec.addClass('ImportDataActionDictionary', 'Dictionary', 'Additional entries specific to an import-data action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be ImportData\nfor an import-data action.')\
          .done().done()\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required) The FDF file from which to import the data. (See implementation\nnotes 87 and 88 in Appendix H.)')\
          .done().done()\
      .done()

  pdfspec.addClass('JavascriptActionDictionary', 'Dictionary', 'Additional entries specific to a JavaScript action')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of action that this dictionary describes; must be JavaScript\nfor a JavaScript action.')\
          .done().done()\
      .required('NULL')\
          .field('JS')\
          .name('JS')\
          .type('string or stream')\
          .comment('(Required) A string or stream containing the JavaScript script to be executed.\nNote: PDFDocEncoding or Unicode encoding (the latter identified by the Unicode\nprefix U+ FEFF) is used to encode the contents of the string or stream. (See imple-\nmentation note 89 in Appendix H.)')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFTrailerDictionary', 'Dictionary', 'Entry in the FDF trailer dictionary')\
      .optional()\
          .field('Root')\
          .name('Root')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The catalog object for this FDF file (see\n"FDF Catalog," below).')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFCatalogDictionary', 'Dictionary', 'Entries in the FDF catalog dictionary')\
      .optional()\
          .field('Version')\
          .name('Version')\
          .type('name')\
          .comment('(Optional; PDF 1.4) The version of the PDF specification to which\nthis FDF file conforms (for example, 1.4), if later than the version\nspecified in the file\'s header (see "FDF Header" on page 559). If the\nheader specifies a later version, or if this entry is absent, the docu-\nment conforms to the version specified in the header.\nNote: The value of this entry is a name object, not a number, and so\nmust be preceded by a slash character (/) when written in the FDF file\n(for example, /1.4).')\
          .done().done()\
      .required('NULL')\
          .field('FDF')\
          .name('FDF')\
          .type('dictionary')\
          .comment('(Required) The FDF dictionary for this file (see Table 8.69).')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFDictionary', 'Dictionary', 'Entries in the FDF dictionary')\
      .optional()\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Optional) The source file or target file: the PDF document file that\nthis FDF file was exported from or is intended to be imported into.')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('array')\
          .comment('(Optional) An array of two strings constituting a file identifier (see\nSection 9.3, "File Identifiers") for the source or target file designated\nby F, taken from the ID entry in the file\'s trailer dictionary (see Sec-\ntion 3.4.4, "File Trailer").')\
          .done().done()\
      .optional()\
          .field('Fields')\
          .name('Fields')\
          .type('array')\
          .comment('(Optional) An array of FDF field dictionaries (see "FDF Fields" on\npage 564) describing the root fields (those with no ancestors in\nthe field hierarchy) to be exported or imported. This entry and\nthe Pages entry may not both be present.')\
          .done().done()\
      .optional()\
          .field('Status')\
          .name('Status')\
          .type('string')\
          .comment('(Optional) A status string to be displayed indicating the result of an\naction, typically a submit-form action (see "Submit-Form Actions"\non page 550). The string is encoded with PDFDocEncoding. (See\nimplementation note 91 in Appendix H.) This entry and the Pages\nentry may not both be present.')\
          .done().done()\
      .optional()\
          .field('Pages')\
          .name('Pages')\
          .type('array')\
          .comment('(Optional; PDF 1.3) An array of FDF page dictionaries (see "FDF\nPages" on page 566) describing new pages to be added to a PDF\ntarget document. The Fields and Status entries may not be present\ntogether with this entry.')\
          .done().done()\
      .optional()\
          .field('Encoding')\
          .name('Encoding')\
          .type('name')\
          .comment('(Optional; PDF 1.3) The encoding to be used for any FDF field\nvalue or option (V or Opt in the field dictionary; see Table 8.72 on\npage 564) that is a string and does not begin with the Unicode pre-\nfix U+FEFF. (See implementation note 92 in Appendix H.) Default\nvalue: PDFDocEncoding.')\
          .done().done()\
      .optional()\
          .field('Annots')\
          .name('Annots')\
          .type('array')\
          .comment('(Optional; PDF 1.3) An array of FDF annotation dictionaries (see\n"FDF Annotation Dictionaries" on page 568). The array can in-\nclude annotations of any of the standard types listed in Table 8.14\non page 499 except Link, Movie, Widget, PrinterMark, and TrapNet.')\
          .done().done()\
      .optional()\
          .field('Differences')\
          .name('Differences')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A stream containing all the bytes in all incre-\nmental updates made to the underlying PDF document since it was\nopened (see Section 3.4.5, "Incremental Updates"). If a submit-\nform action submitting the document to a remote server in FDF\nformat has its IncludeAppendSaves flag set (see "Submit-Form\nActions" on page 550), the contents of this stream are included in\nthe submission. This allows any digital signatures (see "Signature\nFields" on page 547) to be transmitted to the server. An incremental\nupdate is automatically performed just before the submission takes\nplace, in order to capture all changes made to the document. Note\nthat the submission always includes the full set of incremental up-\ndates back to the time the document was first opened, even if some\nof them may already have been included in intervening submissions.\nNote: Although a Fields or Annots entry (or both) may be present\nalong with Differences, there is no guarantee that their contents will be\nconsistent with it. In particular, if Differences contains a digital signa-\nture, only the values of the form fields given in the Differences stream\ncan be considered trustworthy under that signature.')\
          .done().done()\
      .optional()\
          .field('Target')\
          .name('Target')\
          .type('string')\
          .comment('(Optional; PDF 1.4) The name of a browser frame in which the un-\nderlying PDF document is to be opened. This mimics the behavior\nof the target attribute in HTML <href> tags.')\
          .done().done()\
      .optional()\
          .field('EmbeddedFDFs')\
          .name('EmbeddedFDFs')\
          .type('array')\
          .comment('(Optional; PDF 1.4) An array of file specifications (see Section 3.10,\n"File Specifications") representing other FDF files embedded with-\nin this one (Section 3.10.3, "Embedded File Streams").')\
          .done().done()\
      .optional()\
          .field('JavaScript')\
          .name('JavaScript')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A JavaScript dictionary (see Table 8.71) defin-\ning document-level JavaScript scripts.')\
          .done().done()\
      .done()

  pdfspec.addClass('EncryptedEmbeddedFileStreamDictionary', 'Dictionary', 'Additional entry in an embedded file stream dictionary for an encrypted FDF file')\
      .optional()\
          .field('EncryptionRevision')\
          .name('EncryptionRevision')\
          .type('integer')\
          .comment('(Required if the FDF file is encrypted; PDF 1.4) The revision number of the\nFDF encryption algorithm used to encrypt the file. The only valid value\ndefined at the time of publication is 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('JavascriptDictionary', 'Dictionary', 'Entries in the JavaScript dictionary')\
      .optional()\
          .field('Before')\
          .name('Before')\
          .type('string or stream')\
          .comment('(Optional) A string or stream containing a JavaScript script to be executed\njust before the FDF file is imported.')\
          .done().done()\
      .optional()\
          .field('After')\
          .name('After')\
          .type('string or stream')\
          .comment('(Optional) A string or stream containing a JavaScript script to be executed\njust after the FDF file is imported.')\
          .done().done()\
      .optional()\
          .field('Doc')\
          .name('Doc')\
          .type('array')\
          .comment('(Optional) An array defining additional JavaScript scripts to be added to\nthose defined in the JavaScript entry of the document\'s name dictionary (see\nSection 3.6.3, "Name Dictionary"). The array contains an even number of\nelements, organized in pairs. The first element of each pair is a name and the\nsecond is a string or stream defining the script corresponding to that name.\nEach of the defined scripts will be added to those already defined in the name\ndictionary and then executed before the script defined in the Before entry is\nexecuted. As described in "JavaScript Actions" on page 556, these scripts are\nused to define JavaScript functions for use by other scripts in the document.')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFFieldDictionary', 'Dictionary', 'Entries in an FDF field dictionary')\
      .optional()\
          .field('Kids')\
          .name('Kids')\
          .type('array')\
          .comment('(Optional) An array containing the immediate children of this field.\nNote: Unlike the children of fields in a PDF file, which must be specified as indirect\nobject references, those of an FDF field may be either direct or indirect objects.')\
          .done().done()\
      .required('NULL')\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Required) The partial field name (see "Field Names" on page 532).')\
          .done().done()\
      .optional()\
          .field('V')\
          .name('V')\
          .type('(various)')\
          .comment('(Optional) The field\'s value, whose format varies depending on the field type; see\nthe descriptions of individual field types in Section 8.6.3 for further information.')\
          .done().done()\
      .optional()\
          .field('Ff')\
          .name('Ff')\
          .type('integer')\
          .comment('(Optional) A set of flags specifying various characteristics of the field (see Tables\n8.50 on page 532, 8.53 on page 538, 8.56 on page 543, and 8.58 on page 546). When\nimported into an interactive form, the value of this entry replaces that of the Ff\nentry in the form\'s corresponding field dictionary. If this field is present, the SetFf\nand ClrFf entries, if any, are ignored.')\
          .done().done()\
      .optional()\
          .field('SetFf')\
          .name('SetFf')\
          .type('integer')\
          .comment('(Optional) A set of flags to be set (turned on) in the Ff entry of the form\'s cor-\nresponding field dictionary. Bits equal to 1 in SetFf cause the corresponding bits in\nFf to be set to 1. This entry is ignored if an Ff entry is present in the FDF field\ndictionary.')\
          .done().done()\
      .optional()\
          .field('ClrFf')\
          .name('ClrFf')\
          .type('integer')\
          .comment('(Optional) A set of flags to be cleared (turned off) in the Ff entry of the form\'s cor-\nresponding field dictionary. Bits equal to 1 in ClrFf cause the corresponding bits in\nFf to be set to 0. If a SetFf entry is also present in the FDF field dictionary, it is\napplied before this entry. This entry is ignored if an Ff entry is present in the FDF\nfield dictionary.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('integer')\
          .comment('(Optional) A set of flags specifying various characteristics of the field\'s widget anno-\ntation (see Section 8.4.2, "Annotation Flags"). When imported into an interactive\nform, the value of this entry replaces that of the F entry in the form\'s corresponding\nannotation dictionary. If this field is present, the SetF and ClrF entries, if any, are\nignored.')\
          .done().done()\
      .optional()\
          .field('SetF')\
          .name('SetF')\
          .type('integer')\
          .comment('(Optional) A set of flags to be set (turned on) in the F entry of the form\'s corre-\nsponding widget annotation dictionary. Bits equal to 1 in SetF cause the corre-\nsponding bits in F to be set to 1. This entry is ignored if an F entry is present in the\nFDF field dictionary.')\
          .done().done()\
      .optional()\
          .field('ClrF')\
          .name('ClrF')\
          .type('integer')\
          .comment('(Optional) A set of flags to be cleared (turned off) in the F entry of the form\'s corre-\nsponding widget annotation dictionary. Bits equal to 1 in ClrF cause the corre-\nsponding bits in F to be set to 0. If a SetF entry is also present in the FDF field\ndictionary, it is applied before this entry. This entry is ignored if an F entry is\npresent in the FDF field dictionary.')\
          .done().done()\
      .optional()\
          .field('AP')\
          .name('AP')\
          .type('dictionary')\
          .comment('(Optional) An appearance dictionary specifying the appearance of a pushbutton\nfield (see "Pushbuttons" on page 539). The appearance dictionary\'s contents are as\nshown in Table 8.13 on page 497, except that the values of the N, R, and D entries\nmust all be streams.')\
          .done().done()\
      .optional()\
          .field('APRef')\
          .name('APRef')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3) A dictionary holding references to external PDF files contain-\ning the pages to use for the appearances of a pushbutton field. This dictionary is\nsimilar to an appearance dictionary (see Table 8.13 on page 497), except that the\nvalues of the N, R, and D entries must all be named page reference dictionaries\n(Table 8.76 on page 568). This entry is ignored if an AP entry is present.')\
          .done().done()\
      .optional()\
          .field('IF')\
          .name('IF')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.3; button fields only) An icon fit dictionary (see Table 8.73) speci-\nfying how to display a button field\'s icon within the annotation rectangle of its wid-\nget annotation.')\
          .done().done()\
      .optional()\
          .field('Opt')\
          .name('Opt')\
          .type('array')\
          .comment('(Required; choice fields only) An array of options to be presented to the user. Each\nelement of the array can take either of two forms:\n*  A text string representing one of the available options\n*  A two-element array consisting of a text string representing one of the available\n   options and a default appearance string for constructing the item\'s appearance\n   dynamically at viewing time (see "Variable Text" on page 533)')\
          .done().done()\
      .optional()\
          .field('A')\
          .name('A')\
          .type('dictionary')\
          .comment('(Optional) An action to be performed when this field\'s widget annotation is activat-\ned (see Section 8.5, "Actions").')\
          .done().done()\
      .optional()\
          .field('AA')\
          .name('AA')\
          .type('dictionary')\
          .comment('(Optional) An additional-actions dictionary defining the field\'s behavior in re-\nsponse to various trigger events (see Section 8.5.2, "Trigger Events").')\
          .done().done()\
      .done()

  pdfspec.addClass('IconFitDictionary', 'Dictionary', 'Entries in an icon fit dictionary')\
      .required('NULL')\
          .field('SW')\
          .name('SW')\
          .type('name')\
          .comment('(Required) The circumstances under which the icon should be scaled inside the annota-\ntion rectangle:\n    A    Always scale.\n    B    Scale only when the icon is bigger than the annotation rectangle.\n    S    Scale only when the icon is smaller than the annotation rectangle.\n    N    Never scale.\nDefault value: A.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The type of scaling to use:\n    A    Anamorphic scaling: scale the icon to fill the annotation rectangle exactly, with-\n         out regard to its original aspect ratio (ratio of width to height).\n    P    Proportional scaling: scale the icon to fit the width or height of the annotation\n         rectangle while maintaining the icon\'s original aspect ratio. If the required hori-\n         zontal and vertical scaling factors are different, use the smaller of the two, cen-\n         tering the icon within the annotation rectangle in the other dimension.\nDefault value: P.')\
          .done().done()\
      .required('NULL')\
          .field('A')\
          .name('A')\
          .type('array')\
          .comment('(Required) An array of two numbers between 0.0 and 1.0 indicating the fraction of left-\nover space to allocate at the left and bottom of the icon. A value of [0.0 0.0] positions the\nicon at the bottom-left corner of the annotation rectangle; a value of [0.5 0.5] centers it\nwithin the rectangle. This entry is used only if the icon is scaled proportionally. Default\nvalue: [0.5 0.5].')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFPageDictionary', 'Dictionary', 'Entries in an FDF page dictionary')\
      .required('NULL')\
          .field('Templates')\
          .name('Templates')\
          .type('array')\
          .comment('(Required) An array of FDF template dictionaries (see Table 8.75) describing the\nnamed pages that serve as templates on the page.')\
          .done().done()\
      .optional()\
          .field('Info')\
          .name('Info')\
          .type('dictionary')\
          .comment('(Optional) An FDF page information dictionary containing additional informa-\ntion about the page. At the time of publication, no entries have been defined for\nthis dictionary.')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFTemplateDictionary', 'Dictionary', 'Entries in an FDF template dictionary')\
      .required('NULL')\
          .field('TRef')\
          .name('TRef')\
          .type('dictionary')\
          .comment('(Required) A named page reference dictionary (see Table 8.76) specifying the\nlocation of the template.')\
          .done().done()\
      .optional()\
          .field('Fields')\
          .name('Fields')\
          .type('array')\
          .comment('(Optional) An array of references to FDF field dictionaries (see Table 8.72 on\npage 564) describing the root fields to be imported (those with no ancestors in\nthe field hierarchy).')\
          .done().done()\
      .optional()\
          .field('Rename')\
          .name('Rename')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether fields imported from the template may be\nrenamed in the event of name conflicts with existing fields; see below for further\ndiscussion. Default value: true.')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFNamedPageReferenceDictionary', 'Dictionary', 'Entries in an FDF named page reference dictionary')\
      .required('NULL')\
          .field('Name')\
          .name('Name')\
          .type('string')\
          .comment('(Required) The name of the referenced page.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Optional) The file containing the named page. If this key is absent, it is\nassumed that the page resides in the associated PDF file.')\
          .done().done()\
      .done()

  pdfspec.addClass('FDFFileAnnotationDictionary', 'Dictionary', 'Additional entry for annotation dictionaries in an FDF file')\
      .optional()\
          .field('Page')\
          .name('Page')\
          .type('integer')\
          .comment('(Required for annotations in FDF files) The ordinal page number on which\nthis annotation should appear, where page 0 is the first page.')\
          .done().done()\
      .done()

  pdfspec.addClass('SoundObjectDictionary', 'Dictionary', 'Additional entries specific to a sound object')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nSound for a sound object.')\
          .done().done()\
      .required('NULL')\
          .field('R')\
          .name('R')\
          .type('number')\
          .comment('(Required) The sampling rate, in samples per second.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('integer')\
          .comment('(Optional) The number of sound channels. Default value: 1. (See implementation\nnote 101 in Appendix H.)')\
          .done().done()\
      .optional()\
          .field('B')\
          .name('B')\
          .type('integer')\
          .comment('(Optional) The number of bits per sample value per channel. Default value: 8.')\
          .done().done()\
      .optional()\
          .field('E')\
          .name('E')\
          .type('name')\
          .comment('(Optional) The encoding format for the sample data:\n   Raw          Unspecified or unsigned values in the range 0 to 2B - 1\n   Signed       Twos-complement values\n   muLaw        mu-law\'encoded samples\n   ALaw         A-law\'encoded samples\nDefault value: Raw.')\
          .done().done()\
      .optional()\
          .field('CO')\
          .name('CO')\
          .type('name')\
          .comment('(Optional) The sound compression format used on the sample data. (Note that this is\nseparate from any stream compression specified by the sound object\'s Filter entry; see\nTable 3.4 on page 38 and Section 3.3, "Filters.") If this entry is absent, then no sound\ncompression has been used; the data contains sampled waveforms to be played at R\nsamples per second per channel.')\
          .done().done()\
      .optional()\
          .field('CP')\
          .name('CP')\
          .type('(various)')\
          .comment('(Optional) Optional parameters specific to the sound compression format used.\nNote: At the time of publication, no standard values have been defined for the CO and CP\nentries.')\
          .done().done()\
      .done()

  pdfspec.addClass('MovieDictionary', 'Dictionary', 'Entries in a movie dictionary')\
      .required('NULL')\
          .field('F')\
          .name('F')\
          .type('file specification')\
          .comment('(Required) A file specification identifying a self-describing movie file.\nNote: The format of a "self-describing movie file" is left unspecified, and there is\nno guarantee of portability.')\
          .done().done()\
      .optional()\
          .field('Aspect')\
          .name('Aspect')\
          .type('array')\
          .comment('(Optional) The width and height of the movie\'s bounding box, in pixels,\nspecified as [width height]. This entry should be omitted for a movie consist-\ning entirely of sound with no visible images.')\
          .done().done()\
      .optional()\
          .field('Rotate')\
          .name('Rotate')\
          .type('integer')\
          .comment('(Optional) The number of degrees by which the movie is rotated clockwise\nrelative to the page. The value must be a multiple of 90. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('Poster')\
          .name('Poster')\
          .type('boolean or stream')\
          .comment('(Optional) A flag or stream specifying whether and how to display a poster\nimage representing the movie. If this value is a stream, it contains an image\nXObject (see Section 4.8, "Images") to be displayed as the poster; if it is the\nboolean value true, the poster image should be retrieved from the movie file\nitself; if it is false, no poster should be displayed. Default value: false.')\
          .done().done()\
      .done()

  pdfspec.addClass('MovieActivationDictionary', 'Dictionary', 'Entries in a movie activation dictionary')\
      .optional()\
          .field('Start')\
          .name('Start')\
          .type('(various)')\
          .comment('(Optional) The starting time of the movie segment to be played. Movie time\nvalues are expressed in units of time based on a time scale, which defines the\nnumber of units per second; the default time scale is defined in the movie\ndata itself. The starting time is nominally a 64-bit integer, specified as follows:\n*  If it is representable as an integer (subject to the implementation limit for\n   integers, as described in Appendix C), it should be specified as such.\n*  If it is not representable as an integer, it should be specified as an 8-byte\n   string representing a 64-bit twos-complement integer, most significant\n   byte first.\n*  If it is expressed in a time scale different from that of the movie itself, it is\n   represented as an array of two values: an integer or string denoting the\n   starting time, as above, followed by an integer specifying the time scale in\n   units per second.\nIf this entry is omitted, the movie is played from the beginning.')\
          .done().done()\
      .optional()\
          .field('Duration')\
          .name('Duration')\
          .type('(various)')\
          .comment('(Optional) The duration of the movie segment to be played, specified in the\nsame form as Start. Negative values specify that the movie is to be played\nbackward. If this entry is omitted, the movie is played to the end.')\
          .done().done()\
      .optional()\
          .field('Rate')\
          .name('Rate')\
          .type('number')\
          .comment('(Optional) The initial speed at which to play the movie. If the value of this\nentry is negative, the movie is played backward with respect to Start and\nDuration. Default value: 1.0.')\
          .done().done()\
      .optional()\
          .field('Volume')\
          .name('Volume')\
          .type('number')\
          .comment('(Optional) The initial sound volume at which to play the movie, in the range\n-1.0 to 1.0. Higher values denote greater volume; negative values mute the\nsound. Default value: 1.0.')\
          .done().done()\
      .optional()\
          .field('ShowControls')\
          .name('ShowControls')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to display a movie controller bar while\nplaying the movie. Default value: false.')\
          .done().done()\
      .optional()\
          .field('Mode')\
          .name('Mode')\
          .type('name')\
          .comment('(Optional) The play mode for playing the movie:\n    Once              Play once and stop.\n    Open              Play and leave the movie controller bar open.\n    Repeat            Play repeatedly from beginning to end until stopped.\n    Palindrome        Play continuously forward and backward until stopped.\nDefault value: Once.')\
          .done().done()\
      .optional()\
          .field('Synchronous')\
          .name('Synchronous')\
          .type('boolean')\
          .comment('(Optional) A flag specifying whether to play the movie synchronously or\nasynchronously. If this value is true, the movie player will retain control until\nthe movie is completed or dismissed by the user; if false, it will return control\nto the viewer application immediately after starting the movie. Default value:\nfalse.')\
          .done().done()\
      .optional()\
          .field('FWScale')\
          .name('FWScale')\
          .type('array')\
          .comment('(Optional) The magnification (zoom) factor at which to play the movie. The\npresence of this entry implies that the movie is to be played in a floating win-\ndow; if the entry is absent, it will be played in the annotation rectangle.\nThe value of the entry is an array of two integers, [numerator denominator],\ndenoting a rational magnification factor for the movie. The final window\nsize, in pixels, is\n    (numerator / denominator) x Aspect\nwhere the value of Aspect is taken from the movie dictionary (see Table 8.79).')\
          .done().done()\
      .optional()\
          .field('FWPosition')\
          .name('FWPosition')\
          .type('array')\
          .comment('(Optional) For floating play windows, the relative position of the window on\nthe screen. The value is an array of two numbers\n    [horiz vert]\neach in the range 0.0 to 1.0, denoting the relative horizontal and vertical posi-\ntion of the movie window with respect to the screen. For example, the value\n[0.5 0.5] centers the window on the screen. Default value: [0.5 0.5].\n              CHAPTER 9')\
          .done().done()\
      .done()

  pdfspec.addClass('DocumentInformationDictionary', 'Dictionary', 'Entries in the document information dictionary')\
      .optional()\
          .field('Title')\
          .name('Title')\
          .type('text string')\
          .comment('(Optional; PDF 1.1) The document\'s title.')\
          .done().done()\
      .optional()\
          .field('Author')\
          .name('Author')\
          .type('text string')\
          .comment('(Optional) The name of the person who created the document.')\
          .done().done()\
      .optional()\
          .field('Subject')\
          .name('Subject')\
          .type('text string')\
          .comment('(Optional; PDF 1.1) The subject of the document.')\
          .done().done()\
      .optional()\
          .field('Keywords')\
          .name('Keywords')\
          .type('text string')\
          .comment('(Optional; PDF 1.1) Keywords associated with the document.')\
          .done().done()\
      .optional()\
          .field('Creator')\
          .name('Creator')\
          .type('text string')\
          .comment('(Optional) If the document was converted to PDF from another format, the\nname of the application (for example, Adobe FrameMaker(R)) that created the\noriginal document from which it was converted.')\
          .done().done()\
      .optional()\
          .field('Producer')\
          .name('Producer')\
          .type('text string')\
          .comment('(Optional) If the document was converted to PDF from another format, the\nname of the application (for example, Acrobat Distiller) that converted it to\nPDF.')\
          .done().done()\
      .optional()\
          .field('CreationDate')\
          .name('CreationDate')\
          .type('date')\
          .comment('(Optional) The date and time the document was created, in human-readable\nform (see Section 3.8.2, "Dates").')\
          .done().done()\
      .optional()\
          .field('ModDate')\
          .name('ModDate')\
          .type('date')\
          .comment('(Optional; PDF 1.1) The date and time the document was most recently\nmodified, in human-readable form (see Section 3.8.2, "Dates").')\
          .done().done()\
      .optional()\
          .field('Trapped')\
          .name('Trapped')\
          .type('name')\
          .comment('(Optional; PDF 1.3) A name object indicating whether the document has\nbeen modified to include trapping information (see Section 9.10.5, "Trap-\nping Support"):\n   True         The document has been fully trapped; no further trapping is\n                needed. (Note that this is the name True, not the boolean\n                value true.)\n   False        The document has not yet been trapped; any desired trap-\n                ping must still be done. (Note that this is the name False, not\n                the boolean value false.)\n   Unknown      Either it is unknown whether the document has been\n                trapped or it has been partly but not yet fully trapped; some\n                additional trapping may still be needed.\nDefault value: Unknown.\nThe value of this entry may be set automatically by the software creating the\ndocument\'s trapping information or may be known only to a human opera-\ntor and entered manually.')\
          .done().done()\
      .done()

  pdfspec.addClass('MetadataStreamDictionary', 'Dictionary', 'Additional entries in a metadata stream dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be Metadata\nfor a metadata stream.')\
          .done().done()\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of metadata stream that this dictionary describes; must be\nXML.')\
          .done().done()\
      .done()

  pdfspec.addClass('ComponentsWithMetadataDictionary', 'Dictionary', 'Additional entry for components having metadata')\
      .optional()\
          .field('Metadata')\
          .name('Metadata')\
          .type('stream')\
          .comment('(Optional; PDF 1.4) A metadata stream containing metadata for the component.')\
          .done().done()\
      .done()

  pdfspec.addClass('PagePieceDictionary', 'Dictionary', 'Entries in a page-piece dictionary')\
      .optional()\
          .field('[any_application_name_or_well_known_data_type]')\
          .name('[any_application_name_or_well_known_data_type]')\
          .type('dictionary')\
          .comment('()An application data dictionary (see Table 9.7).')\
          .done().done()\
      .done()

  pdfspec.addClass('ApplicationDataDictionary', 'Dictionary', 'Entries in an application data dictionary')\
      .required('NULL')\
          .field('LastModified')\
          .name('LastModified')\
          .type('date')\
          .comment('(Required) The date and time when the contents of the page or form\nwere most recently modified by this application.')\
          .done().done()\
      .optional()\
          .field('Private')\
          .name('Private')\
          .type('(any)')\
          .comment('(Optional) Any private data appropriate to the application, typically\nin the form of a dictionary.')\
          .done().done()\
      .done()

  pdfspec.addClass('StructureTreeRootDictionary', 'Dictionary', 'Entries in the structure tree root')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must\nbe StructTreeRoot for a structure tree root.')\
          .done().done()\
      .optional()\
          .field('K')\
          .name('K')\
          .type('dictionary or array')\
          .comment('(Optional) The immediate child or children of the structure tree root in\nthe structure hierarchy. The value may be either a dictionary represent-\ning a single structure element or an array of such dictionaries.')\
          .done().done()\
      .optional()\
          .field('IDTree')\
          .name('IDTree')\
          .type('name tree')\
          .comment('(Required if any structure elements have element identifiers) A name tree\nthat maps element identifiers (see Table 9.10) to the structure elements\nthey denote.')\
          .done().done()\
      .optional()\
          .field('ParentTree')\
          .name('ParentTree')\
          .type('number tree')\
          .comment('(Required if any structure element contains PDF objects or marked-content\nsequences as content items) A number tree (see Section 3.8.5, "Number\nTrees") used in finding the structure elements to which content items\nbelong. Each integer key in the number tree corresponds to a single page\nof the document or to an individual object (such as an annotation or an\nXObject) that is a content item in its own right. The integer key is given\nas the value of the StructParent or StructParents entry in that object (see\n"Finding Structure Elements from Content Items" on page 600). The\nform of the associated value depends on the nature of the object:\n*  For an object that is a content item in its own right, the value is an in-\n   direct reference to the object\'s parent element (the structure element\n   that contains it as a content item).\n*  For a page object or content stream containing marked-content\n   sequences that are content items, the value is an array of references to\n   the parent elements of those marked-content sequences.\nSee "Finding Structure Elements from Content Items" on page 600 for\nfurther discussion.')\
          .done().done()\
      .optional()\
          .field('ParentTreeNextKey')\
          .name('ParentTreeNextKey')\
          .type('integer')\
          .comment('(Optional) An integer greater than any key in the parent tree, to be used\nas a key for the next entry added to the tree.')\
          .done().done()\
      .optional()\
          .field('RoleMap')\
          .name('RoleMap')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping the names of structure types used in\nthe document to their approximate equivalents in the set of standard\nstructure types (see Section 9.7.4, "Standard Structure Types").')\
          .done().done()\
      .optional()\
          .field('ClassMap')\
          .name('ClassMap')\
          .type('dictionary')\
          .comment('(Optional) A dictionary mapping name objects designating attribute\nclasses to the corresponding attribute objects or arrays of attribute ob-\njects (see "Attribute Classes" on page 605).')\
          .done().done()\
      .done()

  pdfspec.addClass('StructureElementDictionary', 'Dictionary', 'Entries in a structure element dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if\npresent, must be StructElem for a structure element.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The structure type, a name object identifying the nature of the\nstructure element and its role within the document, such as a chapter,\nparagraph, or footnote (see Section 9.6.2, "Structure Types"). Names of\nstructure types must conform to the guidelines described in Appendix E.')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('dictionary')\
          .comment('(Required; must be an indirect reference) The structure element that is the\nimmediate parent of this one in the structure hierarchy.')\
          .done().done()\
      .optional()\
          .field('ID')\
          .name('ID')\
          .type('string')\
          .comment('(Optional) The element identifier, a string designating this structure\nelement. The string must be unique among all elements in the docu-\nment\'s structure hierarchy. The IDTree entry in the structure tree root\n(see Table 9.9) defines the correspondence between element identifiers\nand the structure elements they denote.')\
          .done().done()\
      .optional()\
          .field('Pg')\
          .name('Pg')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) A page object representing a\npage on which some or all of the content items designated by the K entry\nare rendered.')\
          .done().done()\
      .optional()\
          .field('K')\
          .name('K')\
          .type('(various)')\
          .comment('(Optional) The contents of this structure element, which may consist of\none or more marked-content sequences, PDF objects, and other struc-\nture elements. The value of this entry may be any of the following:\n*  An integer marked-content identifier denoting a marked-content\n   sequence\n*  A marked-content reference dictionary denoting a marked-content\n   sequence\n*  An object reference dictionary denoting a PDF object\n*  A structure element dictionary denoting another structure element\n*  An array, each of whose elements is one of the objects listed above\nSee Section 9.6.3, "Structure Content" for further discussion of each of\nthese forms of representation.')\
          .done().done()\
      .optional()\
          .field('A')\
          .name('A')\
          .type('(various)')\
          .comment('(Optional) The attribute object or objects, if any, associated with this\nstructure element. Each attribute object is either a dictionary or a\nstream; the value of this entry may be either a single attribute object or\nan array of such objects together with their revision numbers (see\nSection 9.6.4, "Structure Attributes," and "Attribute Revision Numbers"\non page 606).')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('name or array')\
          .comment('(Optional) The attribute class or classes, if any, to which this structure\nelement belongs. The value of this entry may be either a single class\nname or an array of class names together with their revision numbers\n(see "Attribute Classes" on page 605 and "Attribute Revision Numbers"\non page 606).\nNote: If both the A and C entries are present and a given attribute is speci-\nfied by both, the one specified by the A entry takes precedence.')\
          .done().done()\
      .optional()\
          .field('R')\
          .name('R')\
          .type('integer')\
          .comment('(Optional) The current revision number of this structure element (see\n"Attribute Revision Numbers" on page 606). The value must be a non-\nnegative integer. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Optional) The title of the structure element, a text string representing it\nin human-readable form. The title should characterize the specific struc-\nture element, such as Chapter 1, rather than merely a generic element\ntype, such as Chapter.')\
          .done().done()\
      .optional()\
          .field('Lang')\
          .name('Lang')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) A language identifier specifying the natural language\nfor all text in the structure element except where overridden by language\nspecifications for nested structure elements or marked content (see Sec-\ntion 9.8.1, "Natural Language Specification"). If this entry is absent, the\nlanguage (if any) specified in the document catalog applies.')\
          .done().done()\
      .optional()\
          .field('Alt')\
          .name('Alt')\
          .type('text string')\
          .comment('(Optional) An alternate description of the structure element and its\nchildren in human-readable form, useful when extracting the docu-\nment\'s contents in support of accessibility to disabled users or for other\npurposes (see Section 9.8.2, "Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('ActualText')\
          .name('ActualText')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) Text that is an exact replacement for the structure\nelement and its children. This replacement text (which should apply to\nas small a piece of content as possible) is useful when extracting the doc-\nument\'s contents in support of accessibility to disabled users or for other\npurposes (see Section 9.8.3, "Replacement Text").')\
          .done().done()\
      .done()

  pdfspec.addClass('MarkedContentReferenceDictionary', 'Dictionary', 'Entries in a marked-content reference dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be MCR\nfor a marked-content reference.')\
          .done().done()\
      .optional()\
          .field('Pg')\
          .name('Pg')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) The page object representing the page on\nwhich the graphics objects in the marked-content sequence are rendered. This\nentry overrides any Pg entry in the structure element containing the marked-\ncontent reference; it is required if the structure element has no such entry.')\
          .done().done()\
      .optional()\
          .field('Stm')\
          .name('Stm')\
          .type('stream')\
          .comment('(Optional; must be an indirect reference) The content stream containing the\nmarked-content sequence. This entry is needed only if the marked-content\nsequence resides in some other content stream associated with the page-for\nexample, in a form XObject (see Section 4.9, "Form XObjects") or an annota-\ntion\'s appearance stream (Section 8.4.4, "Appearance Streams"). Default value:\nthe content stream of the page identified by Pg.')\
          .done().done()\
      .optional()\
          .field('StmOwn')\
          .name('StmOwn')\
          .type('(any)')\
          .comment('(Optional; must be an indirect reference) The PDF object owning the stream\nidentified by Stm-for example, the annotation to which an appearance stream\nbelongs.')\
          .done().done()\
      .required('NULL')\
          .field('MCID')\
          .name('MCID')\
          .type('integer')\
          .comment('(Required) The marked-content identifier of the marked-content sequence with-\nin its content stream.')\
          .done().done()\
      .done()

  pdfspec.addClass('ObjectReferenceDictionary', 'Dictionary', 'Entries in an object reference dictionary')\
      .required('NULL')\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Required) The type of PDF object that this dictionary describes; must be OBJR for an\nobject reference.')\
          .done().done()\
      .optional()\
          .field('Pg')\
          .name('Pg')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) The page object representing the page on\nwhich the object is rendered. This entry overrides any Pg entry in the structure ele-\nment containing the object reference; it is required if the structure element has no such\nentry.')\
          .done().done()\
      .optional()\
          .field('Obj')\
          .name('Obj')\
          .type('(any)')\
          .comment('(Required; must be an indirect reference) The referenced object.')\
          .done().done()\
      .done()

  pdfspec.addClass('StructureElementAccessDictionary', 'Dictionary', 'Additional dictionary entries for structure element access')\
      .optional()\
          .field('StructParent')\
          .name('StructParent')\
          .type('integer')\
          .comment('(Required for all objects that are structural content items; PDF 1.3) The integer key\nof this object\'s entry in the structural parent tree.')\
          .done().done()\
      .optional()\
          .field('StructParents')\
          .name('StructParents')\
          .type('integer')\
          .comment('(Required for all content streams containing marked-content sequences that are\nstructural content items; PDF 1.3) The integer key of this object\'s entry in the\nstructural parent tree.\nNote: At most one of these two entries may be present in a given object. An object\ncan be either a content item in its entirety or a container for marked-content\nsequences that are content items, but not both.')\
          .done().done()\
      .done()

  pdfspec.addClass('AttributeObjectDictionary', 'Dictionary', 'Entry common to all attribute objects')\
      .required('NULL')\
          .field('O')\
          .name('O')\
          .type('name')\
          .comment('(Required) The name of the application or plug-in extension owning the attribute data.\nThe name must conform to the guidelines described in Appendix E.')\
          .done().done()\
      .done()

  pdfspec.addClass('MarkInformationDictionary', 'Dictionary', 'Entry in the mark information dictionary')\
      .optional()\
          .field('Marked')\
          .name('Marked')\
          .type('boolean')\
          .comment('(Optional) A flag indicating whether the document conforms to Tagged PDF\nconventions. Default value: false.')\
          .done().done()\
      .done()

  pdfspec.addClass('ArtifactsDictionary', 'Dictionary', 'Property list entries for artifacts')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of artifact that this property list describes; if present, must\nbe one of the names Pagination, Layout, or Page.')\
          .done().done()\
      .optional()\
          .field('BBox')\
          .name('BBox')\
          .type('rectangle')\
          .comment('(Optional) An array of four numbers in default user space units giving the coor-\ndinates of the left, bottom, right, and top edges, respectively, of the artifact\'s\nbounding box (the rectangle that completely encloses its visible extent).')\
          .done().done()\
      .optional()\
          .field('Attached')\
          .name('Attached')\
          .type('array')\
          .comment('(Optional; pagination artifacts only) An array of name objects containing one to\nfour of the names Top, Bottom, Left, and Right, specifying the edges of the page, if\nany, to which the artifact is logically attached. Page edges are defined by the\npage\'s crop box (see Section 9.10.1, "Page Boundaries"). The ordering of names\nwithin the array is immaterial. Including both Left and Right or both Top and\nBottom indicates a full-width or full-height artifact, respectively.')\
          .done().done()\
      .done()

  pdfspec.addClass('StandardStructureDictionary', 'Dictionary', 'Standard layout attributes common to all standard structure types')\
      .optional()\
          .field('Placement')\
          .name('Placement')\
          .type('name')\
          .comment('(Optional) The positioning of the element with respect to the enclosing refer-\nence area and other content:\n    Block        Stacked in the block-progression direction within an enclos-\n                 ing reference area or parent BLSE.\n    Inline       Packed in the inline-progression direction within an enclos-\n                 ing BLSE.\n    Before       Placed so that the before edge of the element\'s allocation rec-\n                 tangle (see "Content and Allocation Rectangles" on page\n                 648) coincides with that of the nearest enclosing reference\n                 area. The element may float, if necessary, to achieve the speci-\n                 fied placement (see note below). The element is treated as a\n                 block occupying the full extent of the enclosing reference\n                 area in the inline direction; other content is stacked so as to\n                 begin at the after edge of the element\'s allocation rectangle.\n    Start        Placed so that the start edge of the element\'s allocation rec-\n                 tangle (see "Content and Allocation Rectangles" on page\n                 648) coincides with that of the nearest enclosing reference\n                 area. The element may float, if necessary, to achieve the speci-\n                 fied placement (see note below). Other content that would\n                 intrude into the element\'s allocation rectangle is laid out as a\n                 runaround.\n    End          Placed so that the end edge of the element\'s allocation rec-\n                 tangle (see "Content and Allocation Rectangles" on page\n                 648) coincides with that of the nearest enclosing reference\n                 area. The element may float, if necessary, to achieve the speci-\n                 fied placement (see note below). Other content that would\n                 intrude into the element\'s allocation rectangle is laid out as a\n                 runaround.\nWhen applied to an ILSE, any value except Inline causes the element to be\ntreated as a BLSE instead. Default value: Inline.\nNote: Elements with Placement values of Before, Start, or End are removed from\nthe normal stacking or packing process and allowed to "float" to the specified\nedge of the enclosing reference area or parent BLSE. Multiple such floating ele-\nments may be positioned adjacent to one another against the specified edge of the\nreference area, or placed serially against the edge, in the order encountered.\n     Complex cases such as floating elements that interfere with each other or do not\n     fit on the same page may be handled differently by different layout applications;\n     Tagged PDF merely identifies the elements as floating and indicates their desired\n     placement.')\
          .done().done()\
      .optional()\
          .field('WritingMode')\
          .name('WritingMode')\
          .type('name')\
          .comment('(Optional) The directions of layout progression for packing of ILSEs (inline\nprogression) and stacking of BLSEs (block progression):\n    LrTb         Inline progression from left to right; block progression from\n                 top to bottom. This is the typical writing mode for Western\n                 writing systems.\n    RlTb         Inline progression from right to left; block progression from\n                 top to bottom. This is the typical writing mode for Arabic\n                 and Hebrew writing systems.\n    TbRl         Inline progression from top to bottom; block progression\n                 from right to left. This is the typical writing mode for Chi-\n                 nese and Japanese writing systems.\nThe specified layout directions apply to the given structure element and all of\nits descendants to any level of nesting. Default value: LrTb.\nFor elements that produce multiple columns, the writing mode defines the\ndirection of column progression within the reference area: the inline direc-\ntion determines the stacking direction for columns and the default flow\norder of text from column to column. For tables, the writing mode controls\nthe layout of rows and columns: table rows (structure type TR) are stacked\nin the block direction, cells within a row (structure type TD) in the inline\ndirection.\nNote: The inline-progression direction specified by the writing mode is subject to\nlocal override within the text being laid out, as described in Unicode Standard\nAnnex #9, The Bidirectional Algorithm, available from the Unicode Consor-\ntium (see the Bibliography).')\
          .done().done()\
      .done()

  pdfspec.addClass('BlockLevelStructureElementsDictionary', 'Dictionary', 'Additional standard layout attributes specific to block-level structure elements')\
      .optional()\
          .field('SpaceBefore')\
          .name('SpaceBefore')\
          .type('number')\
          .comment('(Optional) The amount of extra space preceding the before edge of the BLSE,\nmeasured in default user space units in the block-progression direction. This\nvalue is added to any adjustments induced by the LineHeight attributes of\nILSEs within the first line of the BLSE (see "Layout Attributes for ILSEs" on\npage 646). If the preceding BLSE has a SpaceAfter attribute, the greater of the\ntwo attribute values is used. Default value: 0.\nNote: This attribute is disregarded for the first BLSE placed in a given reference\narea.')\
          .done().done()\
      .optional()\
          .field('SpaceAfter')\
          .name('SpaceAfter')\
          .type('number')\
          .comment('(Optional) The amount of extra space following the after edge of the BLSE,\nmeasured in default user space units in the block-progression direction. This\nvalue is added to any adjustments induced by the LineHeight attributes of\nILSEs within the last line of the BLSE (see "Layout Attributes for ILSEs" on\npage 646). If the following BLSE has a SpaceBefore attribute, the greater of\nthe two attribute values is used. Default value: 0.\nNote: This attribute is disregarded for the last BLSE placed in a given reference\narea.')\
          .done().done()\
      .optional()\
          .field('StartIndent')\
          .name('StartIndent')\
          .type('number')\
          .comment('(Optional) The distance from the start edge of the reference area to that of the\nBLSE, measured in default user space units in the inline-progression direc-\ntion. This attribute applies only to structure elements with a Placement\nattribute of Block or Start (see "General Layout Attributes" on page 640); it is\ndisregarded for those with other Placement values. Default value: 0.\nNote: A negative value for this attribute places the start edge of the BLSE out-\nside that of the reference area. The results are implementation-dependent and\nmay not be supported by all Tagged PDF consumer applications or export\nformats.\nNote: If a structure element with a StartIndent attribute is placed adjacent to a\nfloating element with a Placement attribute of Start, the actual value used for\nthe element\'s starting indent will be its own StartIndent attribute or the inline\nextent of the adjacent floating element, whichever is greater. This value may\nthen be further adjusted by the element\'s TextIndent attribute, if any.')\
          .done().done()\
      .optional()\
          .field('EndIndent')\
          .name('EndIndent')\
          .type('number')\
          .comment('(Optional) The distance from the end edge of the BLSE to that of the ref-\nerence area, measured in default user space units in the inline-progression\ndirection. This attribute applies only to structure elements with a Placement\nattribute of Block or End (see "General Layout Attributes" on page 640); it is\ndisregarded for those with other Placement values. Default value: 0.\nNote: A negative value for this attribute places the end edge of the BLSE outside\nthat of the reference area. The results are implementation-dependent and may\nnot be supported by all Tagged PDF consumer applications or export formats.\nNote: If a structure element with an EndIndent attribute is placed adjacent to a\nfloating element with a Placement attribute of End, the actual value used for the\nelement\'s ending indent will be its own EndIndent attribute or the inline extent\nof the adjacent floating element, whichever is greater.')\
          .done().done()\
      .optional()\
          .field('TextIndent')\
          .name('TextIndent')\
          .type('number')\
          .comment('(Optional; applies only to some BLSEs, as described below) The additional\ndistance, measured in default user space units in the inline-progression\ndirection, from the start edge of the BLSE, as specified by StartIndent\n(above), to that of the first line of text. A negative value indicates a hanging\nindent. Default value: 0.\nThis attribute applies only to paragraphlike BLSEs and those of structure\ntypes Lbl (Label), LBody (List body), TH (Table header), and TD (Table data),\nprovided that they contain content other than nested BLSEs.')\
          .done().done()\
      .optional()\
          .field('TextAlign')\
          .name('TextAlign')\
          .type('name')\
          .comment('(Optional; applies only to BLSEs containing text) The alignment, in the inline-\nprogression direction, of text and other content within lines of the BLSE:\nStart        Aligned with the start edge.\nCenter       Centered between the start and end edges.\nEnd          Aligned with the end edge.\nJustify      Aligned with both the start and end edges, with internal\n        spacing within each line expanded, if necessary, to achieve\n        such alignment. The last (or only) line is aligned with the\n        start edge only, as for Start (above).\n        Default value: Start.')\
          .done().done()\
      .optional()\
          .field('BBox')\
          .name('BBox')\
          .type('rectangle')\
          .comment('(Illustrations and tables only; required if the element appears in its entirety on a\nsingle page) An array of four numbers in default user space units giving the\ncoordinates of the left, bottom, right, and top edges, respectively, of the ele-\nment\'s bounding box (the rectangle that completely encloses its visible con-\ntent). This attribute applies only to elements of structure type Figure,\nFormula, Form, or Table.')\
          .done().done()\
      .optional()\
          .field('Width')\
          .name('Width')\
          .type('number or name')\
          .comment('(Optional; illustrations, tables, table headers, and table cells only; strongly\nrecommended for table cells) The desired width of the element\'s content\nrectangle (see "Content and Allocation Rectangles" on page 648), measured\nin default user space units in the inline-progression direction. This attribute\napplies only to elements of structure type Figure, Formula, Form, Table, TH\n(Table header), or TD (Table data).\nThe name Auto in place of a numeric value indicates that no specific width\nconstraint is to be imposed; the element\'s width is determined by the intrin-\nsic width of its content. Default value: Auto.')\
          .done().done()\
      .optional()\
          .field('Height')\
          .name('Height')\
          .type('number or name')\
          .comment('(Optional; illustrations, tables, table headers, and table cells only) The desired\nheight of the element\'s content rectangle (see "Content and Allocation\nRectangles" on page 648), measured in default user space units in the block-\nprogression direction. This attribute applies only to elements of structure\ntype Figure, Formula, Form, Table, TH (Table header), or TD (Table data).\nThe name Auto in place of a numeric value indicates that no specific height\nconstraint is to be imposed; the element\'s height is determined by the intrin-\nsic height of its content. Default value: Auto.')\
          .done().done()\
      .optional()\
          .field('BlockAlign')\
          .name('BlockAlign')\
          .type('name')\
          .comment('(Optional; table cells only) The alignment, in the block-progression direction,\nof content within the table cell:\n     Before        Before edge of the first child\'s allocation rectangle aligned\n                   with that of the table cell\'s content rectangle.\n     Middle        Children centered within the table cell, so that the distance\n                   between the before edge of the first child\'s allocation rec-\n                   tangle and that of the table cell\'s content rectangle is the same\n                   as the distance between the after edge of the last child\'s allo-\n                   cation rectangle and that of the table cell\'s content rectangle.\n     After         After edge of the last child\'s allocation rectangle aligned with\n                   that of the table cell\'s content rectangle.\n     Justify       Children aligned with both the before and after edges of the\n                   table cell\'s content rectangle. The first child is placed as\n                   described above for Before and the last child as described for\n                   After, with equal spacing between the children. If there is only\n                   one child, it is aligned with the before edge only, as for Before.\n This attribute applies only to elements of structure type TH (Table header) or\n TD (Table data), and controls the placement of all BLSEs that are children of\n the given element. The table cell\'s content rectangle (see "Content and Allo-\n cation Rectangles" on page 648) becomes the reference area for all of its\n descendants. Default value: Before.')\
          .done().done()\
      .optional()\
          .field('InlineAlign')\
          .name('InlineAlign')\
          .type('name')\
          .comment('(Optional; table cells only) The alignment, in the inline-progression direction,\nof content within the table cell:\n   Start         Start edge of each child\'s allocation rectangle aligned with\n                 that of the table cell\'s content rectangle\n   Center        Each child centered within the table cell, so that the distance\n                 between the start edges of the child\'s allocation rectangle and\n                 the table cell\'s content rectangle is the same as the distance\n                 between their end edges\n   End           End edge of each child\'s allocation rectangle aligned with\n                 that of the table cell\'s content rectangle\nThis attribute applies only to elements of structure type TH (Table header) or\nTD (Table data), and controls the placement of all BLSEs that are children of\nthe given element. The table cell\'s content rectangle (see "Content and Allo-\ncation Rectangles" on page 648) becomes the reference area for all of its\ndescendants. Default value: Start.')\
          .done().done()\
      .done()

  pdfspec.addClass('InlineLevelStructureElementsDictionary', 'Dictionary', 'Standard layout attributes specific to inline-level structure elements')\
      .optional()\
          .field('LineHeight')\
          .name('LineHeight')\
          .type('number or name')\
          .comment('(Optional) The element\'s preferred height, measured in default user space\nunits in the block-progression direction. The height of a line is deter-\nmined by the largest LineHeight value for any complete or partial ILSE\nthat it contains.\nThe name Normal or Auto in place of a numeric value indicates that no\nspecific height constraint is to be imposed; the element\'s height is set to a\nreasonable value based on the content\'s font size:\n    Normal         Adjust the line height to include any nonzero value\n                   specified for BaselineShift (see below).\n    Auto           Do not adjust for the value of BaselineShift.\nDefault value: Normal.')\
          .done().done()\
      .done()

  pdfspec.addClass('ListAttributeDictionary', 'Dictionary', 'Standard list attribute')\
      .optional()\
          .field('ListNumbering')\
          .name('ListNumbering')\
          .type('name')\
          .comment('(Optional) The numbering system used to generate the content of the Lbl (Label)\nelements in an autonumbered list, or the symbol used to identify each item in an\nunnumbered list:\n    None              No autonumbering; Lbl elements (if present) contain arbi-\n                      trary text not subject to any numbering scheme\n    Disc              Solid circular bullet\n    Circle            Open circular bullet\n    Square            Solid square bullet\n    Decimal           Decimal arabic numerals (1\'9, 10\'99, ...)\n    UpperRoman        Uppercase roman numerals (I, II, III, IV, ...)\n    LowerRoman        Lowercase roman numerals (i, ii, iii, iv, ...)\n    UpperAlpha        Uppercase letters (A, B, C, ...)\n    LowerAlpha        Lowercase letters (a, b, c, ...)\nDefault value: None.\nNote: The alphabet used for UpperAlpha and LowerAlpha is determined by the pre-\nvailing Lang entry (see Section 9.8.1, "Natural Language Specification").\nNote: The set of possible values may be expanded as Unicode identifies additional\nnumbering systems.')\
          .done().done()\
      .done()

  pdfspec.addClass('TableAttributesDictionary', 'Dictionary', 'Standard table attributes')\
      .optional()\
          .field('RowSpan')\
          .name('RowSpan')\
          .type('integer')\
          .comment('(Optional) The number of rows in the enclosing table that are spanned by the\ncell. The cell expands by adding rows in the block-progression direction speci-\nfied by the table\'s WritingMode attribute. Default value: 1.')\
          .done().done()\
      .optional()\
          .field('ColSpan')\
          .name('ColSpan')\
          .type('integer')\
          .comment('(Optional) The number of columns in the enclosing table that are spanned by\nthe cell. The cell expands by adding columns in the inline-progression direction\nspecified by the table\'s WritingMode attribute. Default value: 1.')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCaptureInformationDictionary', 'Dictionary', 'Entries in the Web Capture information dictionary')\
      .required('NULL')\
          .field('V')\
          .name('V')\
          .type('number')\
          .comment('(Required) The Web Capture version number. For PDF 1.3, the version number is 1.0.\nNote: This value is a single real number, not a major and minor version number. Thus, for\nexample, a version number of 1.2 would be considered greater than 1.15.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('array')\
          .comment('(Optional) An array of indirect references to Web Capture command dictionaries (see\n"Command Dictionaries" on page 672) describing commands that were used in building\nthe PDF file. The commands appear in the array in the order in which they were executed\nin building the file.')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCaptureDictionary', 'Dictionary', 'Entries common to all Web Capture content sets')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes; if present, must be\nSpiderContentSet for a Web Capture content set.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The subtype of content set that this dictionary describes:\n   SPS     ("Spider page set") A page set\n   SIS     ("Spider image set") An image set')\
          .done().done()\
      .required('NULL')\
          .field('ID')\
          .name('ID')\
          .type('string')\
          .comment('(Required) The digital identifier of the content set (see "Digital Identifiers" on page\n664). If the content set has been located via the URLS name tree, this allows its related\nentry in the IDS name tree to be found.')\
          .done().done()\
      .required('NULL')\
          .field('O')\
          .name('O')\
          .type('array')\
          .comment('(Required) An array of indirect references to the objects belonging to the content set.\nThe order of objects in the array is undefined in general, but may be restricted by spe-\ncific content set subtypes.')\
          .done().done()\
      .required('NULL')\
          .field('SI')\
          .name('SI')\
          .type('dictionary or array')\
          .comment('(Required) A source information dictionary (see Section 9.9.4, "Source Information"),\nor an array of such dictionaries, describing the sources from which the objects belong-\ning to the content set were created.')\
          .done().done()\
      .optional()\
          .field('CT')\
          .name('CT')\
          .type('string')\
          .comment('(Optional) The content type, a string characterizing the source from which the objects\nbelonging to the content set were created. The string should conform to the content\ntype specification described in Internet RFC 2045, Multipurpose Internet Mail Exten-\nsions (MIME) Part One: Format of Internet Message Bodies (see the Bibliography). For\nexample, for a page set consisting of a group of PDF pages created from an HTML file,\nthe content type would be text/html.')\
          .done().done()\
      .optional()\
          .field('TS')\
          .name('TS')\
          .type('date')\
          .comment('(Optional) A time stamp giving the date and time at which the content set was created.')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCapturePageSetDictionary', 'Dictionary', 'Additional entries specific to a Web Capture page set')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The subtype of content set that this dictionary describes; must be SPS\n("Spider page set") for a page set.')\
          .done().done()\
      .optional()\
          .field('T')\
          .name('T')\
          .type('text string')\
          .comment('(Optional) The title of the page set, a text string representing it in human-readable\nform.')\
          .done().done()\
      .optional()\
          .field('TID')\
          .name('TID')\
          .type('string')\
          .comment('(Optional) A text identifier generated from the text of the page set, as described in\n"Digital Identifiers" on page 664.')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCaptureImageSetDictionary', 'Dictionary', 'Additional entries specific to a Web Capture image set')\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The subtype of content set that this dictionary describes; must be SIS ("Spider\nimage set") for an image set.')\
          .done().done()\
      .required('NULL')\
          .field('R')\
          .name('R')\
          .type('integer or array')\
          .comment('(Required) The reference counts (see below) for the image XObjects belonging to the\nimage set. For an image set containing a single XObject, the value is simply the integer\nreference count for that XObject. If the image set contains multiple XObjects, the value is\nan array of reference counts parallel to the O array (see Table 9.33 on page 668); that is,\neach element in the R array holds the reference count for the image XObject at the corre-\nsponding position in the O array.')\
          .done().done()\
      .done()

  pdfspec.addClass('SourceInformationDictionary', 'Dictionary', 'Entries in a source information dictionary')\
      .required('NULL')\
          .field('AU')\
          .name('AU')\
          .type('string or dictionary')\
          .comment('(Required) A string or URL alias dictionary (see "URL Alias Dictionaries," below)\nidentifying the URLs from which the source data was retrieved.')\
          .done().done()\
      .optional()\
          .field('TS')\
          .name('TS')\
          .type('date')\
          .comment('(Optional) A time stamp giving the most recent date and time at which the content\nset\'s contents were known to be up to date with the source data.')\
          .done().done()\
      .optional()\
          .field('E')\
          .name('E')\
          .type('date')\
          .comment('(Optional) An expiration stamp giving the date and time at which the content set\'s\ncontents should be considered out of date with the source data.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('integer')\
          .comment('(Optional) A code indicating the type of form submission, if any, by which the source\ndata was accessed (see "Submit-Form Actions" on page 550):\n    0    Not accessed via a form submission\n    1    Accessed via an HTTP GET request\n    2    Accessed via an HTTP POST request\nThis entry should be present only in source information dictionaries associated with\npage sets. Default value: 0.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('dictionary')\
          .comment('(Optional; must be an indirect reference) A command dictionary (see "Command Dic-\ntionaries" on page 672) describing the command that caused the source data to be\nretrieved. This entry should be present only in source information dictionaries associ-\nated with page sets.')\
          .done().done()\
      .done()

  pdfspec.addClass('URLAliasDictionary', 'Dictionary', 'Entries in a URL alias dictionary')\
      .required('NULL')\
          .field('U')\
          .name('U')\
          .type('string')\
          .comment('(Required) The destination URL to which all of the chains specified by the C entry lead.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('array')\
          .comment('(Optional) An array of one or more arrays of strings, each representing a chain of URLs\nleading to the common destination specified by U.')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCaptureCommandDictionary', 'Dictionary', 'Entries in a Web Capture command dictionary')\
      .required('NULL')\
          .field('URL')\
          .name('URL')\
          .type('string')\
          .comment('(Required) The initial URL from which source data was requested.')\
          .done().done()\
      .optional()\
          .field('L')\
          .name('L')\
          .type('integer')\
          .comment('(Optional) The number of levels of pages retrieved from the initial URL. Default\nvalue: 1.')\
          .done().done()\
      .optional()\
          .field('F')\
          .name('F')\
          .type('integer')\
          .comment('(Optional) A set of flags specifying various characteristics of the command (see\nTable 9.39). Default value: 0.')\
          .done().done()\
      .optional()\
          .field('P')\
          .name('P')\
          .type('string or stream')\
          .comment('(Optional) Data that was posted to the URL.')\
          .done().done()\
      .optional()\
          .field('CT')\
          .name('CT')\
          .type('string')\
          .comment('(Optional) A content type describing the data posted to the URL. Default value:\napplication/x-www-form-urlencoded.')\
          .done().done()\
      .optional()\
          .field('H')\
          .name('H')\
          .type('string')\
          .comment('(Optional) Additional HTTP request headers sent to the URL.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('dictionary')\
          .comment('(Optional) A command settings dictionary containing settings used in the con-\nversion process (see "Command Settings" on page 674).')\
          .done().done()\
      .done()

  pdfspec.addClass('WebCaptureCommandSettingsDictionary', 'Dictionary', 'Entries in a Web Capture command settings dictionary')\
      .optional()\
          .field('G')\
          .name('G')\
          .type('dictionary')\
          .comment('(Optional) A dictionary containing global conversion engine settings relevant to all con-\nversion engines. If this key is absent, default settings will be used.')\
          .done().done()\
      .optional()\
          .field('C')\
          .name('C')\
          .type('dictionary')\
          .comment('(Optional) Settings for specific conversion engines. Each key in this dictionary is the\ninternal name of a conversion engine (see below). The associated value is a dictionary\ncontaining the settings associated with that conversion engine. If the settings for a par-\nticular conversion engine are not found in the dictionary, default settings will be used.')\
          .done().done()\
      .done()

  pdfspec.addClass('BoxColorInformationDictionary', 'Dictionary', 'Entries in a box color information dictionary')\
      .optional()\
          .field('CropBox')\
          .name('CropBox')\
          .type('dictionary')\
          .comment('(Optional) A box style dictionary (see Table 9.42) specifying the visual characteris-\ntics for displaying guidelines for the page\'s crop box. This entry is ignored if no crop\nbox is defined in the page object.')\
          .done().done()\
      .optional()\
          .field('BleedBox')\
          .name('BleedBox')\
          .type('dictionary')\
          .comment('(Optional) A box style dictionary (see Table 9.42) specifying the visual characteris-\ntics for displaying guidelines for the page\'s bleed box. This entry is ignored if no\nbleed box is defined in the page object.')\
          .done().done()\
      .optional()\
          .field('TrimBox')\
          .name('TrimBox')\
          .type('dictionary')\
          .comment('(Optional) A box style dictionary (see Table 9.42) specifying the visual characteris-\ntics for displaying guidelines for the page\'s trim box. This entry is ignored if no trim\nbox is defined in the page object.')\
          .done().done()\
      .optional()\
          .field('ArtBox')\
          .name('ArtBox')\
          .type('dictionary')\
          .comment('(Optional) A box style dictionary (see Table 9.42) specifying the visual characteris-\ntics for displaying guidelines for the page\'s art box. This entry is ignored if no art\nbox is defined in the page object.')\
          .done().done()\
      .done()

  pdfspec.addClass('BoxStyleDictionary', 'Dictionary', 'Entries in a box style dictionary')\
      .required('NULL')\
          .field('C')\
          .name('C')\
          .type('array')\
          .comment('(Required) An array of three numbers in the range 0.0 to 1.0, representing the com-\nponents in the DeviceRGB color space of the color to be used for displaying the\nguidelines. Default value: [0.0 0.0 0.0].')\
          .done().done()\
      .optional()\
          .field('W')\
          .name('W')\
          .type('number')\
          .comment('(Optional) The guideline width in default user space units. Default value: 1.')\
          .done().done()\
      .optional()\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Optional) The guideline style:\n    S    (Solid) A solid rectangle.\n    D    (Dashed) A dashed rectangle. The dash pattern is specified by the D entry\n         (see below).\nOther guideline styles may be defined in the future. Default value: S.')\
          .done().done()\
      .optional()\
          .field('D')\
          .name('D')\
          .type('array')\
          .comment('(Optional) A dash array defining a pattern of dashes and gaps to be used in drawing\ndashed guidelines (guideline style D above). The dash array is specified in default\nuser space units, in the same format as in the line dash pattern parameter of the\ngraphics state (see "Line Dash Pattern" on page 155). The dash phase is not speci-\nfied and is assumed to be 0. For example, a D entry of [3 2] specifies guidelines\ndrawn with 3-point dashes alternating with 2-point gaps. Default value: [3].')\
          .done().done()\
      .done()

  pdfspec.addClass('PrinterMarkAnnotationDictionary', 'Dictionary', 'Additional entries specific to a printer\'s mark annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be Printer-\nMark for a printer\'s mark annotation.')\
          .done().done()\
      .optional()\
          .field('MN')\
          .name('MN')\
          .type('name')\
          .comment('(Optional) An arbitrary name identifying the type of printer\'s mark, such as Color-\nBar or RegistrationTarget.')\
          .done().done()\
      .done()

  pdfspec.addClass('PrinterMarkFormDictionary', 'Dictionary', 'Additional entries specific to a printer\'s mark form dictionary')\
      .optional()\
          .field('MarkStyle')\
          .name('MarkStyle')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) A text string representing the printer\'s mark in\nhuman-readable form, suitable for presentation to the user on the screen.')\
          .done().done()\
      .optional()\
          .field('Colorants')\
          .name('Colorants')\
          .type('dictionary')\
          .comment('(Optional; PDF 1.4) A dictionary identifying the individual colorants\nassociated with a printer\'s mark such as a color bar. For each entry in this\ndictionary, the key is a colorant name and the value is an array defining a\nSeparation color space for that colorant (see "Separation Color Spaces"\non page 201). The key must match the colorant name given in that color\nspace.')\
          .done().done()\
      .done()

  pdfspec.addClass('SeparationDictionary', 'Dictionary', 'Entries in a separation dictionary')\
      .required('NULL')\
          .field('Pages')\
          .name('Pages')\
          .type('array')\
          .comment('(Required) An array of indirect references to page objects representing separa-\ntions of the same document page. One of the page objects in the array must be\nthe one with which this separation dictionary is associated, and all of them must\nhave separation dictionaries (SeparationInfo entries) containing Pages arrays\nidentical to this one.')\
          .done().done()\
      .required('NULL')\
          .field('DeviceColorant')\
          .name('DeviceColorant')\
          .type('name or string')\
          .comment('(Required) The name of the device colorant to be used in rendering this\nseparation, such as Cyan or PANTONE 35 CV.')\
          .done().done()\
      .optional()\
          .field('ColorSpace')\
          .name('ColorSpace')\
          .type('array')\
          .comment('(Optional) An array defining a Separation or DeviceN color space (see "Separa-\ntion Color Spaces" on page 201 and "DeviceN Color Spaces" on page 205). This\nprovides additional information about the color specified by DeviceColorant-\nin particular, the alternate color space and tint transformation function that\nwould be used to represent the colorant as a process color. This information\nenables a viewer application to preview the separation in a color that approxi-\nmates the device colorant.\nThe value of DeviceColorant must match the space\'s colorant name (if it is a\nSeparation space) or be one of the space\'s colorant names (if it is a DeviceN\nspace).')\
          .done().done()\
      .done()

  pdfspec.addClass('PDF_XOutputIntentDictionary', 'Dictionary', 'Entries in a PDF/X output intent dictionary')\
      .optional()\
          .field('Type')\
          .name('Type')\
          .type('name')\
          .comment('(Optional) The type of PDF object that this dictionary describes;\nif present, must be OutputIntent for an output intent dictionary.')\
          .done().done()\
      .required('NULL')\
          .field('S')\
          .name('S')\
          .type('name')\
          .comment('(Required) The output intent subtype; must be GTS_PDFX for a\nPDF/X output intent.')\
          .done().done()\
      .optional()\
          .field('OutputCondition')\
          .name('OutputCondition')\
          .type('text string')\
          .comment('(Optional) A text string concisely identifying the intended out-\nput device or production condition in human-readable form.\nThis is the preferred method of defining such a string for pre-\nsentation to the user.')\
          .done().done()\
      .required('NULL')\
          .field('OutputConditionIdentifier')\
          .name('OutputConditionIdentifier')\
          .type('string')\
          .comment('(Required) A string identifying the intended output device or\nproduction condition in human- or machine-readable form. If\nhuman-readable, this string may be used in lieu of an Output-\nCondition string for presentation to the user.\nA typical value for this entry would be the name of a production\ncondition maintained in an industry-standard registry such as\nthe ICC Characterization Data Registry (see the Bibliography). If\nthe designated condition matches that in effect at production\ntime, it is the responsibility of the production software to pro-\nvide the corresponding ICC profile as defined in the registry.\nIf the intended production condition is not a recognized\nstandard, the value Custom is recommended for this entry; the\nDestOutputProfile entry defines the ICC profile and the Info\nentry is used for further human-readable identification.')\
          .done().done()\
      .optional()\
          .field('RegistryName')\
          .name('RegistryName')\
          .type('string')\
          .comment('(Optional) A string (conventionally a uniform resource identifi-\ner, or URI) identifying the registry in which the condition desig-\nnated by OutputConditionIdentifier is defined.')\
          .done().done()\
      .optional()\
          .field('Info')\
          .name('Info')\
          .type('text string')\
          .comment('(Required if OutputConditionIdentifier does not specify a standard\nproduction condition; optional otherwise) A human-readable text\nstring containing additional information or comments about\nthe intended target device or production condition.')\
          .done().done()\
      .optional()\
          .field('DestOutputProfile')\
          .name('DestOutputProfile')\
          .type('stream')\
          .comment('(Required if OutputConditionIdentifier does not specify a standard\nproduction condition; optional otherwise) An ICC profile stream\ndefining the transformation from the PDF document\'s source\ncolors to output device colorants.\nThe format of the profile stream is the same as that used in speci-\nfying an ICCBased color space (see "ICCBased Color Spaces" on\npage 189). The output transformation uses the profile\'s "from\nCIE" information (BToA in ICC terminology); the "to CIE"\n(AToB) information can optionally be used to remap source\ncolor values to some other destination color space, such as for\nscreen preview or hardcopy proofing. (See implementation note\n111 in Appendix H.)')\
          .done().done()\
      .done()

  pdfspec.addClass('TrapNetworkAnnotationDictionary', 'Dictionary', 'Additional entries specific to a trap network annotation')\
      .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('(Required) The type of annotation that this dictionary describes; must be\nTrapNet for a trap network annotation.')\
          .done().done()\
      .optional()\
          .field('Contents')\
          .name('Contents')\
          .type('text string')\
          .comment('(Optional; PDF 1.4) An alternate description of the annotation\'s contents in\nhuman-readable form, useful when extracting the document\'s contents in\nsupport of accessibility to disabled users or for other purposes (see Section\n9.8.2, "Alternate Descriptions").')\
          .done().done()\
      .optional()\
          .field('LastModified')\
          .name('LastModified')\
          .type('date')\
          .comment('(Required if Version and AnnotStates are absent; must be absent if Version and\nAnnotStates are present; PDF 1.4) The date and time (see Section 3.8.2,\n"Dates") when the trap network was most recently modified.')\
          .done().done()\
      .optional()\
          .field('Version')\
          .name('Version')\
          .type('array')\
          .comment('(Required if AnnotStates is present; must be absent if LastModified is present)\nAn unordered array of all objects present in the page description at the time\nthe trap networks were generated and that, if changed, could affect the\nappearance of the page. If present, the array must include the following\nobjects:\n*  All content streams identified in the page object\'s Contents entry (see\n   "Page Objects" on page 87)\n*  All resource objects (other than procedure sets) in the page\'s resource dic-\n   tionary (see Section 3.7.2, "Resource Dictionaries")\n*  All resource objects (other than procedure sets) in the resource diction-\n   aries of any form XObjects on the page (see Section 4.9, "Form XObjects")\n*  All OPI dictionaries associated with XObjects on the page (see Section\n   9.10.6, "Open Prepress Interface (OPI)")')\
          .done().done()\
      .optional()\
          .field('AnnotStates')\
          .name('AnnotStates')\
          .type('array')\
          .comment('(Required if Version is present; must be absent if LastModified is present) An\narray of name objects representing the appearance states (value of the AS\nentry) for annotations associated with the page. The appearance states must\nbe listed in the same order as the annotations in the page\'s Annots array (see\n"Page Objects" on page 87). For an annotation with no AS entry, the corre-\nsponding array element should be null. No appearance state should be\nincluded for the trap network annotation itself.')\
          .done().done()\
      .optional()\
          .field('FontFauxing')\
          .name('FontFauxing')\
          .type('array')\
          .comment('(Optional) An array of font dictionaries representing fonts that were "fauxed"\n(replaced by substitute fonts) during the generation of trap networks for the\npage.')\
          .done().done()\
      .done()

  pdfspec.addClass('TrapNetworkAppearanceStreamDictionary', 'Dictionary', 'Additional entries specific to a trap network appearance stream')\
      .required('NULL')\
          .field('PCM')\
          .name('PCM')\
          .type('name')\
          .comment('(Required) The name of the process color model that was assumed\nwhen this trap network was created; equivalent to the PostScript\npage device parameter ProcessColorModel (see Section 6.2.5 of the\nPostScript Language Reference, Third Edition). Valid values are\nDeviceGray, DeviceRGB, DeviceCMYK, DeviceCMY, DeviceRGBK,\nand DeviceN.')\
          .done().done()\
      .optional()\
          .field('SeparationColorNames')\
          .name('SeparationColorNames')\
          .type('array')\
          .comment('(Optional) An array of names identifying the colorants that were\nassumed when this network was created; equivalent to the Post-\nScript page device parameter of the same name (see Section 6.2.5 of\nthe PostScript Language Reference, Third Edition). Colorants im-\nplied by the process color model PCM are available automatically\nand need not be explicitly declared. If this entry is absent, the\ncolorants implied by PCM are assumed.')\
          .done().done()\
      .optional()\
          .field('TrapRegions')\
          .name('TrapRegions')\
          .type('array')\
          .comment('(Optional) An array of indirect references to TrapRegion objects\ndefining the page\'s trapping zones and the associated trapping\nparameters, as described in Adobe Technical Note #5620, Portable\nJob Ticket Format. These references are to objects comprising\nportions of a PJTF job ticket that is embedded in the PDF file.\nWhen the trapping zones and parameters are defined by an external\njob ticket (or by some other means, such as with JDF), this entry is\nabsent.')\
          .done().done()\
      .optional()\
          .field('TrapStyles')\
          .name('TrapStyles')\
          .type('text string')\
          .comment('(Optional) A human-readable text string that applications can use\nto describe this trap network to the user (for example, to allow\nswitching between trap networks).')\
          .done().done()\
      .done()

  pdfspec.addClass('OpiVersionDictionary', 'Dictionary', 'Entry in an OPI version dictionary')\
      .required('NULL')\
          .field('version_number')\
          .name('version_number')\
          .type('dictionary')\
          .comment('(Required; PDF 1.2) An OPI dictionary specifying the attributes of this proxy\n(see Tables 9.50 and 9.51). The key for this entry must be the name 1.3 or 2.0,\nidentifying the version of OPI to which the proxy corresponds.')\
          .done().done()\
      .done()


