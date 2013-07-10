#!/usr/local/bin/python
# coding: utf-8

import sys
import re

# TODO(edisonn): put processed part of file in a new file
# put unprocessed part, in a new file, so we see what we miss
# keep blank lines, and generate a version without the blank lines

#TODO (edisonn): deal manually with tables that don't have "KEY TYPE VALUE' header, e.g. 
#         TABLE 7.11 Restrictions on the entries in a soft-mask image dictionary
#KEY                       RESTRICTION


 
lines = 0
table = ''
tableHeaderFound = False
tableLine = 0
tableRow = 0
columnWidth = []
columnValues = None
mustFollowTableHeader = False
emitedDitionaryName = ''

knownTypes = {
'(any)',
unicode('undeﬁned', 'utf8'),
'(undefined)',
'(various)',
'array',
'or',
'boolean',
'date',
'dictionary',
'function',
'integer',
unicode('ﬁle', 'utf8'),
'file',
unicode('speciﬁcation', 'utf8'),
'specification',
'name',
'tree',
'number',
'rectangle',
'stream',
'string',
'text',
',',
' '
}

# TODO(edisonn): add a third element in the vector, the base class, by default it is Dictionary
# TODO(edisonn): add overrides for types map<field_name, type_name>
# e.g. ,{'Resources', 'ResourceDictionary'}
# TODO(edisonn): can be added one by one, or extracted from documentation

