#!/usr/local/bin/python
# coding: utf-8

import sys
import re

# TODO(edisonn): put processed part of file in a new file
# put unprocessed part, in a new file, so we see what we miss
# keep blank lines, and generate a version without the blank lines

#TODO (edisonn): deal manually with
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

tableToClassName = {
'TABLE 3.4': ['StreamDictionariesCommon', 'Entries common to all stream dictionaries'],
'TABLE 3.7': ['LzwdecodeAndFlatedecodeFiltersOptionalParameters', 'Optional parameters for LZWDecode and FlateDecode filters'],
'TABLE 3.9': ['CcittfaxdecodeFilterOptionalParameters', 'Optional parameters for the CCITTFaxDecode filter'],
'TABLE 3.10': ['Jbig2DecodeFilterOptionalParameters', 'Optional parameter for the JBIG2Decode filter'],
'TABLE 3.11': ['DctdecodeFilterOptionalParameters', 'Optional parameter for the DCTDecode filter'],
'TABLE 3.12': ['FileTrailerDictionary', 'Entries in the file trailer dictionary'],
'TABLE 3.13': ['EncryptionDictionariesCommon', 'Entries common to all encryption dictionaries'],
'TABLE 3.14': ['DictionaryEntriesForTheStandardSecurityHandler', 'Additional encryption dictionary entries for the standard security handler'],
'TABLE 3.16': ['CatalogDictionary', 'Entries in the catalog dictionary'],
'TABLE 3.17': ['PageTreeNodeDictionary', 'Required entries in a page tree node'],
'TABLE 3.18': ['PageObjectDictionary', 'Entries in a page object'],
'TABLE 3.19': ['NameDictionary', 'Entries in the name dictionary'],
'TABLE 3.21': ['ResourceDictionary', 'Entries in a resource dictionary'],
'TABLE 3.23': ['NameTreeNodeDictionary', 'Entries in a name tree node dictionary'],
'TABLE 3.25': ['NumberTreeNodeDictionary', 'Entries in a number tree node dictionary'],
'TABLE 3.26': ['FunctionDictionariesCommon', 'Entries common to all function dictionaries'],
'TABLE 3.27': ['SpecificToAType0FunctionDictionary', 'Additional entries specific to a type 0 function dictionary'],
'TABLE 3.28': ['SpecificToAType2FunctionDictionary', 'Additional entries specific to a type 2 function dictionary'],
'TABLE 3.29': ['SpecificToAType3FunctionDictionary', 'Additional entries specific to a type 3 function dictionary'],
'TABLE 3.32': ['FileSpecificationDictionary', 'Entries in a file specification dictionary'],
'TABLE 3.33': ['EmbeddedFileStreamDictionary', 'Additional entries in an embedded file stream dictionary'],
'TABLE 3.35': ['MacOsFileInformationDictionary', 'Entries in a Mac OS file information dictionary'],
'TABLE 4.13': ['CalgrayColorSpaceDictionary', 'Entries in a CalGray color space dictionary'],
'TABLE 4.14': ['CalrgbColorSpaceDictionary', 'Entries in a CalRGB color space dictionary'],
'TABLE 4.15': ['LabColorSpaceDictionary', 'Entries in a Lab color space dictionary'],
'TABLE 4.16': ['SpecificToAnIccProfileStreamDictionary', 'Additional entries specific to an ICC profile stream dictionary'],
'TABLE 4.20': ['DevicenColorSpaceAttributesDictionary', 'Entry in a DeviceN color space attributes dictionary'],
'TABLE 4.22': ['SpecificToAType1PatternDictionary', 'Additional entries specific to a type 1 pattern dictionary'],
'TABLE 4.23': ['Type2PatternDictionary', 'Entries in a type 2 pattern dictionary'],
'TABLE 4.25': ['ShadingDictionariesCommon', 'Entries common to all shading dictionaries'],
'TABLE 4.26': ['SpecificToAType1ShadingDictionary', 'Additional entries specific to a type 1 shading dictionary'],
'TABLE 4.27': ['SpecificToAType2ShadingDictionary', 'Additional entries specific to a type 2 shading dictionary'],
'TABLE 4.28': ['SpecificToAType3ShadingDictionary', 'Additional entries specific to a type 3 shading dictionary'],
'TABLE 4.29': ['SpecificToAType4ShadingDictionary', 'Additional entries specific to a type 4 shading dictionary'],
'TABLE 4.30': ['SpecificToAType5ShadingDictionary', 'Additional entries specific to a type 5 shading dictionary'],
'TABLE 4.31': ['SpecificToAType6ShadingDictionary', 'Additional entries specific to a type 6 shading dictionary'],
'TABLE 4.35': ['SpecificToAnImageDictionary', 'Additional entries specific to an image dictionary'],
'TABLE 4.37': ['AlternateImageDictionary', 'Entries in an alternate image dictionary'],
'TABLE 4.41': ['SpecificToAType1FormDictionary', 'Additional entries specific to a type 1 form dictionary'],
'TABLE 4.42': ['GroupAttributesDictionariesCommon', 'Entries common to all group attributes dictionaries'],
'TABLE 4.43': ['ReferenceDictionary', 'Entries in a reference dictionary'],
'TABLE 4.44': ['SpecificToAPostscriptXobjectDictionary', 'Additional entries specific to a PostScript XObject dictionary'],
'TABLE 5.8': ['Type1FontDictionary', 'Entries in a Type 1 font dictionary'],
'TABLE 5.9': ['Type3FontDictionary', 'Entries in a Type 3 font dictionary'],
'TABLE 5.11': ['EncodingDictionary', 'Entries in an encoding dictionary'],
'TABLE 5.12': ['CidsysteminfoDictionary', 'Entries in a CIDSystemInfo dictionary'],
'TABLE 5.13': ['CidfontDictionary', 'Entries in a CIDFont dictionary'],
'TABLE 5.16': ['CmapDictionary', 'Additional entries in a CMap dictionary'],
'TABLE 5.17': ['Type0FontDictionary', 'Entries in a Type 0 font dictionary'],
'TABLE 5.18': ['FontDescriptorsCommon', 'Entries common to all font descriptors'],
'TABLE 5.20': ['DescriptorEntriesForCidfonts', 'Additional font descriptor entries for CIDFonts'],
'TABLE 5.23': ['EmbeddedFontStreamDictionary', 'Additional entries in an embedded font stream dictionary'],
'TABLE 6.3': ['Type1HalftoneDictionary', 'Entries in a type 1 halftone dictionary'],
'TABLE 6.4': ['SpecificToAType6HalftoneDictionary', 'Additional entries specific to a type 6 halftone dictionary'],
'TABLE 6.5': ['SpecificToAType10HalftoneDictionary', 'Additional entries specific to a type 10 halftone dictionary'],
'TABLE 6.6': ['SpecificToAType16HalftoneDictionary', 'Additional entries specific to a type 16 halftone dictionary'],
'TABLE 6.7': ['Type5HalftoneDictionary', 'Entries in a type 5 halftone dictionary'],
'TABLE 7.10': ['Soft-MaskDictionary', 'Entries in a soft-mask dictionary'],
'TABLE 7.12': ['Soft-MaskImageDictionary', 'Additional entry in a soft-mask image dictionary'],
'TABLE 7.13': ['SpecificToATransparencyGroupAttributesDictionary', 'Additional entries specific to a transparency group attributes dictionary'],
'TABLE 8.1': ['ViewerPreferencesDictionary', 'Entries in a viewer preferences dictionary'],
'TABLE 8.3': ['OutlineDictionary', 'Entries in the outline dictionary'],
'TABLE 8.6': ['PageLabelDictionary', 'Entries in a page label dictionary'],
'TABLE 8.7': ['ThreadDictionary', 'Entries in a thread dictionary'],
'TABLE 8.9': ['TransitionDictionary', 'Entries in a transition dictionary'],
'TABLE 8.10': ['AnnotationDictionariesCommon', 'Entries common to all annotation dictionaries'],
'TABLE 8.12': ['BorderStyleDictionary', 'Entries in a border style dictionary'],
'TABLE 8.13': ['AppearanceDictionary', 'Entries in an appearance dictionary'],
'TABLE 8.15': ['SpecificToATextAnnotation', 'Additional entries specific to a text annotation'],
'TABLE 8.16': ['SpecificToALinkAnnotation', 'Additional entries specific to a link annotation'],
'TABLE 8.17': ['SpecificToAFreeTextAnnotation', 'Additional entries specific to a free text annotation'],
'TABLE 8.18': ['SpecificToALineAnnotation', 'Additional entries specific to a line annotation'],
'TABLE 8.20': ['SpecificToASquareOrCircleAnnotation', 'Additional entries specific to a square or circle annotation'],
'TABLE 8.21': ['SpecificToMarkupAnnotations', 'Additional entries specific to markup annotations'],
'TABLE 8.22': ['SpecificToARubberStampAnnotation', 'Additional entries specific to a rubber stamp annotation'],
'TABLE 8.23': ['SpecificToAnInkAnnotation', 'Additional entries specific to an ink annotation'],
'TABLE 8.24': ['SpecificToAPop-UpAnnotation', 'Additional entries specific to a pop-up annotation'],
'TABLE 8.26': ['SpecificToASoundAnnotation', 'Additional entries specific to a sound annotation'],
'TABLE 8.27': ['SpecificToAMovieAnnotation', 'Additional entries specific to a movie annotation'],
'TABLE 8.28': ['SpecificToAWidgetAnnotation', 'Additional entries specific to a widget annotation'],
'TABLE 8.29': ['ActionDictionariesCommon', 'Entries common to all action dictionaries'],
'TABLE 8.30': ['Annotation’SAdditional-ActionsDictionary', 'Entries in an annotation’s additional-actions dictionary'],
'TABLE 8.35': ['SpecificToAGo-ToAction', 'Additional entries specific to a go-to action'],
'TABLE 8.36': ['SpecificToARemoteGo-ToAction', 'Additional entries specific to a remote go-to action'],
'TABLE 8.37': ['SpecificToALaunchAction', 'Additional entries specific to a launch action'],
'TABLE 8.39': ['SpecificToAThreadAction', 'Additional entries specific to a thread action'],
'TABLE 8.40': ['SpecificToAUriAction', 'Additional entries specific to a URI action'],
'TABLE 8.41': ['UriDictionary', 'Entry in a URI dictionary'],
'TABLE 8.42': ['SpecificToASoundAction', 'Additional entries specific to a sound action'],
'TABLE 8.43': ['SpecificToAMovieAction', 'Additional entries specific to a movie action'],
'TABLE 8.44': ['SpecificToAHideAction', 'Additional entries specific to a hide action'],
'TABLE 8.46': ['SpecificToNamedActions', 'Additional entries specific to named actions'],
'TABLE 8.47': ['InteractiveFormDictionary', 'Entries in the interactive form dictionary'],
'TABLE 8.49': ['FieldDictionariesCommon', 'Entries common to all field dictionaries'],
'TABLE 8.51': ['CommonToAllFieldsContainingVariableText', 'Additional entries common to all fields containing variable text'],
'TABLE 8.52': ['AppearanceCharacteristicsDictionary', 'Entries in an appearance characteristics dictionary'],
'TABLE 8.54': ['SpecificToACheckboxField', 'Additional entry specific to a checkbox field'],
'TABLE 8.55': ['SpecificToARadioButtonField', 'Additional entry specific to a radio button field'],
'TABLE 8.57': ['SpecificToATextField', 'Additional entry specific to a text field'],
'TABLE 8.59': ['SpecificToAChoiceField', 'Additional entries specific to a choice field'],
'TABLE 8.60': ['SignatureDictionary', 'Entries in a signature dictionary'],
'TABLE 8.61': ['SpecificToASubmit-FormAction', 'Additional entries specific to a submit-form action'],
'TABLE 8.63': ['SpecificToAReset-FormAction', 'Additional entries specific to a reset-form action'],
'TABLE 8.65': ['SpecificToAnImport-DataAction', 'Additional entries specific to an import-data action'],
'TABLE 8.66': ['SpecificToAJavascriptAction', 'Additional entries specific to a JavaScript action'],
'TABLE 8.67': ['FdfTrailerDictionary', 'Entry in the FDF trailer dictionary'],
'TABLE 8.68': ['FdfCatalogDictionary', 'Entries in the FDF catalog dictionary'],
'TABLE 8.70': ['EmbeddedFileStreamDictionary', 'Additional entry in an embedded file stream dictionary for an encrypted FDF file'],
'TABLE 8.71': ['JavascriptDictionary', 'Entries in the JavaScript dictionary'],
'TABLE 8.72': ['FdfFieldDictionary', 'Entries in an FDF field dictionary'],
'TABLE 8.73': ['IconFitDictionary', 'Entries in an icon fit dictionary'],
'TABLE 8.74': ['FdfPageDictionary', 'Entries in an FDF page dictionary'],
'TABLE 8.75': ['FdfTemplateDictionary', 'Entries in an FDF template dictionary'],
'TABLE 8.76': ['FdfNamedPageReferenceDictionary', 'Entries in an FDF named page reference dictionary'],
'TABLE 8.77': ['ForAnnotationDictionariesInAnFdfFile', 'Additional entry for annotation dictionaries in an FDF file'],
'TABLE 8.78': ['SpecificToASoundObject', 'Additional entries specific to a sound object'],
'TABLE 8.79': ['MovieDictionary', 'Entries in a movie dictionary'],
'TABLE 9.2': ['DocumentInformationDictionary', 'Entries in the document information dictionary'],
'TABLE 9.3': ['MetadataStreamDictionary', 'Additional entries in a metadata stream dictionary'],
'TABLE 9.4': ['ForComponentsHavingMetadata', 'Additional entry for components having metadata'],
'TABLE 9.6': ['Page-PieceDictionary', 'Entries in a page-piece dictionary'],
'TABLE 9.7': ['ApplicationDataDictionary', 'Entries in an application data dictionary'],
'TABLE 9.9': ['StructureTreeRootDictionary', 'Entries in the structure tree root'],
'TABLE 9.11': ['Marked-ContentReferenceDictionary', 'Entries in a marked-content reference dictionary'],
'TABLE 9.12': ['ObjectReferenceDictionary', 'Entries in an object reference dictionary'],
'TABLE 9.13': ['EntriesForStructureElementAccess', 'Additional dictionary entries for structure element access'],
'TABLE 9.14': ['AttributeObjectsCommon', 'Entry common to all attribute objects'],
'TABLE 9.15': ['MarkInformationDictionary', 'Entry in the mark information dictionary'],
'TABLE 9.16': ['ArtifactsDictionary', 'Property list entries for artifacts'],
'TABLE 9.27': ['StandardStructureTypesCommon', 'Standard layout attributes common to all standard structure types'],
'TABLE 9.28': ['LayoutAttributesSpecificToBlock-LevelStructureElements', 'Additional standard layout attributes specific to block-level structure elements'],
'TABLE 9.29': ['LayoutAttributesSpecificToInline-LevelStructureElementsDictionary', 'Standard layout attributes specific to inline-level structure elements'],
'TABLE 9.30': ['ListAttributeDictionary', 'Standard list attribute'],
'TABLE 9.31': ['TableAttributesDictionary', 'Standard table attributes'],
'TABLE 9.32': ['WebCaptureInformationDictionary', 'Entries in the Web Capture information dictionary'],
'TABLE 9.33': ['WebCaptureContentSetsCommon', 'Entries common to all Web Capture content sets'],
'TABLE 9.35': ['SpecificToAWebCaptureImageSet', 'Additional entries specific to a Web Capture image set'],
'TABLE 9.36': ['SourceInformationDictionary', 'Entries in a source information dictionary'],
'TABLE 9.37': ['UrlAliasDictionary', 'Entries in a URL alias dictionary'],
'TABLE 9.38': ['WebCaptureCommandDictionary', 'Entries in a Web Capture command dictionary'],
'TABLE 9.40': ['WebCaptureCommandSettingsDictionary', 'Entries in a Web Capture command settings dictionary'],
'TABLE 9.41': ['BoxColorInformationDictionary', 'Entries in a box color information dictionary'],
'TABLE 9.43': ['SpecificToAPrinter’SMarkAnnotation', 'Additional entries specific to a printer’s mark annotation'],
'TABLE 9.44': ['SpecificToAPrinter’SMarkFormDictionary', 'Additional entries specific to a printer’s mark form dictionary'],
'TABLE 9.45': ['SeparationDictionary', 'Entries in a separation dictionary'],
'TABLE 9.46': ['Pdf/XOutputIntentDictionary', 'Entries in a PDF/X output intent dictionary'],
'TABLE 9.47': ['SpecificToATrapNetworkAnnotation', 'Additional entries specific to a trap network annotation'],
'TABLE 9.48': ['SpecificToATrapNetworkAppearanceStream', 'Additional entries specific to a trap network appearance stream'],
'TABLE 9.49': ['OpiVersionDictionary', 'Entry in an OPI version dictionary'],
}


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

  
def commitRow():
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
  
  columnValues = [columnValues[0].replace(unicode('ﬁ', 'utf8'), 'fi'), columnValues[1].replace(unicode('ﬁ', 'utf8'), 'fi'), columnValues[2].replace(unicode('ﬁ', 'utf8'), 'fi')]
  
  if emitedDitionaryName == '':
    table = table.replace(unicode('ﬁ', 'utf8'), 'fi')
    
    
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
    
    tableKey = re.search('(TABLE [0-9].[0-9][0-9]?)', table).group(1)
    #print tableKey
    #print('\'' + tableKey + '\': [\'' + emitedDitionaryName + '\', \'' + table[len(tableKey) + 1:] + '\'],')

    emitedDitionaryName = tableToClassName[tableKey][0]
    comment = tableToClassName[tableKey][1]
  
    print('  all.addClass(\'' + emitedDitionaryName + '\', \'Dictionary\', \'' + comment + '\')\\')

  if required:
    print('      .required(\'NULL\')\\')
  else:
    print('      .optional()\\')
    
  print('          .field(\'' + columnValues[0] + '\')\\')
  print('          .name(\'' + columnValues[0] + '\')\\')
  print('          .type(\'' + columnValues[1] + '\')\\')
  print('          .comment(\'' + columnValues[2] + '\')\\')
  print('          .done().done()\\')
  
  
  columnValues = None
  
