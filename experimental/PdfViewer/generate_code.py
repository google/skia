

import sys

import datatypes
import pdfspec_autogen

knowTypes = {
'(any)': ['SkPdfObject*', 'ObjectFromDictionary', datatypes.CppNull(), 'true'],
'(undefined)': ['SkPdfObject*', 'ObjectFromDictionary', datatypes.CppNull(), 'true'],
'(various)': ['SkPdfObject*', 'ObjectFromDictionary', datatypes.CppNull(), 'true'],
'array': ['SkPdfArray', 'ArrayFromDictionary', datatypes.PdfArrayNone(), 'ret->podofo()->GetDataType() == ePdfDataType_Array'],
'boolean': ['bool', 'BoolFromDictionary', datatypes.PdfBoolean('false'), 'ret->podofo()->GetDataType() == ePdfDataType_Bool'],
'date': ['SkPdfDate', 'DateFromDictionary', datatypes.PdfDateNever(), 'ret->podofo()->GetDataType() == ePdfDataType_Array'],
'dictionary': ['SkPdfDictionary*', 'DictionaryFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == ePdfDataType_Dictionary'],
'function': ['SkPdfFunction', 'FunctionFromDictionary', datatypes.PdfFunctionNone(), 'ret->podofo()->GetDataType() == ePdfDataType_Reference'],
'integer': ['long', 'LongFromDictionary', datatypes.PdfInteger(0), 'ret->podofo()->GetDataType() == ePdfDataType_Number'],
'file_specification': ['SkPdfFileSpec', 'FileSpecFromDictionary', datatypes.FileSpecNone(), 'ret->podofo()->GetDataType() == ePdfDataType_Reference'],
'name': ['std::string', 'NameFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == ePdfDataType_Name'],
'tree': ['SkPdfTree*', 'TreeFromDictionary', datatypes.CppNull(), 'ret->podofo()->GetDataType() == ePdfDataType_Reference'],
'number': ['double', 'DoubleFromDictionary', datatypes.PdfNumber(0), 'ret->podofo()->GetDataType() == ePdfDataType_Real || ret->podofo()->GetDataType() == ePdfDataType_Number'],
'rectangle': ['SkRect', 'SkRectFromDictionary', datatypes.PdfEmptyRect(), 'ret->podofo()->GetDataType() == ePdfDataType_Array'],
'stream': ['SkPdfStream*', 'StreamFromDictionary',  datatypes.CppNull(), 'ret->podofo()->HasStream()'],
'string': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == ePdfDataType_String || ret->podofo()->GetDataType() == ePdfDataType_HexString'],
'text': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == ePdfDataType_String || ret->podofo()->GetDataType() == ePdfDataType_HexString'],
'text string': ['std::string', 'StringFromDictionary', datatypes.PdfString('""'), 'ret->podofo()->GetDataType() == ePdfDataType_String || ret->podofo()->GetDataType() == ePdfDataType_HexString'],
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
    types = types.replace('or', ' ')
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
    self.fCCPublic = []
    self.fCCPrivate = []
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
    
  def carbonCopyPublic(self, cc):
    self.fCCPublic.append(cc)
    return self 

  def carbonCopyPrivate(self, cc):
    self.fCCPrivate.append(cc)
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


  def writeAsNull(self, fileClass, cls, enumToCls):
    fileClass.write('  virtual SkPdf' + cls.fName +'* as' + cls.fName + '() {return NULL;}\n')
    fileClass.write('  virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return NULL;}\n')
    fileClass.write('\n')

    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeAsNull(fileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
      cnt = cnt + 1

       
  def writeAsFoo(self, fileClass, cls, enumToCls):
    # TODO(edisonn): add a container, with sections, public, private, default, ...
    # the end code will be grouped
    
    # me
    fileClass.write('public:\n')
    fileClass.write('  virtual SkPdf' + cls.fName +'* as' + cls.fName + '() {return this;}\n')
    fileClass.write('  virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return this;}\n')
    fileClass.write('\n')

    if cls.fName == 'Object':
      cnt = 0
      for sub in cls.fEnumSubclasses:
        self.writeAsNull(fileClass, enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
            
    if cls.fName != 'Object':
      fileClass.write('private:\n')
      base = self.fClasses[cls.fBase]
      cnt = 0
      for sub in base.fEnumSubclasses:
        if enumToCls[base.fEnumSubclasses[cnt]].fName != cls.fName:
          self.writeAsNull(fileClass, enumToCls[base.fEnumSubclasses[cnt]], enumToCls)
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
    global fileHeaders 
    global knowTypes
  
    # generate enum
    enumsRoot = []

    enumToCls = {}
    
    for name in self.fClasses:
      cls = self.fClasses[name]
      cls.fEnum = 'k' + name + '_SkPdfObjectType'
      cls.fEnumEnd = 'k' + name + '__End_SkPdfObjectType'

      fileHeaders.write('#include "SkPdf' + cls.fName + '_autogen.h"\n')
            
      if cls.fBase != '':
        self.fClasses[cls.fBase].fEnumSubclasses.append(cls.fEnum)

      if cls.fBase == '':
        enumsRoot.append(cls.fEnum)
       
      enumToCls[cls.fEnum] = cls
      
    enumsRoot.sort()
    
   
    # TODO(edisonn): move each .h in it's own file
    # write imports
    
    # write enums
    fileEnums = open('SkPdfEnums_autogen.h', 'w')
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
      
      fileClass = open('SkPdf' + cls.fName + '_autogen.h', 'w')
      fileClass.write('#ifndef __DEFINED__SkPdf' + cls.fName + '\n')
      fileClass.write('#define __DEFINED__SkPdf' + cls.fName + '\n')
      fileClass.write('\n')

      fileClass.write('#include "SkPdfEnums_autogen.h"\n')
      fileClass.write('#include "SkPdfArray_autogen.h"\n')
      if cls.fBase != '':
        fileClass.write('#include "SkPdf' + cls.fBase + '_autogen.h"\n')
      fileClass.write('\n')
      
      if cls.fComment != '':
        fileClass.write('// ' + cls.fComment + '\n')
      
      if cls.fBase == '':
        fileClass.write('class SkPdf' + cls.fName + ' {\n')
      else:
        fileClass.write('class SkPdf' + cls.fName + ' : public SkPdf' + cls.fBase + ' {\n')
      
      fileClass.write('public:\n')
      fileClass.write('  virtual SkPdfObjectType getType() const { return ' + cls.fEnum + ';}\n')
      if len(cls.fEnumSubclasses) == 0:
        fileClass.write('  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(' + cls.fEnum + ' + 1);}\n')
      else:
        fileClass.write('  virtual SkPdfObjectType getTypeEnd() const { return ' + cls.fEnumEnd + ';}\n')
      
      self.writeAsFoo(fileClass, cls, enumToCls)
      
      fileClass.write('public:\n')
      for cc in cls.fCCPublic:
        fileClass.write('  ' + cc + '\n')
    
      fileClass.write('private:\n')
      for cc in cls.fCCPrivate:
        fileClass.write('  ' + cc + '\n')

      if cls.fBase == '':
        fileClass.write('protected:\n')
        fileClass.write('  const PdfMemDocument* fPodofoDoc;\n')
        fileClass.write('  const PdfObject* fPodofoObj;\n')
        fileClass.write('\n')
        fileClass.write('public:\n')
        fileClass.write('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : fPodofoDoc(podofoDoc), fPodofoObj(podofoObj) {}\n')
        fileClass.write('  const PdfMemDocument* doc() const { return fPodofoDoc;}\n')
        fileClass.write('  const PdfObject* podofo() const { return fPodofoObj;}\n')
      else:
        fileClass.write('public:\n')
        fileClass.write('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdf' + cls.fBase + '(podofoDoc, podofoObj) {}\n')
        fileClass.write('\n')
      
      #check required fieds, also, there should be an internal_valid() manually wrote for complex
      # situations
      # right now valid return true      
      fileClass.write('  virtual bool valid() const {return true;}\n')
      fileClass.write('\n')
      
      fileClass.write('  SkPdf' + cls.fName + '& operator=(const SkPdf' + cls.fName + '& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}\n')
      fileClass.write('\n')
      
      for field in cls.fFields:
        prop = field.fProp
        if prop.fCppName != '':
          
          lines = prop.fComment.split('\n')
          if prop.fComment != '' and len(lines) > 0:
            fileClass.write('/** ' + lines[0] + '\n')
            for line in lines[1:]:
              fileClass.write(' *  ' + line + '\n')
            fileClass.write('**/\n')
          
          if prop.fCppName[0] == '[':
            fileClass.write('/*\n')  # comment code of the atributes that can have any name
          
          # TODO(edisonn): has_foo();  
          fileClass.write('  bool has_' + prop.fCppName + '() const {\n')
          fileClass.write('    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", NULL));\n')
          fileClass.write('  }\n') 
          fileClass.write('\n') 

          if len(prop.fTypes.split()) == 1:
            t = prop.fTypes.strip()
            fileClass.write('  ' + knowTypes[t][0] + ' ' + prop.fCppName + '() const {\n')
            fileClass.write('    ' + knowTypes[t][0] + ' ret;\n')
            fileClass.write('    if (' + knowTypes[t][1] + '(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
            if field.fRequired == False and prop.fDefault != '':
              fileClass.write('    return ' + prop.fDefault.toCpp() + ';\n');
            else:
              fileClass.write('    // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
              fileClass.write('    return ' + knowTypes[t][2].toCpp() + ';\n');
            fileClass.write('  }\n') 
            fileClass.write('\n')
          else:
            for type in prop.fTypes.split():
              t = type.strip()
              fileClass.write('  bool is' + prop.fCppName + 'A' + t.title() + '() const {\n')
              fileClass.write('    SkPdfObject* ret = NULL;\n')
              fileClass.write('    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return false;\n')
              fileClass.write('    return ' + knowTypes[t][3] + ';\n')
              fileClass.write('  }\n')
              fileClass.write('\n')

              fileClass.write('  ' + knowTypes[t][0] + ' get' + prop.fCppName + 'As' + t.title() + '() const {\n')
              fileClass.write('    ' + knowTypes[t][0] + ' ret = ' + knowTypes[t][2].toCpp() + ';\n')
              fileClass.write('    if (' + knowTypes[t][1] + '(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;\n')
              fileClass.write('    // TODO(edisonn): warn about missing required field, assert for known good pdfs\n')
              fileClass.write('    return ' + knowTypes[t][2].toCpp() + ';\n')
              fileClass.write('  }\n') 
              fileClass.write('\n')
               
           
          if prop.fCppName[0] == '[':
            fileClass.write('*/\n')  # comment code of the atributes that can have any name
         

      fileClass.write('};\n')
      fileClass.write('\n')

      fileClass.write('#endif  // __DEFINED__SkPdf' + cls.fName + '\n')
      fileClass.close()
    
      
    
      # generate constructor when knowing the type
      # later, p2, generate constructor when not knowing the type - very similar with parsing?
      
    # generate parser  
    # TODO(edisonn): fast recognition based on must attributes.
    fileMapper = open('SkPdfPodofoMapper_autogen.h', 'w')
    fileMapper.write('#ifndef __DEFINED__SkPdfPodofoMapper\n')
    fileMapper.write('#define __DEFINED__SkPdfPodofoMapper\n')
    fileMapper.write('\n')

    fileMapper.write('#include "SkPdfHeaders_autogen.h"\n')
    fileMapper.write('class PodofoMapper {\n')
    fileMapper.write('public:\n')
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      

      fileMapper.write('  static bool map(const SkPdfObject& in, SkPdf' + name + '** out) {\n')
      fileMapper.write('    return map(*in.doc(), *in.podofo(), out);\n')
      fileMapper.write('  }\n') 
      fileMapper.write('\n')

      fileMapper.write('  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdf' + name + '** out) {\n')
      fileMapper.write('    if (!is' + name + '(podofoDoc, podofoObj)) return false;\n')
      fileMapper.write('\n')

      # stream must be last one
      hasStream = False
      for sub in cls.fEnumSubclasses:
        if cls.fName == 'Object' and enumToCls[sub].fName == 'Stream':
          hasStream = True
        else:
          fileMapper.write('    if (map(podofoDoc, podofoObj, (SkPdf' + enumToCls[sub].fName + '**)out)) return true;\n')
      
      if hasStream:
        fileMapper.write('    if (map(podofoDoc, podofoObj, (SkPdfStream**)out)) return true;\n')
      

      fileMapper.write('\n')
      
      fileMapper.write('    *out = new SkPdf' + name + '(&podofoDoc, &podofoObj);\n')
      fileMapper.write('    return true;\n')        
      fileMapper.write('  }\n') 
      fileMapper.write('\n')
       
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      fileMapper.write('  static bool is' + name + '(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {\n')
      
      if cls.fCheck != '':
        fileMapper.write('    return ' + cls.fCheck + ';\n')
      else:
        cntMust = 0
        for field in cls.fFields:
          prop = field.fProp
          if prop.fHasMust:
            cntMust = cntMust + 1
            fileMapper.write('    ' + knowTypes[prop.fTypes.strip()][0] + ' ' + prop.fCppName + ';\n')
            fileMapper.write('    if (!' + knowTypes[prop.fTypes.strip()][1] + '(&podofoDoc, podofoObj.GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &' + prop.fCppName + ')) return false;\n')
            
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
              fileMapper.write('    if (' + eval + ') return false;\n')
              fileMapper.write('\n')
      
        fileMapper.write('    return true;\n')
              
      fileMapper.write('  }\n') 
      fileMapper.write('\n')    
    
    fileMapper.write('};\n') 
    fileMapper.write('\n')
    
    fileMapper.write('#endif  // __DEFINED__SkPdfPodofoMapper\n')
    fileMapper.close()
    
    return

def generateCode():
  global fileHeaders 

  fileHeaders = open('SkPdfHeaders_autogen.h', 'w')
  fileHeaders.write('#ifndef __DEFINED__SkPdfHeaders\n')
  fileHeaders.write('#define __DEFINED__SkPdfHeaders\n')
  fileHeaders.write('\n')
  
  fileHeaders.write('#include "SkPdfEnums_autogen.h"\n')

  manager = PdfClassManager()
  
  manager.addClass('Object')
  
  manager.addClass('Null').check('podofoObj.GetDataType() == ePdfDataType_Null')
  manager.addClass('Boolean').check('podofoObj.GetDataType() == ePdfDataType_Bool')\
                             .carbonCopyPublic('bool value() const {return fPodofoObj->GetBool();}')
                             
  manager.addClass('Integer').check('podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real')\
                             .carbonCopyPublic('long value() const {return fPodofoObj->GetNumber();}')
  
  manager.addClass('Number', 'Integer').check('podofoObj.GetDataType() == ePdfDataType_Number || podofoObj.GetDataType() == ePdfDataType_Real')\
                             .carbonCopyPublic('double value() const {return fPodofoObj->GetReal();}')
  
  manager.addClass('Name').check('podofoObj.GetDataType() == ePdfDataType_Name')\
                             .carbonCopyPublic('const std::string& value() const {return fPodofoObj->GetName().GetName();}')
  
  manager.addClass('Reference').check('podofoObj.GetDataType() == ePdfDataType_Reference')
  
  manager.addClass('Array').check('podofoObj.GetDataType() == ePdfDataType_Array')\
                             .carbonCopyPublic('const int size() const {return fPodofoObj->GetArray().GetSize();}')\
                             .carbonCopyPublic('SkPdfObject* operator[](int i) const {return new SkPdfObject(fPodofoDoc, &fPodofoObj->GetArray()[i]);}')\
  
  manager.addClass('String').check('podofoObj.GetDataType() == ePdfDataType_String || podofoObj.GetDataType() == ePdfDataType_HexString')\
                             .carbonCopyPublic('const std::string& value() const {return fPodofoObj->GetString().GetStringUtf8();}')
                             
  manager.addClass('HexString', 'String').check('podofoObj.GetDataType() == ePdfDataType_HexString')\
                             .carbonCopyPublic('const std::string& value() const {return fPodofoObj->GetString().GetStringUtf8();}')
  
  manager.addClass('Dictionary').check('podofoObj.GetDataType() == ePdfDataType_Dictionary')\
                                .carbonCopyPublic('const SkPdfObject get(const char* dictionaryKeyName) const {return SkPdfObject(fPodofoDoc, resolveReferenceObject(fPodofoDoc, fPodofoObj->GetDictionary().GetKey(PdfName(dictionaryKeyName))));}')\
                                .carbonCopyPublic('SkPdfObject get(const char* dictionaryKeyName) {return SkPdfObject(fPodofoDoc, resolveReferenceObject(fPodofoDoc, fPodofoObj->GetDictionary().GetKey(PdfName(dictionaryKeyName))));}')\

  manager.addClass('Stream')  # attached to a dictionary in podofo
  
  
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
  
  pdfspec_autogen.buildPdfSpec(manager)  

  manager.addClass('MultiMasterFontDictionary', 'Type1FontDictionary')\
          .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('')\
          .must([datatypes.PdfName('MMType1')])\
          .done().done()\


  manager.write()
  
  fileHeaders.write('#endif  // __DEFINED__SkPdfHeaders\n')
  fileHeaders.close()
  
  return 1

if '__main__' == __name__:
  sys.exit(generateCode())

