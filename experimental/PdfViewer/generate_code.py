

import sys

import datatypes
from autogen.pdfspec_autogen import *

knowTypes = {
'(any)': ['SkPdfObject*', 'SkPdfObjectFromDictionary', datatypes.CppNull(), 'true', 'use a mapper'],
'(undefined)': ['SkPdfObject*', 'SkPdfObjectFromDictionary', datatypes.CppNull(), 'true', 'use a mapper'],
'(various)': ['SkPdfObject*', 'SkPdfObjectFromDictionary', datatypes.CppNull(), 'true', 'use a mapper'],
'array': ['SkPdfArray*', 'ArrayFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Array'],
'boolean': ['bool', 'BoolFromDictionary', datatypes.PdfBoolean('false'), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Bool'],
'date': ['SkPdfDate', 'DateFromDictionary', datatypes.PdfDateNever(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Array'],
'dictionary': ['SkPdfDictionary*', 'SkPdfDictionaryFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Dictionary', 'use a mapper'],
'function': ['SkPdfFunction', 'FunctionFromDictionary', datatypes.PdfFunctionNone(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Reference'],
'integer': ['long', 'LongFromDictionary', datatypes.PdfInteger(0), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Number'],
'file_specification': ['SkPdfFileSpec', 'FileSpecFromDictionary', datatypes.FileSpecNone(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Reference'],
'name': ['std::string', 'NameFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Name'],
'tree': ['SkPdfTree*', 'TreeFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Reference'],
'number': ['double', 'DoubleFromDictionary', datatypes.PdfNumber(0), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Real || ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Number'],
'rectangle': ['SkRect*', 'SkRectFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Array && ret->podofo()->GetArray().GetLength() == 4'],
'stream': ['SkPdfStream*', 'StreamFromDictionary',  datatypes.CppNull(), 'ret->podofo()->HasStream()'],
'string': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_String || ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_HexString'],
'text': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_String || ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_HexString'],
'text string': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_String || ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_HexString'],
'matrix': ['SkMatrix*', 'SkMatrixFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == PoDoFo::ePdfDataType_Array && ret->podofo()->GetArray().GetLength() == 4'],
}


class PdfField:
  def __init__(self, parent, name, abr):
    self.fParent = parent
    self.fName = name
    self.fAbr = abr
    
    self.fDefault = ''
    self.fTypes = ''
    self.fCppName = ''
    self.fEnumValues = []
    self.fHasMust = False
    self.fMustBe = []
    self.fComment = ''

  def must(self, value):
    self.fHasMust = True
    self.fMustBe = value
    return self
    
  def default(self, value):
    self.fDefault = value
    return self
    
  def multiple(self, enumValues):
    self.fEnumValues = enumValues
    return self

  def name(self, name):
    self.fCppName = name
    return self
    
  def type(self, types):
    # TODO (edisonn): if simple type, use it, otherwise set it to Dictionary, and set a mask for valid types, like array or name
    types = types.strip()
    types = types.replace(' or ', ' ')
    types = types.replace(' or,', ' ')
    types = types.replace(',or ', ' ')
    types = types.replace(',or,', ' ')
    types = types.replace(',', ' ')
    types = types.replace('text', ' ') # TODO(edisonn): what is the difference between 'text string' and 'string'?
    types = types.replace('file specification', 'file_specification')
    
    
    self.fTypes = types
    return self

  def comment(self, comment):
    self.fComment = comment
    return self
      
  def done(self):
    return self.fParent


class PdfClassField:
  def __init__(self, parent, required, version='', inheritable=False):
    #self.fProp = ''
    self.fParent = parent
    self.fRequired = required
    self.fVersion = version
    self.fInheritable = inheritable
    
  def field(self, name, abr=''):
    self.fProp = PdfField(self, name, abr)
    return self.fProp 
    
  def done(self):
    return self.fParent

class PdfClass:
  def __init__(self, name, base, comment):
    self.fFields = []
    self.fIncludes = []
    self.fCCPublicPodofo = []
    self.fCCPublicPodofoCpp = []
    self.fName = name
    self.fBase = base
    self.fComment = comment
    
    self.fEnumSubclasses = [] 
    
    self.fEnum = '!UNDEFINED'
    self.fEnumEnd = '!UNDEFINED'
    self.fCheck = ''
    
  def check(self, ifCheck):
    self.fCheck = ifCheck
    return self

  def required(self, badDefault):
    field = PdfClassField(self, True)
    field.fBadDefault = badDefault
    self.fFields.append(field)
    return field
    
  def optional(self):
    field = PdfClassField(self, False)
    self.fFields.append(field)
    return field
  
  #([Required] [;] [inheritable] [;] [version]; [comments])
  # version: PDF [d].[d]
  # ; separate props
  #inheritable
  #version
  #required, if
  #optional, if
    
  def include(self, path):
    self.fIncludes.append(path)
    return self
    
  def carbonCopyPublicPodofo(self, cc):
    self.fCCPublicPodofo.append(cc)
    return self 

  def carbonCopyPublicPodofoCpp(self, cc):
    self.fCCPublicPodofoCpp.append(cc)
    return self 

  def done(self):
    return

class PdfClassManager:
  def __init__(self):
    self.fClasses = {}
    self.fClassesNamesInOrder = []

  def addClass(self, name, base='Object', comment=''):
    if name == 'Object':
      cls = PdfClass(name, '', comment)
    else:
      cls = PdfClass(name, base, comment)
    self.fClasses[name] = cls
    self.fClassesNamesInOrder.append(name)
    return cls
  
  def writeEnum(self, fileEnums, enum, enumToCls):
    fileEnums.write('  ' + enum + ',\n')
    cls = enumToCls[enum]
    cls.fEnumSubclasses.sort()
    
    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeEnum(fileEnums, cls.fEnumSubclasses[cnt], enumToCls)
      cnt = cnt + 1
      
    if cnt != 0:
       fileEnums.write('  ' + cls.fEnumEnd + ',\n')


  def writeAsNull(self, podofoFileClass, cls, enumToCls):
    podofoFileClass.write('  virtual SkPdf' + cls.fName +'* as' + cls.fName + '() {return NULL;}\n')
    podofoFileClass.write('   virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return NULL;}\n')
    podofoFileClass.write('\n')

    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeAsNull(podofoFileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
      cnt = cnt + 1

       
  def writeAsFoo(self, podofoFileClass, cls, enumToCls):
    # TODO(edisonn): add a container, with sections, public, private, default, ...
    # the end code will be grouped
    
    # me
    podofoFileClass.write('public:\n')

    podofoFileClass.write('public:\n')
    podofoFileClass.write('   SkPdf' + cls.fName +'* as' + cls.fName + '() {return this;}\n')
    podofoFileClass.write('  virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return this;}\n')
    podofoFileClass.write('\n')

    if cls.fName == 'Object':
      cnt = 0
      for sub in cls.fEnumSubclasses:
        self.writeAsNull(podofoFileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
            
    if cls.fName != 'Object':
      podofoFileClass.write('private:\n')
      base = self.fClasses[cls.fBase]
      cnt = 0
      for sub in base.fEnumSubclasses:
        if enumToCls[base.fEnumSubclasses[cnt]].fName != cls.fName:
          self.writeAsNull(podofoFileClass, enumToCls[base.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
      
      
  def determineAllMustBe(self, cls, field, enumToCls):
    mustBe = []
    for sub in cls.fEnumSubclasses:
      mustBe = mustBe + self.determineAllMustBe(enumToCls[sub], field, enumToCls)

    for subField in cls.fFields:
      if subField.fProp.fName == field.fProp.fName:
        mustBe = mustBe + subField.fProp.fMustBe

#    while cls.fBase != '':
#      cls = self.fClasses[cls.fBase]
#      # TODO(edisonn): bad perf
#      for subField in cls.fFields:
#        if subField.fProp.fName == field.fProp.fName:
#          mustBe = mustBe + subField.fProp.fMustBe

    return mustBe 
  
  def write(self):
    global fileHeadersPodofo 
    global fileHeadersPodofoCpp 
    global knowTypes
  
    # generate enum
    enumsRoot = []

    enumToCls = {}
    
    for name in self.fClasses:
      cls = self.fClasses[name]
      cls.fEnum = 'k' + name + '_SkPdfObjectType'
      cls.fEnumEnd = 'k' + name + '__End_SkPdfObjectType'

      fileHeadersPodofo.write('#include "SkPdf' + cls.fName + '_autogen.h"\n')
      fileHeadersPodofoCpp.write('#include "SkPdf' + cls.fName + '_autogen.cpp"\n')
            
      if cls.fBase != '':
        self.fClasses[cls.fBase].fEnumSubclasses.append(cls.fEnum)

      if cls.fBase == '':
        enumsRoot.append(cls.fEnum)
       
      enumToCls[cls.fEnum] = cls
      
    enumsRoot.sort()
    
   
    # TODO(edisonn): move each .h in it's own file
    # write imports
    
    # write enums
    fileEnums = open(sys.argv[1] + 'autogen/SkPdfEnums_autogen.h', 'w')
    fileEnums.write('#ifndef __DEFINED__SkPdfEnums\n')
    fileEnums.write('#define __DEFINED__SkPdfEnums\n')
    fileEnums.write('\n')
    
    fileEnums.write('enum SkPdfObjectType {\n')
    for enum in enumsRoot:
      self.writeEnum(fileEnums, enum, enumToCls)
    fileEnums.write('};\n')
    fileEnums.write('\n')
    
    # write forward class declaration
    for name in self.fClassesNamesInOrder:
      fileEnums.write('class SkPdf' + name + ';\n')
    fileEnums.write('\n')

    fileEnums.write('#endif  // __DEFINED__SkPdfEnums\n')
    fileEnums.close()
    
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      enum = cls.fEnum
      
      podofoFileClass = open(sys.argv[1] + 'podofo/autogen/SkPdf' + cls.fName + '_autogen.h', 'w')
      podofoFileClassCpp = open(sys.argv[1] + 'podofo/autogen/SkPdf' + cls.fName + '_autogen.cpp', 'w')

      podofoFileClass.write('#ifndef __DEFINED__SkPdf' + cls.fName + '\n')
      podofoFileClass.write('#define __DEFINED__SkPdf' + cls.fName + '\n')
      podofoFileClass.write('\n')

      podofoFileClassCpp.write('#include "SkPdf' + cls.fName + '_autogen.h"\n\n')
      podofoFileClassCpp.write('#include "podofo.h"\n')
      podofoFileClassCpp.write('#include "SkPodofoUtils.h"\n')
      podofoFileClassCpp.write('#include "SkPdfMapper_autogen.h"\n')
      podofoFileClassCpp.write('\n')

      
      if cls.fBase == '':
        podofoFileClass.write('#include "stddef.h"\n')
        podofoFileClass.write('#include <string>\n')
        podofoFileClass.write('#include "SkPdfEnums_autogen.h"\n')
        podofoFileClass.write('#include "SkPdfNYI.h"\n')
        podofoFileClass.write('#include "SkPodofoUtils.h"\n')

      if cls.fBase != '':
        podofoFileClass.write('#include "SkPdf' + cls.fBase + '_autogen.h"\n')

      if cls.fBase == '':
        podofoFileClass.write('#include "SkPodofoParsedPDF.h"\n')

      podofoFileClass.write('\n')
      
      if cls.fBase == '':
        podofoFileClass.write('namespace PoDoFo {\n')
        podofoFileClass.write('class PdfMemDocument;\n')
        podofoFileClass.write('class PdfObject;\n')      
        podofoFileClass.write('}\n')
      
      if cls.fComment != '':
        podofoFileClass.write('// ' + cls.fComment + '\n')
      
      if cls.fBase == '':
        podofoFileClass.write('class SkPdf' + cls.fName + ' {\n')
      else:
        podofoFileClass.write('class SkPdf' + cls.fName + ' : public SkPdf' + cls.fBase + ' {\n')
      
      podofoFileClass.write('public:\n')
      podofoFileClass.write('  virtual SkPdfObjectType getType() const { return ' + cls.fEnum + ';}\n')
      if len(cls.fEnumSubclasses) == 0:
        podofoFileClass.write('  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(' + cls.fEnum + ' + 1);}\n')
      else:
        podofoFileClass.write('  virtual SkPdfObjectType getTypeEnd() const { return ' + cls.fEnumEnd + ';}\n')
      
      self.writeAsFoo(podofoFileClass, cls, enumToCls)
      
      podofoFileClass.write('public:\n')

      for cc in cls.fCCPublicPodofo:
        podofoFileClass.write('  ' + cc + '\n')
      
      for cc in cls.fCCPublicPodofoCpp:
        podofoFileClassCpp.write(cc + '\n\n')


      if cls.fBase == '':
        podofoFileClass.write('protected:\n')
        podofoFileClass.write('  const PoDoFo::PdfMemDocument* fPodofoDoc;\n')
        podofoFileClass.write('  const SkPodofoParsedPDF* fParsedDoc;\n')
        podofoFileClass.write('  const PoDoFo::PdfObject* fPodofoObj;\n')
        podofoFileClass.write('\n')
        
        podofoFileClass.write('public:\n')
        
        podofoFileClass.write('  SkPdf' + cls.fName + '(const SkPodofoParsedPDF* doc = NULL, const PoDoFo::PdfObject* podofoObj = NULL) : fPodofoDoc(doc->podofo()), fParsedDoc(doc), fPodofoObj(podofoObj) {}\n')
        podofoFileClass.write('\n')
        podofoFileClass.write('  const SkPodofoParsedPDF* doc() const { return fParsedDoc;}\n')
        podofoFileClass.write('  const void* data() const {return fPodofoObj;}\n')
        podofoFileClass.write('  const PoDoFo::PdfObject* podofo() const {return fPodofoObj;}\n')
      else:
        podofoFileClass.write('public:\n')
        podofoFileClass.write('  SkPdf' + cls.fName + '(const SkPodofoParsedPDF* doc = NULL, const PoDoFo::PdfObject* podofoObj = NULL) : SkPdf' + cls.fBase + '(doc, podofoObj) {}\n')
        podofoFileClass.write('\n')
      

      # TODO(edisonn): add is valid ?
      #check required fieds, also, there should be an internal_valid() manually wrote for complex
      # situations
      # right now valid return true      
      #podofoFileClass.write('  virtual bool valid() const {return true;}\n')
      #podofoFileClass.write('\n')
      
      for field in cls.fFields:
        prop = field.fProp
        if prop.fCppName != '':
          
          lines = prop.fComment.split('\n')
          if prop.fComment != '' and len(lines) > 0:
            podofoFileClass.write('/** ' + lines[0] + '\n')
            for line in lines[1:]:
              podofoFileClass.write(' *  ' + line + '\n')
            podofoFileClass.write('**/\n')
          
          if prop.fCppName[0] == '[':
            podofoFileClass.write('/*\n')  # comment code of the atributes that can have any name
            podofoFileClassCpp.write('/*\n')  # comment code of the atributes that can have any name
          

          if len(prop.fTypes.split()) == 1:
            t = prop.fTypes.strip()

            podofoFileClass.write('  ' + knowTypes[t][0] + ' ' + prop.fCppName + '() const;\n')
            podofoFileClassCpp.write('' + knowTypes[t][0] + ' SkPdf' + cls.fName + '::' + prop.fCppName + '() const {\n')
            podofoFileClassCpp.write('  ' + knowTypes[t][0] + ' ret;\n')
            
            #hack, find out if it is dict, they have an extra entry in the array
            if len(knowTypes[t]) == 5:
              podofoFileClassCpp.write('  if (fParsedDoc->mapper()->' + knowTypes[t][1] + '(podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
            else:
              podofoFileClassCpp.write('  if (' + knowTypes[t][1] + '(fParsedDoc, podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
            
            if field.fRequired == False and prop.fDefault != '':
              podofoFileClassCpp.write('  return ' + prop.fDefault.toCpp() + ';\n');
            else:
              podofoFileClassCpp.write('  // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
              podofoFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n');
            podofoFileClassCpp.write('}\n') 
            podofoFileClassCpp.write('\n')
          else:
            for type in prop.fTypes.split():
              t = type.strip()
              
              podofoFileClass.write('  bool is' + prop.fCppName + 'A' + t.title() + '() const;\n')

              podofoFileClassCpp.write('bool SkPdf' + cls.fName + '::is' + prop.fCppName + 'A' + t.title() + '() const {\n')
              podofoFileClassCpp.write('  SkPdfObject* ret = NULL;\n')
              podofoFileClassCpp.write('  if (!fParsedDoc->mapper()->SkPdfObjectFromDictionary(podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return false;\n')
              podofoFileClassCpp.write('  return ' + knowTypes[t][3] + ';\n')
              podofoFileClassCpp.write('}\n')
              podofoFileClassCpp.write('\n')

              podofoFileClass.write('  ' + knowTypes[t][0] + ' get' + prop.fCppName + 'As' + t.title() + '() const;\n')
              podofoFileClassCpp.write('' + knowTypes[t][0] + ' SkPdf' + cls.fName + '::get' + prop.fCppName + 'As' + t.title() + '() const {\n')
              podofoFileClassCpp.write('  ' + knowTypes[t][0] + ' ret = ' + knowTypes[t][2].toCpp() + ';\n')

              # hack
              if len(knowTypes[t]) == 5:
                podofoFileClassCpp.write('  if (fParsedDoc->mapper()->' + knowTypes[t][1] + '(podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
              else:
                podofoFileClassCpp.write('  if (' + knowTypes[t][1] + '(fParsedDoc, podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
              
              podofoFileClassCpp.write('  // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
              podofoFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n')
              podofoFileClassCpp.write('}\n') 
              podofoFileClassCpp.write('\n')
               
          podofoFileClass.write('  bool has_' + prop.fCppName + '() const;\n')
          podofoFileClassCpp.write('bool SkPdf' + cls.fName + '::has_' + prop.fCppName + '() const {\n')
          podofoFileClassCpp.write('  return (ObjectFromDictionary(fParsedDoc, podofo()->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", NULL));\n')
          podofoFileClassCpp.write('}\n') 
          podofoFileClassCpp.write('\n') 
           
          if prop.fCppName[0] == '[':
            podofoFileClass.write('*/\n')  # comment code of the atributes that can have any name
            podofoFileClassCpp.write('*/\n')  # comment code of the atributes that can have any name
         

      podofoFileClass.write('};\n')
      podofoFileClass.write('\n')

      podofoFileClass.write('#endif  // __DEFINED__PODOFO_SkPdf' + cls.fName + '\n')

      podofoFileClass.close()
      podofoFileClassCpp.close()
    
      # generate constructor when knowing the type
      # later, p2, generate constructor when not knowing the type - very similar with parsing?
      
    # generate parser  
    # TODO(edisonn): fast recognition based on must attributes.
    fileMapperPodofo = open(sys.argv[1] + 'podofo/autogen/SkPdfMapper_autogen.h', 'w')
    fileMapperPodofoCpp = open(sys.argv[1] + 'podofo/autogen/SkPdfMapper_autogen.cpp', 'w')

    fileMapperPodofo.write('#ifndef __DEFINED__SkPdfMapper\n')
    fileMapperPodofo.write('#define __DEFINED__SkPdfMapper\n')
    fileMapperPodofo.write('\n')

    fileMapperPodofo.write('#include "SkPdfHeaders_autogen.h"\n')


    fileMapperPodofo.write('namespace PoDoFo {\n')
    fileMapperPodofo.write('class PdfDictionary;\n')
    fileMapperPodofo.write('class PdfMemDocument;\n')
    fileMapperPodofo.write('class PdfObject;\n')      
    fileMapperPodofo.write('}\n')

    fileMapperPodofoCpp.write('#include "SkPdfMapper_autogen.h"\n')
    fileMapperPodofoCpp.write('#include "SkPdfUtils.h"\n')
    fileMapperPodofoCpp.write('#include "podofo.h"\n')
    fileMapperPodofoCpp.write('\n')
    
    fileMapperPodofo.write('class SkPdfMapper {\n')

    fileMapperPodofo.write('  const SkPodofoParsedPDF* fParsedDoc;\n')
    fileMapperPodofo.write('  const PoDoFo::PdfMemDocument* fPodofoDoc;\n')
    
    fileMapperPodofo.write('public:\n')
    
    fileMapperPodofo.write('  SkPdfMapper(const SkPodofoParsedPDF* doc) : fParsedDoc(doc), fPodofoDoc(doc ? doc->podofo() : NULL) {}\n')
    fileMapperPodofo.write('\n')
    
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      fileMapperPodofo.write('  bool map' + name + '(const SkPdfObject* in, SkPdf' + name + '** out) const;\n')

      fileMapperPodofoCpp.write('bool SkPdfMapper::map' + name + '(const SkPdfObject* in, SkPdf' + name + '** out) const {\n')
      fileMapperPodofoCpp.write('  return map' + name + '((const PoDoFo::PdfObject*)in->data(), (SkPdf' + name + '**)out);\n')
      fileMapperPodofoCpp.write('}\n') 
      fileMapperPodofoCpp.write('\n')

      fileMapperPodofo.write('  bool map' + name + '(const PoDoFo::PdfObject* podofoObj, SkPdf' + name + '** out) const ;\n')
      fileMapperPodofoCpp.write('bool SkPdfMapper::map' + name + '(const PoDoFo::PdfObject* podofoObj, SkPdf' + name + '** out) const {\n')
      fileMapperPodofoCpp.write('  if (!is' + name + '(podofoObj)) return false;\n')
      fileMapperPodofoCpp.write('\n')

      # stream must be last one
      hasStream = False
      for sub in cls.fEnumSubclasses:
        if cls.fName == 'Object' and enumToCls[sub].fName == 'Stream':
          hasStream = True
        else:
          fileMapperPodofoCpp.write('  if (map' + enumToCls[sub].fName + '(podofoObj, (SkPdf' + enumToCls[sub].fName + '**)out)) return true;\n')
      
      if hasStream:
        fileMapperPodofoCpp.write('  if (mapStream(podofoObj, (SkPdfStream**)out)) return true;\n')
      

      fileMapperPodofoCpp.write('\n')
      
      fileMapperPodofoCpp.write('  *out = new SkPdf' + name + '(fParsedDoc, podofoObj);\n')
      fileMapperPodofoCpp.write('  return true;\n')        
      fileMapperPodofoCpp.write('}\n') 
      fileMapperPodofoCpp.write('\n')
       
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      fileMapperPodofo.write('  bool is' + name + '(const PoDoFo::PdfObject* podofoObj) const ;\n')
      fileMapperPodofoCpp.write('bool SkPdfMapper::is' + name + '(const PoDoFo::PdfObject* podofoObj) const {\n')
      
      if cls.fCheck != '':
        fileMapperPodofoCpp.write('  return ' + cls.fCheck + ';\n')
      else:
        cntMust = 0
        for field in cls.fFields:
          prop = field.fProp
          if prop.fHasMust:
            cntMust = cntMust + 1
            fileMapperPodofoCpp.write('  ' + knowTypes[prop.fTypes.strip()][0] + ' ' + prop.fCppName + ';\n')
            fileMapperPodofoCpp.write('  if (!podofoObj->IsDictionary()) return false;\n')
            fileMapperPodofoCpp.write('  if (!' + knowTypes[prop.fTypes.strip()][1] + '(fParsedDoc, podofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &' + prop.fCppName + ')) return false;\n')
            
            eval = '';
            # TODO(edisonn): this could get out of hand, and could have poor performance if continued on this path
            # but if we would write our parser, then best thing would be to create a map of (key, value) -> to bits
            # and at each (key, value) we do an and with the bits existent, then we check what bits are left, which would tell the posible types of this dictionary
            # and for non unique posinilities (if any) based on context, or the requester of dictionry we can determine fast the dictionary type
            mustBe = self.determineAllMustBe(cls, field, enumToCls)
            if len(mustBe) > 0:
              for cnd in mustBe:
                if eval == '':
                  eval = '(' + prop.fCppName + ' != ' + cnd.toCpp() + ')'
                else:
                  eval = eval + ' && ' + '(' + prop.fCppName + ' != ' + cnd.toCpp() + ')'
              fileMapperPodofoCpp.write('  if (' + eval + ') return false;\n')
              fileMapperPodofoCpp.write('\n')
      
        fileMapperPodofoCpp.write('  return true;\n')
              
      fileMapperPodofoCpp.write('}\n') 
      fileMapperPodofoCpp.write('\n')    
    
      fileMapperPodofo.write('  bool SkPdf' + name + 'FromDictionary(const PoDoFo::PdfDictionary& dict, const char* key, SkPdf' + name + '** data) const ;\n')
      fileMapperPodofoCpp.write('bool SkPdfMapper::SkPdf' + name + 'FromDictionary(const PoDoFo::PdfDictionary& dict, const char* key, SkPdf' + name + '** data) const {\n')
      fileMapperPodofoCpp.write('  const PoDoFo::PdfObject* value = resolveReferenceObject(fParsedDoc, dict.GetKey(PoDoFo::PdfName(key)), true);\n')
      fileMapperPodofoCpp.write('  if (value == NULL) { return false; }\n')
      fileMapperPodofoCpp.write('  if (data == NULL) { return true; }\n')
      fileMapperPodofoCpp.write('  return map' + name + '(value, (SkPdf' + name + '**)data);\n')
      fileMapperPodofoCpp.write('}\n')
      fileMapperPodofoCpp.write('\n')

      fileMapperPodofo.write('  bool SkPdf' + name + 'FromDictionary(const PoDoFo::PdfDictionary& dict, const char* key, const char* abr, SkPdf' + name + '** data) const ;\n')
      fileMapperPodofoCpp.write('bool SkPdfMapper::SkPdf' + name + 'FromDictionary(const PoDoFo::PdfDictionary& dict, const char* key, const char* abr, SkPdf' + name + '** data) const {\n')
      fileMapperPodofoCpp.write('  if (SkPdf' + name + 'FromDictionary(dict, key, data)) return true;\n')
      fileMapperPodofoCpp.write('  if (abr == NULL || *abr == \'\\0\') return false;\n')
      fileMapperPodofoCpp.write('  return SkPdf' + name + 'FromDictionary(dict, abr, data);\n')
      fileMapperPodofoCpp.write('}\n')
      fileMapperPodofoCpp.write('\n')
          
    fileMapperPodofo.write('};\n') 
    fileMapperPodofo.write('\n')
    
    fileMapperPodofo.write('#endif  // __DEFINED__SkPdfMapper\n')

    fileMapperPodofo.close()
    fileMapperPodofoCpp.close()
    
    return

def generateCode():
  global fileHeadersPodofo 
  global fileHeadersPodofoCpp 
  global knowTypes

  fileHeadersPodofo = open(sys.argv[1] + 'podofo/autogen/SkPdfHeaders_autogen.h', 'w')
  fileHeadersPodofoCpp = open(sys.argv[1] + 'podofo/autogen/SkPdfHeaders_autogen.cpp', 'w')
  
  fileHeadersPodofo.write('#ifndef __DEFINED__SkPdfHeaders\n')
  fileHeadersPodofo.write('#define __DEFINED__SkPdfHeaders\n')
  fileHeadersPodofo.write('\n')

  fileHeadersPodofoCpp.write('#include "SkPdfHeaders_autogen.h"\n')

  manager = PdfClassManager()
  
  manager.addClass('Object')
  
  # TODO(edisonn): perf, instead of virtual functions, store data in field and reurn it.
  # maybe in constructor load it, or laizy load it 
  
  manager.addClass('Null').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Null')
  manager.addClass('Boolean').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Bool')\
                             .carbonCopyPublicPodofo('bool value() const;')\
                             .carbonCopyPublicPodofoCpp('bool SkPdfBoolean::value() const {return podofo()->GetBool();}')
                             
  manager.addClass('Integer').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Number || podofoObj->GetDataType() == PoDoFo::ePdfDataType_Real')\
                             .carbonCopyPublicPodofo('long value() const;')\
                             .carbonCopyPublicPodofoCpp('long SkPdfInteger::value() const {return podofo()->GetNumber();}')
  
  manager.addClass('Number', 'Integer').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Number || podofoObj->GetDataType() == PoDoFo::ePdfDataType_Real')\
                             .carbonCopyPublicPodofo('double value() const;')\
                             .carbonCopyPublicPodofoCpp('double SkPdfNumber::value() const {return podofo()->GetReal();}')\
  
  manager.addClass('Name').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Name')\
                             .carbonCopyPublicPodofo('const std::string& value() const;')\
                             .carbonCopyPublicPodofoCpp('const std::string& SkPdfName::value() const {return podofo()->GetName().GetName();}')
  
  manager.addClass('Reference').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Reference')
  
  manager.addClass('Array').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Array')\
                             .carbonCopyPublicPodofo('const int size() const;')\
                             .carbonCopyPublicPodofoCpp('const int SkPdfArray::size() const {return podofo()->GetArray().GetSize();}')\
                             .carbonCopyPublicPodofo('SkPdfObject* operator[](int i) const;')\
                             .carbonCopyPublicPodofoCpp('SkPdfObject* SkPdfArray::operator[](int i) const { SkPdfObject* ret = NULL;  fParsedDoc->mapper()->mapObject(&podofo()->GetArray()[i], &ret);  return ret; }')
  
  manager.addClass('String').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_String || podofoObj->GetDataType() == PoDoFo::ePdfDataType_HexString')\
                            .carbonCopyPublicPodofo('const std::string& value() const;')\
                            .carbonCopyPublicPodofoCpp('const std::string& SkPdfString::value() const {return podofo()->GetString().GetStringUtf8();}')\
                            .carbonCopyPublicPodofo('const char* c_str() const;')\
                            .carbonCopyPublicPodofoCpp('const char* SkPdfString::c_str() const {return podofo()->GetString().GetString();}')\
                            .carbonCopyPublicPodofo('size_t len() const;')\
                            .carbonCopyPublicPodofoCpp('size_t SkPdfString::len() const {return podofo()->GetString().GetLength();}')
                             
  manager.addClass('HexString', 'String').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_HexString')\
  
  manager.addClass('Dictionary').check('podofoObj->GetDataType() == PoDoFo::ePdfDataType_Dictionary')\
                                .carbonCopyPublicPodofo('SkPdfObject* get(const char* dictionaryKeyName) const;')\
                                .carbonCopyPublicPodofoCpp('SkPdfObject* SkPdfDictionary::get(const char* dictionaryKeyName) const {SkPdfObject* ret = NULL; fParsedDoc->mapper()->mapObject(resolveReferenceObject(fParsedDoc, podofo()->GetDictionary().GetKey(PoDoFo::PdfName(dictionaryKeyName))), &ret); return ret;}')\

  # attached to a dictionary in podofo
  manager.addClass('Stream')\
              .carbonCopyPublicPodofo('bool GetFilteredCopy(char** buffer, long* len) const;')\
              .carbonCopyPublicPodofoCpp('bool SkPdfStream::GetFilteredCopy(char** buffer, long* len) const {try {PoDoFo::pdf_long podofoLen = 0; *buffer = NULL; *len = 0;podofo()->GetStream()->GetFilteredCopy(buffer, &podofoLen); *len = (long)podofoLen;} catch (PoDoFo::PdfError& e) { return false; } return true;}')
  
  
  # these classes are not explicitely backed by a table in the pdf spec
  manager.addClass('XObjectDictionary', 'Dictionary')
  
  manager.addClass('FontDictionary', 'Dictionary')
  
  manager.addClass('TrueTypeFontDictionary', 'Type1FontDictionary')\
          .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('')\
          .must([datatypes.PdfName('TrueType')])\
          .done().done()\
  
  
  addDictionaryTypesTo(knowTypes)
  buildPdfSpec(manager)  

  manager.addClass('MultiMasterFontDictionary', 'Type1FontDictionary')\
          .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('')\
          .must([datatypes.PdfName('MMType1')])\
          .done().done()\


  manager.write()
  
  fileHeadersPodofo.write('#endif  // __DEFINED__SkPdfHeaders\n')

  fileHeadersPodofo.close()
  fileHeadersPodofoCpp.close()

if '__main__' == __name__:
  #print sys.argv
  sys.exit(generateCode())