tableToClassName = {
'TABLE 3.4': ['StreamCommonDictionary', 'Entries common to all stream dictionaries'],
'TABLE 3.7': ['LzwdecodeAndFlatedecodeFiltersDictionary', 'Optional parameters for LZWDecode and FlateDecode filters'],
'TABLE 3.9': ['CcittfaxdecodeFilterDictionary', 'Optional parameters for the CCITTFaxDecode filter'],
'TABLE 3.10': ['Jbig2DecodeFilterDictionary', 'Optional parameter for the JBIG2Decode filter'],
'TABLE 3.11': ['DctdecodeFilterDictionary', 'Optional parameter for the DCTDecode filter'],
'TABLE 3.12': ['FileTrailerDictionary', 'Entries in the file trailer dictionary'],
'TABLE 3.13': ['EncryptionCommonDictionary', 'Entries common to all encryption dictionaries'],
'TABLE 3.14': ['StandardSecurityHandlerDictionary', 'Additional encryption dictionary entries for the standard security handler'],
'TABLE 3.16': ['CatalogDictionary', 'Entries in the catalog dictionary'],
'TABLE 3.17': ['PageTreeNodeDictionary', 'Required entries in a page tree node'],
'TABLE 3.18': ['PageObjectDictionary', 'Entries in a page object'],
'TABLE 3.19': ['NameDictionary', 'Entries in the name dictionary'],
'TABLE 3.21': ['ResourceDictionary', 'Entries in a resource dictionary'],
'TABLE 3.23': ['NameTreeNodeDictionary', 'Entries in a name tree node dictionary'],
'TABLE 3.25': ['NumberTreeNodeDictionary', 'Entries in a number tree node dictionary'],
'TABLE 3.26': ['FunctionCommonDictionary', 'Entries common to all function dictionaries'],
'TABLE 3.27': ['Type0FunctionDictionary', 'Additional entries specific to a type 0 function dictionary'],
'TABLE 3.28': ['Type2FunctionDictionary', 'Additional entries specific to a type 2 function dictionary'],
'TABLE 3.29': ['Type3FunctionDictionary', 'Additional entries specific to a type 3 function dictionary'],
'TABLE 3.32': ['FileSpecificationDictionary', 'Entries in a file specification dictionary'],
'TABLE 3.33': ['EmbeddedFileStreamDictionary', 'Additional entries in an embedded file stream dictionary'],
'TABLE 3.34': ['EmbeddedFileParameterDictionary', 'Entries in an embedded file parameter dictionary'],
'TABLE 3.35': ['MacOsFileInformationDictionary', 'Entries in a Mac OS file information dictionary'],
'TABLE 4.8': ['GraphicsStateDictionary', 'Entries in a graphics state parameter dictionary'],
'TABLE 4.13': ['CalgrayColorSpaceDictionary', 'Entries in a CalGray color space dictionary'],
'TABLE 4.14': ['CalrgbColorSpaceDictionary', 'Entries in a CalRGB color space dictionary'],
'TABLE 4.15': ['LabColorSpaceDictionary', 'Entries in a Lab color space dictionary'],
'TABLE 4.16': ['IccProfileStreamDictionary', 'Additional entries specific to an ICC profile stream dictionary'],
'TABLE 4.20': ['DeviceNColorSpaceDictionary', 'Entry in a DeviceN color space attributes dictionary'],
'TABLE 4.22': ['Type1PatternDictionary', 'Additional entries specific to a type 1 pattern dictionary'],
'TABLE 4.23': ['Type2PatternDictionary', 'Entries in a type 2 pattern dictionary'],
'TABLE 4.25': ['ShadingDictionary', 'Entries common to all shading dictionaries'],
'TABLE 4.26': ['Type1ShadingDictionary', 'Additional entries specific to a type 1 shading dictionary', 'ShadingDictionary'],
'TABLE 4.27': ['Type2ShadingDictionary', 'Additional entries specific to a type 2 shading dictionary', 'ShadingDictionary'],
'TABLE 4.28': ['Type3ShadingDictionary', 'Additional entries specific to a type 3 shading dictionary', 'ShadingDictionary'],
'TABLE 4.29': ['Type4ShadingDictionary', 'Additional entries specific to a type 4 shading dictionary', 'ShadingDictionary'],
'TABLE 4.30': ['Type5ShadingDictionary', 'Additional entries specific to a type 5 shading dictionary', 'ShadingDictionary'],
'TABLE 4.31': ['Type6ShadingDictionary', 'Additional entries specific to a type 6 shading dictionary', 'ShadingDictionary'],
'TABLE 4.35': ['ImageDictionary', 'Additional entries specific to an image dictionary', 'XObjectDictionary', {'Subtype': '[datatypes.PdfName(\'Image\')]'}],
'TABLE 4.37': ['AlternateImageDictionary', 'Entries in an alternate image dictionary'],
'TABLE 4.41': ['Type1FormDictionary', 'Additional entries specific to a type 1 form dictionary', 'XObjectDictionary', {'Subtype': '[datatypes.PdfName(\'Form\')]'}],
'TABLE 4.42': ['GroupAttributesDictionary', 'Entries common to all group attributes dictionaries'],
'TABLE 4.43': ['ReferenceDictionary', 'Entries in a reference dictionary'],
'TABLE 4.44': ['PSXobjectDictionary', 'Additional entries specific to a PostScript XObject dictionary'],
'TABLE 5.8': ['Type1FontDictionary', 'Entries in a Type 1 font dictionary', 'FontDictionary', {'Subtype': '[datatypes.PdfName(\'Type1\')]'}],
'TABLE 5.9': ['Type3FontDictionary', 'Entries in a Type 3 font dictionary', 'Type1FontDictionary', {'Subtype': '[datatypes.PdfName(\'Type3\')]'}],
'TABLE 5.11': ['EncodingDictionary', 'Entries in an encoding dictionary'],
'TABLE 5.12': ['CIDSystemInfoDictionary', 'Entries in a CIDSystemInfo dictionary'],
'TABLE 5.13': ['CIDFontDictionary', 'Entries in a CIDFont dictionary', '', {'Subtype': '[datatypes.PdfName(\'CIDFontType0\'), datatypes.PdfName(\'CIDFontType2\')]'}],
'TABLE 5.16': ['CMapDictionary', 'Additional entries in a CMap dictionary'],
'TABLE 5.17': ['Type0FontDictionary', 'Entries in a Type 0 font dictionary', 'FontDictionary', {'Subtype': '[datatypes.PdfName(\'Type0\')]'}],
'TABLE 5.18': ['FontDescriptorDictionary', 'Entries common to all font descriptors', '', {'Type': '[datatypes.PdfName(\'FontDescriptor\')]'}],
'TABLE 5.20': ['CIDFontDescriptorDictionary', 'Additional font descriptor entries for CIDFonts'],
'TABLE 5.23': ['EmbeddedFontStreamDictionary', 'Additional entries in an embedded font stream dictionary'],
'TABLE 6.3': ['Type1HalftoneDictionary', 'Entries in a type 1 halftone dictionary'],
'TABLE 6.4': ['Type6HalftoneDictionary', 'Additional entries specific to a type 6 halftone dictionary'],
'TABLE 6.5': ['Type10HalftoneDictionary', 'Additional entries specific to a type 10 halftone dictionary'],
'TABLE 6.6': ['Type16HalftoneDictionary', 'Additional entries specific to a type 16 halftone dictionary'],
'TABLE 6.7': ['Type5HalftoneDictionary', 'Entries in a type 5 halftone dictionary'],
'TABLE 7.10': ['SoftMaskDictionary', 'Entries in a soft-mask dictionary'],
'TABLE 7.12': ['SoftMaskImageDictionary', 'Additional entry in a soft-mask image dictionary'],
'TABLE 7.13': ['TransparencyGroupDictionary', 'Additional entries specific to a transparency group attributes dictionary'],
'TABLE 8.1': ['ViewerPreferencesDictionary', 'Entries in a viewer preferences dictionary'],
'TABLE 8.3': ['OutlineDictionary', 'Entries in the outline dictionary'],
'TABLE 8.4': ['OutlineItemDictionary', 'Entries in an outline item dictionary'],
'TABLE 8.6': ['PageLabelDictionary', 'Entries in a page label dictionary'],
'TABLE 8.7': ['ThreadDictionary', 'Entries in a thread dictionary'],
'TABLE 8.8': ['BeadDictionary', 'Entries in a bead dictionary'],
'TABLE 8.9': ['TransitionDictionary', 'Entries in a transition dictionary'],
'TABLE 8.10': ['AnnotationDictionary', 'Entries common to all annotation dictionaries'],
'TABLE 8.12': ['BorderStyleDictionary', 'Entries in a border style dictionary'],
'TABLE 8.13': ['AppearanceDictionary', 'Entries in an appearance dictionary'],
'TABLE 8.15': ['TextAnnotationDictionary', 'Additional entries specific to a text annotation'],
'TABLE 8.16': ['ALinkAnnotationDictionary', 'Additional entries specific to a link annotation'],
'TABLE 8.17': ['FreeTextAnnotationDictionary', 'Additional entries specific to a free text annotation'],
'TABLE 8.18': ['LineAnnotationDictionary', 'Additional entries specific to a line annotation'],
'TABLE 8.20': ['SquareOrCircleAnnotation', 'Additional entries specific to a square or circle annotation'],
'TABLE 8.21': ['MarkupAnnotationsDictionary', 'Additional entries specific to markup annotations'],
'TABLE 8.22': ['RubberStampAnnotationDictionary', 'Additional entries specific to a rubber stamp annotation'],
'TABLE 8.23': ['InkAnnotationDictionary', 'Additional entries specific to an ink annotation'],
'TABLE 8.24': ['PopUpAnnotationDictionary', 'Additional entries specific to a pop-up annotation'],
'TABLE 8.25': ['FileAttachmentAnnotationDictionary', 'Additional entries specific to a file attachment annotation'],
'TABLE 8.26': ['SoundAnnotationDictionary', 'Additional entries specific to a sound annotation'],
'TABLE 8.27': ['MovieAnnotationDictionary', 'Additional entries specific to a movie annotation'],
'TABLE 8.28': ['WidgetAnnotationDictionary', 'Additional entries specific to a widget annotation'],
'TABLE 8.29': ['ActionDictionary', 'Entries common to all action dictionaries'],
'TABLE 8.30': ['AnnotationActionsDictionary', 'Entries in an annotation\'s additional-actions dictionary'],
'TABLE 8.31': ['PageObjectActionsDictionary', 'Entries in a page object\'s additional-actions dictionary'],
'TABLE 8.32': ['FormFieldActionsDictionary', 'Entries in a form field\'s additional-actions dictionary'],
'TABLE 8.33': ['DocumentCatalogActionsDictionary', 'Entries in the document catalog\'s additional-actions dictionary'],
'TABLE 8.35': ['GoToActionDictionary', 'Additional entries specific to a go-to action'],
'TABLE 8.36': ['RemoteGoToActionDictionary', 'Additional entries specific to a remote go-to action'],
'TABLE 8.37': ['LaunchActionDictionary', 'Additional entries specific to a launch action'],
'TABLE 8.38': ['WindowsLaunchActionDictionary', 'Entries in a Windows launch parameter dictionary'],
'TABLE 8.39': ['ThreadActionDictionary', 'Additional entries specific to a thread action'],
'TABLE 8.40': ['URIActionDictionary', 'Additional entries specific to a URI action'],
'TABLE 8.41': ['URIDictionary', 'Entry in a URI dictionary'],
'TABLE 8.42': ['SoundActionDictionary', 'Additional entries specific to a sound action'],
'TABLE 8.43': ['MovieActionDictionary', 'Additional entries specific to a movie action'],
'TABLE 8.44': ['HideActionDictionary', 'Additional entries specific to a hide action'],
'TABLE 8.46': ['NamedActionsDictionary', 'Additional entries specific to named actions'],
'TABLE 8.47': ['InteractiveFormDictionary', 'Entries in the interactive form dictionary'],
'TABLE 8.49': ['FieldDictionary', 'Entries common to all field dictionaries'],
'TABLE 8.51': ['VariableTextFieldDictionary', 'Additional entries common to all fields containing variable text'],
'TABLE 8.52': ['AppearanceCharacteristicsDictionary', 'Entries in an appearance characteristics dictionary'],
'TABLE 8.54': ['CheckboxFieldDictionary', 'Additional entry specific to a checkbox field'],
'TABLE 8.55': ['RadioButtonFieldDictionary', 'Additional entry specific to a radio button field'],
'TABLE 8.57': ['TextFieldDictionary', 'Additional entry specific to a text field'],
'TABLE 8.59': ['ChoiceFieldDictionary', 'Additional entries specific to a choice field'],
'TABLE 8.60': ['SignatureDictionary', 'Entries in a signature dictionary'],
'TABLE 8.61': ['SubmitFormActionDictionary', 'Additional entries specific to a submit-form action'],
'TABLE 8.63': ['ResetFormActionDictionary', 'Additional entries specific to a reset-form action'],
'TABLE 8.65': ['ImportDataActionDictionary', 'Additional entries specific to an import-data action'],
'TABLE 8.66': ['JavascriptActionDictionary', 'Additional entries specific to a JavaScript action'],
'TABLE 8.67': ['FDFTrailerDictionary', 'Entry in the FDF trailer dictionary'],
'TABLE 8.68': ['FDFCatalogDictionary', 'Entries in the FDF catalog dictionary'],
'TABLE 8.69': ['FDFDictionary', 'Entries in the FDF dictionary'],
'TABLE 8.70': ['EncryptedEmbeddedFileStreamDictionary', 'Additional entry in an embedded file stream dictionary for an encrypted FDF file'],
'TABLE 8.71': ['JavascriptDictionary', 'Entries in the JavaScript dictionary'],
'TABLE 8.72': ['FDFFieldDictionary', 'Entries in an FDF field dictionary'],
'TABLE 8.73': ['IconFitDictionary', 'Entries in an icon fit dictionary'],
'TABLE 8.74': ['FDFPageDictionary', 'Entries in an FDF page dictionary'],
'TABLE 8.75': ['FDFTemplateDictionary', 'Entries in an FDF template dictionary'],
'TABLE 8.76': ['FDFNamedPageReferenceDictionary', 'Entries in an FDF named page reference dictionary'],
'TABLE 8.77': ['FDFFileAnnotationDictionary', 'Additional entry for annotation dictionaries in an FDF file'],
'TABLE 8.78': ['SoundObjectDictionary', 'Additional entries specific to a sound object'],
'TABLE 8.79': ['MovieDictionary', 'Entries in a movie dictionary'],
'TABLE 8.80': ['MovieActivationDictionary', 'Entries in a movie activation dictionary'],
'TABLE 9.2': ['DocumentInformationDictionary', 'Entries in the document information dictionary'],
'TABLE 9.3': ['MetadataStreamDictionary', 'Additional entries in a metadata stream dictionary'],
'TABLE 9.4': ['ComponentsWithMetadataDictionary', 'Additional entry for components having metadata'],
'TABLE 9.6': ['PagePieceDictionary', 'Entries in a page-piece dictionary'],
'TABLE 9.7': ['ApplicationDataDictionary', 'Entries in an application data dictionary'],
'TABLE 9.9': ['StructureTreeRootDictionary', 'Entries in the structure tree root'],
'TABLE 9.10': ['StructureElementDictionary', 'Entries in a structure element dictionary'],
'TABLE 9.11': ['MarkedContentReferenceDictionary', 'Entries in a marked-content reference dictionary'],
'TABLE 9.12': ['ObjectReferenceDictionary', 'Entries in an object reference dictionary'],
'TABLE 9.13': ['StructureElementAccessDictionary', 'Additional dictionary entries for structure element access'],
'TABLE 9.14': ['AttributeObjectDictionary', 'Entry common to all attribute objects'],
'TABLE 9.15': ['MarkInformationDictionary', 'Entry in the mark information dictionary'],
'TABLE 9.16': ['ArtifactsDictionary', 'Property list entries for artifacts'],
'TABLE 9.27': ['StandardStructureDictionary', 'Standard layout attributes common to all standard structure types'],
'TABLE 9.28': ['BlockLevelStructureElementsDictionary', 'Additional standard layout attributes specific to block-level structure elements'],
'TABLE 9.29': ['InlineLevelStructureElementsDictionary', 'Standard layout attributes specific to inline-level structure elements'],
'TABLE 9.30': ['ListAttributeDictionary', 'Standard list attribute'],
'TABLE 9.31': ['TableAttributesDictionary', 'Standard table attributes'],
'TABLE 9.32': ['WebCaptureInformationDictionary', 'Entries in the Web Capture information dictionary'],
'TABLE 9.33': ['WebCaptureDictionary', 'Entries common to all Web Capture content sets'],
'TABLE 9.34': ['WebCapturePageSetDictionary', 'Additional entries specific to a Web Capture page set'],
'TABLE 9.35': ['WebCaptureImageSetDictionary', 'Additional entries specific to a Web Capture image set'],
'TABLE 9.36': ['SourceInformationDictionary', 'Entries in a source information dictionary'],
'TABLE 9.37': ['URLAliasDictionary', 'Entries in a URL alias dictionary'],
'TABLE 9.38': ['WebCaptureCommandDictionary', 'Entries in a Web Capture command dictionary'],
'TABLE 9.40': ['WebCaptureCommandSettingsDictionary', 'Entries in a Web Capture command settings dictionary'],
'TABLE 9.41': ['BoxColorInformationDictionary', 'Entries in a box color information dictionary'],
'TABLE 9.42': ['BoxStyleDictionary', 'Entries in a box style dictionary'],
'TABLE 9.43': ['PrinterMarkAnnotationDictionary', 'Additional entries specific to a printer\'s mark annotation'],
'TABLE 9.44': ['PrinterMarkFormDictionary', 'Additional entries specific to a printer\'s mark form dictionary'],
'TABLE 9.45': ['SeparationDictionary', 'Entries in a separation dictionary'],
'TABLE 9.46': ['PDF_XOutputIntentDictionary', 'Entries in a PDF/X output intent dictionary'],
'TABLE 9.47': ['TrapNetworkAnnotationDictionary', 'Additional entries specific to a trap network annotation'],
'TABLE 9.48': ['TrapNetworkAppearanceStreamDictionary', 'Additional entries specific to a trap network appearance stream'],
'TABLE 9.49': ['OpiVersionDictionary', 'Entry in an OPI version dictionary'],
}