def newRow(first, second, third):
  global columnValues
  columnValues = [first.rstrip(), second.rstrip(), third.rstrip()]

def appendRow(second, third):
  global columnValues
  if second.rstrip() != '':
    columnValues[1] = columnValues[1] + ' ' + second.rstrip()
  if third.rstrip() != '':
    columnValues[2] = columnValues[2] + ' ' + third.rstrip()

def rebaseTable(line):
  global knownTypes
  global columnWidth
  
  words = line.split()
  
  if len(words) < 3:
    return False

  i = 1
  while i < len(words) - 1 and words[i] in knownTypes:
    i = i + 1
    
  if words[i].startswith('(Optional') or words[i].startswith('(Required'):
    commitRow()
    
    columnWidth[0] = line.find(words[1])
    
    if words[i].startswith('(Optional'):
      columnWidth[1] = line.find('(Optional') - columnWidth[0] 
    if words[i].startswith('(Required'):
      columnWidth[1] = line.find('(Required') - columnWidth[0] 
    return True
    
  return False
    
    
def stopTable():
  global tableHeaderFound
  global emitedDitionaryName
  commitRow()
  tableHeaderFound = False
  emitedDitionaryName = ''
  print('      .done()')
  print
    

def killTable():
  return

def processLine(line):
  global lines
  global tableLine
  global tableRow
  global columnWidth
  global columnValues
  global mustFollowTableHeader
  
  lines = lines + 1
  
  line = unicode(line, 'utf8')
  
  striped = line.rstrip()
  
  words = line.split()
  if len(words) == 0:
    return
    
  if words[0] == 'TABLE':
    tableDescriptionFound(striped)
    mustFollowTableHeader = True
    return
  
  if mustFollowTableHeader:
    mustFollowTableHeader = False
    if len(words) != 3:
      killTable()
 
    # TODO(edisonn): support for generic table!
    if words[0] != 'KEY' or words[1] != 'TYPE' or words[2] != 'VALUE':
      killTable()
      return

    tableHasHeader()
    columnWidth = [0, 0, 0]
    columnWidth[0] = striped.index('TYPE')
    columnWidth[1] = striped.index('VALUE') - striped.index('TYPE')
    columnWidth[2] = 0
    return
      
  if inTable():
    tableLine = tableLine + 1
    first = striped[0 : columnWidth[0]]
    second = striped[columnWidth[0] : columnWidth[0] + columnWidth[1]]
    third = striped[columnWidth[0] + columnWidth[1] :]
    
    if tableLine == 1:
      if third[0] != '(':
        killTable()
        return

      newRow(first, second, third)
      return
    
    if rebaseTable(striped):
      first = striped[0 : columnWidth[0]]
      second = striped[columnWidth[0] : columnWidth[0] + columnWidth[1]]
      third = striped[columnWidth[0] + columnWidth[1] :]
    
    first = first.rstrip()
    second = second.rstrip()
    third = third.rstrip()
        
    if first == '' and second == '' and third != '':
      appendRow(second, third)
      return
      
    if len(first.split()) > 1:
      stopTable()
      return

    if first != '' and first[0] == ' ':
      stopTable()
      return

    if first != '' and second != '' and third == '':
      stopTable()
      return

    if first == '' and second != '' and second[0] != ' ':
      if acceptType(second):
        appendRow(second, third)
      else:
        stopTable()
      return

    if first != '' and second != '' and third[0] != '(':
      stopTable()
      return
      
    if first == '' and second != '' and second[0] == ' ':
      stopTable()
      return

    if first != '' and second != '' and third[0] == '(':
      commitRow()
      newRow(first, second, third)
      return
  

def generateDef():
  global lines
  for line in sys.stdin:
    processLine(line)
  #print lines

if '__main__' == __name__:
  sys.exit(generateDef())