

import sys

import datatypes
import pdfspec_autogen




class PdfField:
  def __init__(self, parent, name, abr):
    self.fParent = parent
    self.fName = name
    self.fAbr = abr
    
    self.fDefault = ''
    self.fType = ''
    self.fCppName = ''
    self.fCppType = ''
    self.fCppReader = ''
    self.fValidOptions = []
    self.fHasMust = False 
    self.fMustBe = ''

  def must(self, value):
    self.fHasMust = True
    self.fMustBe = value
    return self
    
  def default(self, value):
    self.fDefault = value
    return self
    
  def number(self, name):
    self.fType = 'number'
    self.fCppName = name
    self.fCppType = 'double'
    self.fCppReader = 'DoubleFromDictionary'
    return self
    
  def integer(self, name):
    self.fType = 'integer'
    self.fCppName = name
    self.fCppType = 'long'
    self.fCppReader = 'LongFromDictionary'
    return self

  def name(self, name):
    self.fType = 'name'
    self.fCppName = name
    self.fCppType = 'std::string'
    self.fCppReader = 'NameFromDictionary'
    return self
    
  def string(self, name):
    self.fType = 'string'
    self.fCppName = name
    self.fCppType = 'std::string'
    self.fCppReader = 'StringFromDictionary'
    return self
    
  def multiple(self, validOptions):
    self.fValidOptions = validOptions
    return self
    
  def dictionary(self, name):
    self.fType = 'dictionary'
    self.fCppName = name
    self.fDictionaryType = 'Dictionary'
    self.fCppType = 'SkPdfDictionary*'
    self.fCppReader = 'DictionaryFromDictionary'
    self.fDefault = datatypes.CppNull()
    return self

  def type(self, type):
    # TODO (edisonn): if simple type, use it, otherwise set it to Dictionary, and set a mask for valid types, like array or name
    type = type.replace('or', ' ')
    type = type.replace(',', ' ')
    type = type.replace('text', ' ') # TODO(edisonn): what is the difference between 'text string' and 'string'?
    
    type = type.strip()
    types = type.split()
    
    if len(types) == 1:
      if type == 'integer':
        self.integer(self.fCppName)
        self.default(datatypes.PdfInteger(0))
        return self
        
      if type == 'number':
        self.number(self.fCppName)
        self.default(datatypes.PdfNumber(0))
        return self

      if type == 'string':
        self.string(self.fCppName)
        self.default(datatypes.PdfString('""'))
        return self

      if type == 'name':
        self.name(self.fCppName)
        self.default(datatypes.PdfName('""'))
        return self
    
      if type == 'dictionary':
        self.dictionary(self.fCppName)
        self.default(datatypes.CppNull())
        return self

    self.fType = 'object'
    self.fDictionaryType = 'Object'
    self.fCppType = 'SkPdfObject*'
    self.fCppReader = 'ObjectFromDictionary'
    self.fDefault = datatypes.CppNull()
    return self

  def comment(self, comment):
    return self
      
  def done(self):
    return self.fParent