def buildKnownDictionaries():
  global tableToClassName
  global knownTypes
  
  ret = {}
  for e in tableToClassName:
    ret[tableToClassName[e][0]] = ''
    knownTypes.add(tableToClassName[e][0])
  
  return ret

knownDictionaries = buildKnownDictionaries()

def acceptType(val):
  global knownTypes
  
  ret = val
  
  for item in knownTypes:
    ret = ret.replace(item, '')
    
  return ret == ''


def inTable():
  global tableHeaderFound
  return tableHeaderFound    

def tableDescriptionFound(desc): 
  global table
  table = desc.strip()    

def tableHasHeader(): 
  global table
  global tableHeaderFound

  tableHeaderFound = True
  #print table    

def fix(val):
  ret = val
  
  # fix unicode chars
  ret = ret.replace(unicode('ﬁ', 'utf8'), 'fi')
  ret = ret.replace(u'\u201c', '\"')
  ret = ret.replace(u'\u201d', '\"')
  ret = ret.replace(u'\u2019', '\'')
  ret = ret.replace(u'\ufb02', 'fl')
  ret = ret.replace(u'\xae', '(R)')
  ret = ret.replace(u'\u2026', '...')
  ret = ret.replace(u'\xd7', 'x')
  ret = ret.replace(u'\u2212', '-')
  ret = ret.replace(u'\u2264', '<=')
  ret = ret.replace(u'\u2014', '-')
  ret = ret.replace(u'\u2013', '\'')
  ret = ret.replace(u'\u2022', '*')
  ret = ret.replace(u'\xb5', 'mu')
  ret = ret.replace(u'\xf7', '/')
  ret = ret.replace(u'\xc4', 'A')
  ret = ret.replace(u'\xc5', 'A')
  ret = ret.replace(u'\u2122', '(TM)')


  # how enable to emit this a python string
  ret = ret.replace('\'', '\\\'')
  ret = ret.replace('\n', '\\n')

  
  return ret
  
