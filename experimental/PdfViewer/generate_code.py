

import os
import sys

import datatypes
from autogen.pdfspec_autogen import *

# TODO(edisonn): date and some other types are in fact strings, with a custom format!!!
# TODO(edisonn): refer to page 99 (PDF data types)
knowTypes = {
'(any)': ['SkPdfObject*', 'ret', datatypes.CppNull(), 'true', 'use a mapper'],
# TODO(edisonn): return constant for undefined
'(undefined)': ['SkPdfObject*', 'ret', datatypes.CppNull(), 'true', 'use a mapper'],
'(various)': ['SkPdfObject*', 'ret', datatypes.CppNull(), 'true', 'use a mapper'],
'array': ['SkPdfArray*', '(SkPdfArray*)ret', datatypes.CppNull(), 'ret->isArray()'],
'boolean': ['bool', 'ret->boolValue()', datatypes.PdfBoolean('false'), 'ret->isBoolean()'],
#date is a string, with special formating, add here the 
'date': ['SkPdfDate', 'ret->dateValue()', datatypes.PdfDateNever(), 'ret->isDate()'],
'dictionary': ['SkPdfDictionary*', '(SkPdfDictionary*)ret', datatypes.CppNull(), 'ret->isDictionary()', 'use a mapper'],
'function': ['SkPdfFunction', 'ret->functionValue()', datatypes.PdfFunctionNone(), 'ret->isFunction()'],
'integer': ['int64_t', 'ret->intValue()', datatypes.PdfInteger(0), 'ret->isInteger()'],
'file_specification': ['SkPdfFileSpec', 'ret->fileSpecValue()', datatypes.FileSpecNone(), 'false'],
'name': ['std::string', 'ret->nameValue2()', datatypes.PdfString('""'), 'ret->isName()'],
#should assert, references should never be allowed here, should be resolved way earlier
'tree': ['SkPdfTree', 'ret->treeValue()', datatypes.EmptyTree(), 'false'],
'number': ['double', 'ret->numberValue()', datatypes.PdfNumber(0), 'ret->isNumber()'],
'rectangle': ['SkRect', 'ret->rectangleValue()', datatypes.EmptyRect(), 'ret->isRectangle()'],
'stream': ['SkPdfStream*', 'ret->getStream()',  datatypes.CppNull(), 'ret->hasStream()'],
'string': ['std::string', 'ret->stringValue2()', datatypes.PdfString('""'), 'ret->isAnyString()'],
'text': ['std::string', 'ret->stringValue2()', datatypes.PdfString('""'), 'ret->isAnyString()'],
'text string': ['std::string', 'ret->stringValue2()', datatypes.PdfString('""'), 'ret->isAnyString()'],
'matrix': ['SkMatrix', 'ret->matrixValue()', datatypes.IdentityMatrix(), 'ret->isMatrix()'],
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
    self.fCCPublicNative = []
    self.fCCPublicNativeCpp = []
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
    
  def carbonCopyPublicNative(self, cc):
    self.fCCPublicNative.append(cc)
    return self 

  def carbonCopyPublicNativeCpp(self, cc):
    self.fCCPublicNativeCpp.append(cc)
    return self 

  def done(self):
    return

class PdfClassManager:
  def __init__(self):
    self.fClasses = {}
    self.fClassesNamesInOrder = []

  def addClass(self, name, base='', comment=''):
    if name == 'Dictionary':
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


  def writeAsNull(self, nativeFileClass, cls, enumToCls):
    nativeFileClass.write('   SkPdf' + cls.fName +'* as' + cls.fName + '() {return (SkPdf' + cls.fName + '*)this;}\n')
    nativeFileClass.write('   const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return (const SkPdf' + cls.fName + '*)this;}\n')
    nativeFileClass.write('\n')

    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeAsNull(nativeFileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
      cnt = cnt + 1

       
  def writeAsFoo(self, nativeFileClass, cls, enumToCls):
    # TODO(edisonn): add a container, with sections, public, private, default, ...
    # the end code will be grouped
    
    # me
    nativeFileClass.write('public:\n')

    nativeFileClass.write('public:\n')
    nativeFileClass.write('   SkPdf' + cls.fName +'* as' + cls.fName + '() {return this;}\n')
    nativeFileClass.write('   const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return this;}\n')
    nativeFileClass.write('\n')

    if cls.fName == 'Dictionary':
      cnt = 0
      for sub in cls.fEnumSubclasses:
        self.writeAsNull(nativeFileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
            
    if cls.fName != 'Dictionary':
      nativeFileClass.write('private:\n')
      base = self.fClasses[cls.fBase]
      cnt = 0
      for sub in base.fEnumSubclasses:
        if enumToCls[base.fEnumSubclasses[cnt]].fName != cls.fName:
          self.writeAsNull(nativeFileClass, enumToCls[base.fEnumSubclasses[cnt]], enumToCls)
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
    global fileHeadersNative 
    global fileHeadersNativeCpp 
    global knowTypes
  
    # generate enum
    enumsRoot = []

    enumToCls = {}
    
    for name in self.fClasses:
      cls = self.fClasses[name]
      cls.fEnum = 'k' + name + '_SkPdfObjectType'
      cls.fEnumEnd = 'k' + name + '__End_SkPdfObjectType'

      fileHeadersNative.write('#include "SkPdf' + cls.fName + '_autogen.h"\n')
      fileHeadersNativeCpp.write('#include "SkPdf' + cls.fName + '_autogen.cpp"\n')
            
      if cls.fBase != '':
        self.fClasses[cls.fBase].fEnumSubclasses.append(cls.fEnum)

      if cls.fBase == '':
        enumsRoot.append(cls.fEnum)
       
      enumToCls[cls.fEnum] = cls
      
    enumsRoot.sort()
    
   
    # TODO(edisonn): move each .h in it's own file
    # write imports
    
    # write enums
    fileEnums = open(os.path.join(sys.argv[1], 'autogen', 'SkPdfEnums_autogen.h'), 'w')
    fileEnums.write('#ifndef __DEFINED__SkPdfEnums\n')
    fileEnums.write('#define __DEFINED__SkPdfEnums\n')
    fileEnums.write('\n')
    
    fileEnums.write('enum SkPdfObjectType {\n')
    fileEnums.write('  kNone_SkPdfObjectType = 0,\n')
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
      
      nativeFileClass = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdf' + cls.fName + '_autogen.h'), 'w')
      nativeFileClassCpp = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdf' + cls.fName + '_autogen.cpp'), 'w')

      nativeFileClass.write('#ifndef __DEFINED__SkPdf' + cls.fName + '\n')
      nativeFileClass.write('#define __DEFINED__SkPdf' + cls.fName + '\n')
      nativeFileClass.write('\n')

      nativeFileClassCpp.write('#include "SkPdf' + cls.fName + '_autogen.h"\n\n')
      nativeFileClassCpp.write('\n')

      
      if cls.fBase == '':
        nativeFileClass.write('#include "stddef.h"\n')
        nativeFileClass.write('#include <string>\n')
        nativeFileClass.write('#include "SkPdfEnums_autogen.h"\n')
        nativeFileClass.write('#include "SkPdfNYI.h"\n')
        nativeFileClass.write('#include "SkPdfObject.h"\n')
        nativeFileClass.write('class SkNativeParsedPDF;\n')

      if cls.fBase != '':
        nativeFileClass.write('#include "SkPdf' + cls.fBase + '_autogen.h"\n')

      nativeFileClassCpp.write('#include "SkNativeParsedPDF.h"\n')


      nativeFileClass.write('\n')
      
      if cls.fComment != '':
        nativeFileClass.write('// ' + cls.fComment + '\n')
      
      if cls.fBase == '':
        nativeFileClass.write('class SkPdf' + cls.fName + ' : public SkPdfObject {\n')
      else:
        nativeFileClass.write('class SkPdf' + cls.fName + ' : public SkPdf' + cls.fBase + ' {\n')
      
      self.writeAsFoo(nativeFileClass, cls, enumToCls)
      
      nativeFileClass.write('public:\n')

      for cc in cls.fCCPublicNative:
        nativeFileClass.write('  ' + cc + '\n')
      
      for cc in cls.fCCPublicNativeCpp:
        nativeFileClassCpp.write(cc + '\n\n')


      if cls.fBase == '':
        nativeFileClass.write('public:\n')
        
      # TODO(edisonn): add is valid ?
      #check required fieds, also, there should be an internal_valid() manually wrote for complex
      # situations
      # right now valid return true      
      # TODO(edisonn): cache the value of valid, have a set of bits that would remember what types are valid for this type
      nativeFileClass.write('   bool valid() const {return true;}\n')
      #nativeFileClass.write('\n')
      
      for field in cls.fFields:
        prop = field.fProp
        if prop.fCppName != '':
          
          lines = prop.fComment.split('\n')
          if prop.fComment != '' and len(lines) > 0:
            nativeFileClass.write('/** ' + lines[0] + '\n')
            for line in lines[1:]:
              nativeFileClass.write(' *  ' + line + '\n')
            nativeFileClass.write('**/\n')
          
          if prop.fCppName[0] == '[':
            nativeFileClass.write('/*\n')  # comment code of the atributes that can have any name
            nativeFileClassCpp.write('/*\n')  # comment code of the atributes that can have any name
          

          if len(prop.fTypes.split()) == 1:
            t = prop.fTypes.strip()

            nativeFileClass.write('  ' + knowTypes[t][0] + ' ' + prop.fCppName + '(SkNativeParsedPDF* doc);\n')
            nativeFileClassCpp.write('' + knowTypes[t][0] + ' SkPdf' + cls.fName + '::' + prop.fCppName + '(SkNativeParsedPDF* doc) {\n')
            nativeFileClassCpp.write('  SkPdfObject* ret = get(\"' + prop.fName + '\", \"' + prop.fAbr + '\");\n')
            nativeFileClassCpp.write('  if (doc) {ret = doc->resolveReference(ret);}\n')
            nativeFileClassCpp.write('  if ((ret != NULL && ' + knowTypes[t][3] + ') || (doc == NULL && ret != NULL && ret->isReference())) return ' + knowTypes[t][1] + ';\n')
            
            if field.fRequired:
              nativeFileClassCpp.write('  // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
              nativeFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n');
            elif prop.fDefault != '':
              nativeFileClassCpp.write('  return ' + prop.fDefault.toCpp() + ';\n');
            else:
              nativeFileClassCpp.write('  // TODO(edisonn): warn about missing default value for optional fields\n')
              nativeFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n');
            
            nativeFileClassCpp.write('}\n') 
            nativeFileClassCpp.write('\n')
          else:
            for type in prop.fTypes.split():
              t = type.strip()
              
              nativeFileClass.write('  bool is' + prop.fCppName + 'A' + t.title() + '(SkNativeParsedPDF* doc);\n')

              nativeFileClassCpp.write('bool SkPdf' + cls.fName + '::is' + prop.fCppName + 'A' + t.title() + '(SkNativeParsedPDF* doc) {\n')
              nativeFileClassCpp.write('  SkPdfObject* ret = get(\"' + prop.fName + '\", \"' + prop.fAbr + '\");\n')
              nativeFileClassCpp.write('  if (doc) {ret = doc->resolveReference(ret);}\n')
              nativeFileClassCpp.write('  return ret != NULL && ' + knowTypes[t][3] + ';\n')
              nativeFileClassCpp.write('}\n')
              nativeFileClassCpp.write('\n')

              nativeFileClass.write('  ' + knowTypes[t][0] + ' get' + prop.fCppName + 'As' + t.title() + '(SkNativeParsedPDF* doc);\n')
              nativeFileClassCpp.write('' + knowTypes[t][0] + ' SkPdf' + cls.fName + '::get' + prop.fCppName + 'As' + t.title() + '(SkNativeParsedPDF* doc) {\n')

              nativeFileClassCpp.write('  SkPdfObject* ret = get(\"' + prop.fName + '\", \"' + prop.fAbr + '\");\n')
              nativeFileClassCpp.write('  if (doc) {ret = doc->resolveReference(ret);}\n')
              nativeFileClassCpp.write('  if ((ret != NULL && ' + knowTypes[t][3] + ') || (doc == NULL && ret != NULL && ret->isReference())) return ' + knowTypes[t][1] + ';\n')


              if field.fRequired:
                nativeFileClassCpp.write('  // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
                nativeFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n');
              elif prop.fDefault != '':
                nativeFileClassCpp.write('  return ' + prop.fDefault.toCpp() + ';\n');
              else:
                nativeFileClassCpp.write('  // TODO(edisonn): warn about missing default value for optional fields\n')
                nativeFileClassCpp.write('  return ' + knowTypes[t][2].toCpp() + ';\n');

              nativeFileClassCpp.write('}\n') 
              nativeFileClassCpp.write('\n')
               
          nativeFileClass.write('  bool has_' + prop.fCppName + '() const;\n')
          nativeFileClassCpp.write('bool SkPdf' + cls.fName + '::has_' + prop.fCppName + '() const {\n')
          # TODO(edisonn): has_foo() does not check type, add has_valid_foo(), and check that type is expected (e.g. number, string, ...)
          nativeFileClassCpp.write('  return get(\"' + prop.fName + '\", \"' + prop.fAbr + '\") != NULL;\n')
          nativeFileClassCpp.write('}\n') 
          nativeFileClassCpp.write('\n') 
           
          if prop.fCppName[0] == '[':
            nativeFileClass.write('*/\n')  # comment code of the atributes that can have any name
            nativeFileClassCpp.write('*/\n')  # comment code of the atributes that can have any name
         

      nativeFileClass.write('};\n')
      nativeFileClass.write('\n')

      nativeFileClass.write('#endif  // __DEFINED__NATIVE_SkPdf' + cls.fName + '\n')

      nativeFileClass.close()
      nativeFileClassCpp.close()
    
      # generate constructor when knowing the type
      # later, p2, generate constructor when not knowing the type - very similar with parsing?
      
    # generate parser  
    # TODO(edisonn): fast recognition based on must attributes.
    fileMapperNative = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdfMapper_autogen.h'), 'w')
    fileMapperNativeCpp = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdfMapper_autogen.cpp'), 'w')

    fileMapperNative.write('#ifndef __DEFINED__SkPdfMapper\n')
    fileMapperNative.write('#define __DEFINED__SkPdfMapper\n')
    fileMapperNative.write('\n')

    fileMapperNative.write('#include "SkPdfHeaders_autogen.h"\n')
    fileMapperNative.write('#include "SkNativeParsedPDF.h"\n')
    fileMapperNative.write('#include "SkPdfObject.h"\n')


    fileMapperNativeCpp.write('#include "SkPdfMapper_autogen.h"\n')
    fileMapperNativeCpp.write('#include "SkPdfUtils.h"\n')
    fileMapperNativeCpp.write('#include "SkPdfObject.h"\n')
    fileMapperNativeCpp.write('\n')
    
    fileMapperNative.write('class SkPdfMapper {\n')

    fileMapperNative.write('  SkNativeParsedPDF* fParsedDoc;\n')
    
    fileMapperNative.write('public:\n')
    
    fileMapperNative.write('  SkPdfMapper(SkNativeParsedPDF* doc) : fParsedDoc(doc) {}\n')
    fileMapperNative.write('\n')
    
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      fileMapperNative.write('  SkPdfObjectType map' + name + '(const SkPdfObject* in) const;\n')

      fileMapperNativeCpp.write('SkPdfObjectType SkPdfMapper::map' + name + '(const SkPdfObject* in) const {\n')
      fileMapperNativeCpp.write('  if (!is' + name + '(in)) return kNone_SkPdfObjectType;\n')
      fileMapperNativeCpp.write('\n')
      if len(cls.fEnumSubclasses) > 0:
        fileMapperNativeCpp.write('  SkPdfObjectType ret;\n')

      # stream must be last one
      hasStream = False
      for sub in cls.fEnumSubclasses:
        fileMapperNativeCpp.write('  if (kNone_SkPdfObjectType != (ret = map' + enumToCls[sub].fName + '(in))) return ret;\n')
      
      fileMapperNativeCpp.write('\n')
      
      fileMapperNativeCpp.write('  return k' + name + '_SkPdfObjectType;\n')        
      fileMapperNativeCpp.write('}\n') 
      fileMapperNativeCpp.write('\n')
       
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      fileMapperNative.write('  bool is' + name + '(const SkPdfObject* nativeObj) const ;\n')
      fileMapperNativeCpp.write('bool SkPdfMapper::is' + name + '(const SkPdfObject* nativeObj) const {\n')
      
      if cls.fCheck != '':
        fileMapperNativeCpp.write('  return ' + cls.fCheck + ';\n')
      else:
        cntMust = 0
        emitedRet = False
        for field in cls.fFields:
          prop = field.fProp
          if prop.fHasMust:
            if emitedRet == False:
              fileMapperNativeCpp.write('  const SkPdfObject* ret = NULL;\n')
              emitedRet = True
            cntMust = cntMust + 1
            fileMapperNativeCpp.write('  if (!nativeObj->isDictionary()) return false;\n')
            fileMapperNativeCpp.write('  ret = nativeObj->get(\"' + prop.fName + '\", \"' + prop.fAbr + '\");\n')
            fileMapperNativeCpp.write('  if (ret == NULL) return false;\n')
            
            eval = '';
            # TODO(edisonn): this could get out of hand, and could have poor performance if continued on this path
            # but if we would write our parser, then best thing would be to create a map of (key, value) -> to bits
            # and at each (key, value) we do an and with the bits existent, then we check what bits are left, which would tell the posible types of this dictionary
            # and for non unique posinilities (if any) based on context, or the requester of dictionry we can determine fast the dictionary type
            mustBe = self.determineAllMustBe(cls, field, enumToCls)
            if len(mustBe) > 0:
              for cnd in mustBe:
                if eval == '':
                  eval = '(' + knowTypes[prop.fTypes.strip()][1]  + ' != ' + cnd.toCpp() + ')'
                else:
                  eval = eval + ' && ' + '(' + knowTypes[prop.fTypes.strip()][1]  + ' != ' + cnd.toCpp() + ')'
              fileMapperNativeCpp.write('  if (' + eval + ') return false;\n')
              fileMapperNativeCpp.write('\n')
      
        fileMapperNativeCpp.write('  return true;\n')
              
      fileMapperNativeCpp.write('}\n') 
      fileMapperNativeCpp.write('\n')    
    
      # TODO(edisonn): dict should be a SkPdfDictionary ?
      fileMapperNative.write('  bool SkPdf' + name + 'FromDictionary(const SkPdfObject* dict, const char* key, SkPdf' + name + '** data) const ;\n')
      fileMapperNativeCpp.write('bool SkPdfMapper::SkPdf' + name + 'FromDictionary(const SkPdfObject* dict, const char* key, SkPdf' + name + '** data) const {\n')
      fileMapperNativeCpp.write('  const SkPdfObject* value = dict->get(key);\n')
      fileMapperNativeCpp.write('  if (value == NULL) { return false; }\n')
      fileMapperNativeCpp.write('  if (data == NULL) { return true; }\n')
      fileMapperNativeCpp.write('  if (kNone_SkPdfObjectType == map' + name + '(value)) return false;\n')
      fileMapperNativeCpp.write('  *data = (SkPdf' + name + '*)value;\n')
      fileMapperNativeCpp.write('  return true;\n');
      fileMapperNativeCpp.write('}\n')
      fileMapperNativeCpp.write('\n')

      fileMapperNative.write('  bool SkPdf' + name + 'FromDictionary(const SkPdfObject* dict, const char* key, const char* abr, SkPdf' + name + '** data) const ;\n')
      fileMapperNativeCpp.write('bool SkPdfMapper::SkPdf' + name + 'FromDictionary(const SkPdfObject* dict, const char* key, const char* abr, SkPdf' + name + '** data) const {\n')
      fileMapperNativeCpp.write('  if (SkPdf' + name + 'FromDictionary(dict, key, data)) return true;\n')
      fileMapperNativeCpp.write('  if (abr == NULL || *abr == \'\\0\') return false;\n')
      fileMapperNativeCpp.write('  return SkPdf' + name + 'FromDictionary(dict, abr, data);\n')
      fileMapperNativeCpp.write('}\n')
      fileMapperNativeCpp.write('\n')
          
    fileMapperNative.write('};\n') 
    fileMapperNative.write('\n')
    
    fileMapperNative.write('#endif  // __DEFINED__SkPdfMapper\n')

    fileMapperNative.close()
    fileMapperNativeCpp.close()
    
    return

def generateCode():
  global fileHeadersNative 
  global fileHeadersNativeCpp 
  global knowTypes

  fileHeadersNative = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdfHeaders_autogen.h'), 'w')
  fileHeadersNativeCpp = open(os.path.join(sys.argv[1], 'native', 'autogen', 'SkPdfHeaders_autogen.cpp'), 'w')
  
  fileHeadersNative.write('#ifndef __DEFINED__SkPdfHeaders\n')
  fileHeadersNative.write('#define __DEFINED__SkPdfHeaders\n')
  fileHeadersNative.write('\n')

  fileHeadersNativeCpp.write('#include "SkPdfHeaders_autogen.h"\n')

  manager = PdfClassManager()
  
  # these classes are not explicitely backed by a table in the pdf spec
  manager.addClass('Dictionary')
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
  
  fileHeadersNative.write('#endif  // __DEFINED__SkPdfHeaders\n')

  fileHeadersNative.close()
  fileHeadersNativeCpp.close()

if '__main__' == __name__:
  #print sys.argv
  sys.exit(generateCode())