class PdfClassField:
  def __init__(self, parent, required, version='', inheritable=False, comment=''):
    #self.fProp = ''
    self.fParent = parent
    self.fRequired = required
    self.fVersion = version
    self.fInheritable = inheritable
    self.fComment = comment
    
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
  
  def longName(self, name):
    #return name
    # TODO(edisonn): we need the long name to nenerate and sort enums, but we can generate them recursively
    ret = ''
    while name != '':
      cls = self.fClasses[name]
      ret = name + ret
      name = cls.fBase
      
    return ret
  
  
  def writeEnum(self, enum, enumToCls):
    print('  ' + enum + ',')
    cls = enumToCls[enum]
    cls.fEnumSubclasses.sort()
    
    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeEnum(cls.fEnumSubclasses[cnt], enumToCls)
      cnt = cnt + 1
      
    if cnt != 0:
       print('  ' + cls.fEnumEnd + ',')


  def writeAsNull(self, cls, enumToCls):
    print('  virtual SkPdf' + cls.fName +'* as' + cls.fName + '() {return NULL;}')
    print('  virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return NULL;}')
    print

    cnt = 0
    for sub in cls.fEnumSubclasses:
      self.writeAsNull(enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
      cnt = cnt + 1

       
  def writeAsFoo(self, cls, enumToCls):
    # TODO(edisonn): add a container, with sections, public, private, default, ...
    # the end code will be grouped
    
    # me
    print('public:')
    print('  virtual SkPdf' + cls.fName +'* as' + cls.fName + '() {return this;}')
    print('  virtual const SkPdf' + cls.fName +'* as' + cls.fName + '() const {return this;}')
    print

    if cls.fName == 'Object':
      cnt = 0
      for sub in cls.fEnumSubclasses:
        self.writeAsNull(enumToCls[cls.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
            
    if cls.fName != 'Object':
      print('private:')
      base = self.fClasses[cls.fBase]
      cnt = 0
      for sub in base.fEnumSubclasses:
        if enumToCls[base.fEnumSubclasses[cnt]].fName != cls.fName:
          self.writeAsNull(enumToCls[base.fEnumSubclasses[cnt]], enumToCls)
        cnt = cnt + 1
      
      
  
  def write(self):
    # generate enum
    enumsRoot = []

    enumToCls = {}
    
    for name in self.fClasses:
      cls = self.fClasses[name]
      enum = self.longName(name)
      cls.fEnum = 'k' + enum + '_SkPdfObjectType'
      cls.fEnumEnd = 'k' + enum + '__End_SkPdfObjectType'
            
      if cls.fBase != '':
        self.fClasses[cls.fBase].fEnumSubclasses.append(cls.fEnum)

      if cls.fBase == '':
        enumsRoot.append(cls.fEnum)
       
      enumToCls[cls.fEnum] = cls
      
    enumsRoot.sort()
    
   
    # TODO(edisonn): move each .h in it's own file
    # write imports
    
    # write enums
    print('enum SkPdfObjectType {')
    for enum in enumsRoot:
      self.writeEnum(enum, enumToCls)
    print('};')
    print
    
    # write forward class declaration
    for name in self.fClassesNamesInOrder:
      print('class SkPdf' + name + ';')
    print
    
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      enum = cls.fEnum
      
      if cls.fBase == '':
        print('class SkPdf' + cls.fName + ' {')
      else:
        print('class SkPdf' + cls.fName + ' : public SkPdf' + cls.fBase + ' {')
      
      print('public:')
      print('  virtual SkPdfObjectType getType() const { return ' + cls.fEnum + ';}')
      if len(cls.fEnumSubclasses) == 0:
        print('  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(' + cls.fEnum + ' + 1);}')
      else:
        print('  virtual SkPdfObjectType getTypeEnd() const { return ' + cls.fEnumEnd + ';}')
      
      
      self.writeAsFoo(cls, enumToCls)
      
      print('public:')
      for cc in cls.fCCPublic:
        print('  ' + cc)
    
      print('private:')
      for cc in cls.fCCPrivate:
        print('  ' + cc)

      if cls.fBase == '':
        print('protected:')
        print('  const PdfMemDocument* fPodofoDoc;')
        print('  const PdfObject* fPodofoObj;')
        print
        print('public:')
        print('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : fPodofoDoc(podofoDoc), fPodofoObj(podofoObj) {}')
        print('  const PdfMemDocument* doc() const { return fPodofoDoc;}')
        print('  const PdfObject* podofo() const { return fPodofoObj;}')
      else:
        print('public:')
        print('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdf' + cls.fBase + '(podofoDoc, podofoObj) {}')
        print
      
      #check required fieds, also, there should be an internal_valid() manually wrote for complex
      # situations
      # right now valid return true      
      print('  virtual bool valid() const {return true;}')
      print
      
      print('  SkPdf' + cls.fName + '& operator=(const SkPdf' + cls.fName + '& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}')
      print
      
      for field in cls.fFields:
        prop = field.fProp
        if prop.fCppName != '':
          if prop.fCppName[0] == '[':
            print('/*')  # comment code of the atributes that can have any name
            
          print('  ' + prop.fCppType + ' ' + prop.fCppName + '() const {')
          print('    ' + prop.fCppType + ' ret;')
          print('    if (' + prop.fCppReader + '(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &ret)) return ret;')
          if field.fRequired == False:
            print('    return ' + prop.fDefault.toCpp() + ';');
          if field.fRequired == True:
            print('    // TODO(edisonn): warn about missing required field, assert for known good pdfs')
            print('    return ' + field.fBadDefault + ';');
          print('  }') 
          print
           
          if prop.fCppName[0] == '[':
            print('*/')  # comment code of the atributes that can have any name
         

      print('};')
      print
      print
    
      
    
      # generate constructor when knowing the type
      # later, p2, generate constructor when not knowing the type - very similar with parsing?
      
    # generate parser  
    
    # TODO(edisonn): fast recognition based on must attributes.
    print('class PodofoMapper {')
    print('public:')
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      

      print('  static bool map(const SkPdfObject& in, SkPdf' + name + '** out) {')
      print('    return map(*in.doc(), *in.podofo(), out);')
      print('  }') 
      print

      print('  static bool map(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdf' + name + '** out) {')
      print('    if (!isA' + name + '(podofoDoc, podofoObj)) return false;')
      print
      
      for sub in cls.fEnumSubclasses:
        print('    if (map(podofoDoc, podofoObj, (SkPdf' + enumToCls[sub].fName + '**)out)) return true;')

      print
      
      print('    *out = new SkPdf' + name + '(&podofoDoc, &podofoObj);')
      print('    return true;')        
      print('  }') 
      print
       
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      print('  static bool isA' + name + '(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {')
      
      if cls.fCheck != '':
        print('    return ' + cls.fCheck + ';')
      else:
        cntMust = 0
        for field in cls.fFields:
          prop = field.fProp
          if prop.fHasMust:
            cntMust = cntMust + 1
            print('    ' + prop.fCppType + ' ' + prop.fCppName + ';')
            print('    if (!' + prop.fCppReader + '(&podofoDoc, podofoObj.GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &' + prop.fCppName + ')) return false;')
            print('    if (' + prop.fCppName + ' != ' + prop.fMustBe.toCpp() + ') return false;')
            print
      
        print('    return true;')
              
      print('  }') 
      print    
    
    print('};') 
    print
    
    return

def generateCode():
  manager = PdfClassManager()
  
  manager.addClass('Object')
  
  manager.addClass('Null').check('podofoObj.GetDataType() == ePdfDataType_Null')
  manager.addClass('Boolean').check('podofoObj.GetDataType() == ePdfDataType_Bool')
  manager.addClass('Integer').check('podofoObj.GetDataType() == ePdfDataType_Number')
  manager.addClass('Number').check('podofoObj.GetDataType() == ePdfDataType_Real')
  manager.addClass('Name').check('podofoObj.GetDataType() == ePdfDataType_Name')
  #manager.addClass('Stream') - attached to a dictionary
  manager.addClass('Reference').check('podofoObj.GetDataType() == ePdfDataType_Reference')
  manager.addClass('Array').check('podofoObj.GetDataType() == ePdfDataType_Array')
  manager.addClass('String').check('podofoObj.GetDataType() == ePdfDataType_String')
  manager.addClass('HexString').check('podofoObj.GetDataType() == ePdfDataType_HexString')
  
  manager.addClass('Dictionary').check('podofoObj.GetDataType() == ePdfDataType_Dictionary')
  
  # these classes are not explicitely backed by a table in the pdf spec
  manager.addClass('XObjectDictionary', 'Dictionary')
  
  manager.addClass('FontDictionary', 'Dictionary')
  
  manager.addClass('TrueTypeFontDictionary', 'FontDictionary')
  
  pdfspec_autogen.buildPdfSpec(manager)  

  manager.addClass('MultiMasterFontDictionary', 'Type1FontDictionary')\
          .required('NULL')\
          .field('Subtype')\
          .name('Subtype')\
          .type('name')\
          .comment('')\
          .must(datatypes.PdfName('MMType1'))\
          .done().done()\


  manager.write()
  
  return 1

if '__main__' == __name__:
  sys.exit(generateCode())

  