def commitRow(fspecPy):
  global columnValues
  global emitedDitionaryName
  global table
  global tableToClassName
  
  if columnValues == None:
    return
  
  #print columnValues
  
  lastClosed = columnValues[2].find(')')
  if lastClosed < 0:
    print 'ERRRRRRRRRRRRRRROR'
    print columnValues
    return
    
  spec = columnValues[2][:lastClosed + 1]
  spec = spec.replace('(', ';')
  spec = spec.replace(')', ';')
  spec = spec.strip(';')
  
  specs = spec.split(';')

  # clearly required, but it can be required with conditions. don't handle this ones here, but manually  
  required = specs[0] == 'Required' 
  
  inheritable = False
  version = ''
  for s in specs:
    if s.strip() == 'inheritable' or s.strip() == 'Inheritable':
      inheritable = True
    elif re.match('^PDF [0-9]*[\.[0-9]*]*', s.strip()):
      version = s.strip()
    elif s != 'Required':
      required = False
      
  #print spec
  #print specs
  #print required
  #print inheritable
  #print version
  #print columnValues
  
  columnValues = [fix(columnValues[0]), fix(columnValues[1]), fix(columnValues[2])]
  
  tableKey = re.search('(TABLE [0-9].[0-9][0-9]?)', table).group(1)

  if emitedDitionaryName == '':
    table = fix(table)
    
    #print table
    emitedDitionaryName = 'foo'
    e = re.search('[Entries|Entry] in [a-z]* (.* dictionary)', table)
    a = re.search('Additional [a-z]* in a[n]? (.* dictionary)', table)
    s = re.search('Additional [a-z]* (.*)', table)
    c = re.search('[Entries|Entry] common to all (.*)', table)
    o1 = re.search('Optional parameter[s]? for the (.*)', table)
    o2 = re.search('Optional parameter[s]? for (.*)', table)
    t = re.search('.*ntries in [a-z]* (.*)', table)

    r = re.search('Property list entries for (.*)', table)
    st = re.search('Standard (.*)', table)
    
    if e:
      emitedDitionaryName = e.group(1).title().replace(' ', '')
      #print emitedDitionaryName
    elif a:
      emitedDitionaryName = a.group(1).title().replace(' ', '')
      #print emitedDitionaryName
    elif s:
      emitedDitionaryName = s.group(1).title().replace(' ', '')
      #print emitedDitionaryName
    elif c:
      emitedDitionaryName = c.group(1).title().replace(' ', '') + 'Common'
      #print emitedDitionaryName
    elif o1:
      emitedDitionaryName = o1.group(1).title().replace(' ', '') + 'OptionalParameters'
      #print emitedDitionaryName
    elif o2:
      emitedDitionaryName = o2.group(1).title().replace(' ', '') + 'OptionalParameters'
      #print emitedDitionaryName
    elif t:
      emitedDitionaryName = t.group(1).title().replace(' ', '') + 'Dictionary'
      #print emitedDitionaryName
    elif r:
      emitedDitionaryName = r.group(1).title().replace(' ', '') + 'Dictionary'
      #print emitedDitionaryName
    elif st:
      emitedDitionaryName = st.group(1).title().replace(' ', '')  + 'Dictionary'
      #print emitedDitionaryName
    #else:
      #print table
    
    #print tableKey
    #print('\'' + tableKey + '\': [\'' + emitedDitionaryName + '\', \'' + table[len(tableKey) + 1:] + '\'],')

    emitedDitionaryName = tableToClassName[tableKey][0]
    comment = fix(tableToClassName[tableKey][1])
    
    if len(tableToClassName[tableKey]) >= 3 and tableToClassName[tableKey][2] != '':
      fspecPy.write('  pdfspec.addClass(\'' + emitedDitionaryName + '\', \'' + tableToClassName[tableKey][2] + '\', \'' + comment + '\')\\\n')
    else:
      fspecPy.write('  pdfspec.addClass(\'' + emitedDitionaryName + '\', \'Dictionary\', \'' + comment + '\')\\\n')

  if len(tableToClassName[tableKey]) >= 4 and columnValues[0] in tableToClassName[tableKey][3]:
    required = True

  if required:
    fspecPy.write('      .required(\'NULL\')\\\n')
  else:
    fspecPy.write('      .optional()\\\n')
    
  fspecPy.write('          .field(\'' + columnValues[0] + '\')\\\n')
  fspecPy.write('          .name(\'' + columnValues[0] + '\')\\\n')
  fspecPy.write('          .type(\'' + columnValues[1] + '\')\\\n')
  fspecPy.write('          .comment(\'' + columnValues[2] + '\')\\\n')

  if len(tableToClassName[tableKey]) >= 4 and columnValues[0] in tableToClassName[tableKey][3]:
    fspecPy.write('          .must(' + tableToClassName[tableKey][3][columnValues[0]] + ')\\\n')

  fspecPy.write('          .done().done()\\\n')
  
  
  columnValues = None
  
def newRow(first, second, third):
  global columnValues 
  columnValues = [first.rstrip(), second.rstrip(), third.rstrip()]

def appendRow(second, third):
  global columnValues
  if second.rstrip() != '':
    columnValues[1] = columnValues[1] + ' ' + second.rstrip()
  if third.rstrip() != '':
    columnValues[2] = columnValues[2] + '\n' + third.rstrip()

def rebaseTable(fspecPy, line):
  global knownTypes
  global columnWidth
  
  line2 = line.replace(',', ' , ')
  
  words = line2.split()
  
  if len(words) < 3:
    return False

  i = 1
  while i < len(words) - 1 and words[i] in knownTypes:
    i = i + 1
    
  if words[i].startswith('(Optional') or words[i].startswith('(Required'):
    commitRow(fspecPy)
    
    columnWidth[0] = line.find(words[1])
    
    if words[i].startswith('(Optional'):
      columnWidth[1] = line.find('(Optional') - columnWidth[0] 
    if words[i].startswith('(Required'):
      columnWidth[1] = line.find('(Required') - columnWidth[0] 
    return True
    
  return False
    
    
def stopTable(fspecPy):
  global tableHeaderFound
  global emitedDitionaryName

  if not inTable():
    return
  
  commitRow(fspecPy)
  tableHeaderFound = False
  emitedDitionaryName = ''
  fspecPy.write('      .done()\n')
  fspecPy.write('\n')
    

def killTable():
  return

def processLineCore(fspecPy, line):
  global lines
  global tableLine
  global tableRow
  global columnWidth
  global columnValues
  global mustFollowTableHeader
  
  #global fnewspec
  
  lines = lines + 1
  
  line = unicode(line, 'utf8')
  
  striped = line.rstrip()
  
  words = line.split()
  if len(words) == 0:
    stopTable(fspecPy)
    return False
        
  isTableHeader = re.search('^[\s]*(TABLE [0-9].[0-9][0-9]?)', striped)
  if isTableHeader:
    stopTable(fspecPy)
    tableDescriptionFound(striped)
    mustFollowTableHeader = True
    return False
  
  if mustFollowTableHeader:
    mustFollowTableHeader = False
    if len(words) != 3:
      killTable()
      return False
 
    # TODO(edisonn): support for generic table!
    if words[0] != 'KEY' or words[1] != 'TYPE' or words[2] != 'VALUE':
      killTable()
      return False

    tableHasHeader()
    columnWidth = [0, 0, 0]
    columnWidth[0] = striped.index('TYPE')
    columnWidth[1] = striped.index('VALUE') - striped.index('TYPE')
    columnWidth[2] = 0
    return True
      
  if inTable():
    tableLine = tableLine + 1
    first = striped[0 : columnWidth[0]]
    second = striped[columnWidth[0] : columnWidth[0] + columnWidth[1]]
    third = striped[columnWidth[0] + columnWidth[1] :]

    if tableLine == 1:
      if third[0] != '(':
        killTable()
        return False

      newRow(first, second, third)
      return True
    
    if rebaseTable(fspecPy, striped):
      first = striped[0 : columnWidth[0]]
      second = striped[columnWidth[0] : columnWidth[0] + columnWidth[1]]
      third = striped[columnWidth[0] + columnWidth[1] :]
    
    first = first.rstrip()
    second = second.rstrip()
    third = third.rstrip()
        
    if first == '' and second == '' and third != '':
      appendRow(second, third)
      return True
      
    if len(first.split()) > 1:
      stopTable(fspecPy)
      return False

    if first != '' and first[0] == ' ':
      stopTable(fspecPy)
      return False

    if first != '' and second != '' and third == '':
      stopTable(fspecPy)
      return False

    if first == '' and second != '' and second[0] != ' ':
      if acceptType(second):
        appendRow(second, third)
        return True
      else:
        stopTable(fspecPy)
        return False

    if first != '' and second != '' and third[0] != '(':
      stopTable()
      return False
      
    if first == '' and second != '' and second[0] == ' ':
      stopTable(fspecPy)
      return False

    if first != '' and second != '' and third[0] == '(':
      commitRow(fspecPy)
      newRow(first, second, third)
      return True
    
    return False
  return False
  
def processLine(fspecPy, line):
  #global fnewspec
  
  inSpec = processLineCore(fspecPy, line)
  
  #just return, use the next lines if you wish to rewrite spec
  return
  
  if inSpec:
    #resize colum with types
    line = line[:columnWidth[0] + columnWidth[1]] + (' ' * (60 - columnWidth[1])) + line[columnWidth[0] + columnWidth[1]:]
    line = line[:columnWidth[0]] + (' ' * (40 - columnWidth[0])) + line[columnWidth[0]:]
  
  #fnewspec.write(line)
  

def generateDef():
  global lines
  #global fnewspec
  
  #fnewspec = open('PdfReference-okular-2.txt', 'w')
  
  # pdf spec in text format
  fspecText = open(sys.argv[1], 'r')
  
  # pdf spec in python directives 
  fspecPy = open(sys.argv[2], 'w')
  
  fspecPy.write('import datatypes\n')
  fspecPy.write('\n')

  fspecPy.write('def buildPdfSpec(pdfspec):\n')
  
  for line in fspecText:
    processLine(fspecPy, line)
   
  # close last table if it was not closed already 
  stopTable(fspecPy)
  
  fspecPy.write('\n')

  fspecPy.write('def addDictionaryTypesTo(knowTypes):\n')  
  for e in tableToClassName:
    #TODO(edisonn): build this map
    
    fspecPy.write('  knowTypes[\'' + tableToClassName[e][0] + '\'] = [\'SkPdf' + tableToClassName[e][0] + '*\', \'(SkPdf' + tableToClassName[e][0] + '*)ret\', datatypes.CppNull(), \'ret->isDictionary() && ((SkPdf' + tableToClassName[e][0] + '*)ret)->valid()\', \'A_DICTIONARY\']\n')
  fspecPy.write('\n')
  
  #print lines
  #fnewspec.close()

if '__main__' == __name__:
  sys.exit(generateDef())
