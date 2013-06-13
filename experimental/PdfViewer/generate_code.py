import sys

class PdfName:
  def __init__(self, name, abr=''):
    self.fName = name
    self.fAbr = abr
    
  def toCpp(self):
    return '\"' + self.fName + '\"'

class PdfString:
  def __init__(self, value):
    self.fValue = value
    
  def toCpp(self):
    return '\"' + self.fValue + '\"'

class PdfInteger:
  def __init__(self, value):
    self.fValue = value

  def toCpp(self):
    return str(self.fValue)

class PdfReal:
  def __init__(self, value):
    self.fValue = value

  def toCpp(self):
    return str(self.fValue)

class PdfString:
  def __init__(self, value):
    self.fValue = value

  def toCpp(self):
    return self.fValue

class PdfBoolean:
  def __init__(self, value):
    self.fValue = value

  def toCpp(self):
    return self.fValue

class CppNull:
  def toCpp(self):
    return 'NULL'


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

  def real(self, name):
    self.fType = 'real'
    self.fCppName = name
    self.fCppType = 'double'
    self.fCppReader = 'DoubleFromDictionary'
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
    self.fDictionaryType = 'Resources'  # TODO(edisonn): Dictionary?
    self.fCppReader = 'DictionaryFromDictionary'
    self.fDefault = CppNull()
    return self

  def type(self, type):
    # TODO (edisonn): if simple type, use it, otherwise set it to Dictionary, and set a mask for valid types, like array or name
    self.fType = 'dictionary'
    self.fDictionaryType = 'Dictionary'
    self.fCppReader = 'DictionaryFromDictionary'
    self.fDefault = CppNull()
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
        print('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : fPodofoDoc(podofoDoc), fPodofoObj(podofoObj) {}')
        print('  const PdfObject* podofo() const { return fPodofoObj;}')
      else:
        print('public:')
        print('  SkPdf' + cls.fName + '(const PdfMemDocument* podofoDoc, const PdfObject* podofoObj) : SkPdf' + cls.fBase + '(podofoDoc, podofoObj) {}')
        print
      
      #check required fieds, also, there should be an internal_valid() manually wrote for complex
      # situations
      # right now valid return true      
      print('  virtual bool valid() const {return true;}')
      print
      
      for field in cls.fFields:
        prop = field.fProp
        if prop.fCppName != '':
          if prop.fType != 'dictionary':
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
         
          if prop.fType == 'dictionary':
            print('  SkPdf' + prop.fDictionaryType + '* ' + prop.fCppName + '() const {')
            print('    SkPdfObject* dict = NULL;')
            print('    if (' + prop.fCppReader + '(fPodofoDoc, fPodofoObj->GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &dict) && dict != NULL) {')
            print('      SkPdf' + prop.fDictionaryType + '* ret = new SkPdf' + prop.fDictionaryType + '(fPodofoDoc, dict->podofo());')
            print('      delete dict; dict = NULL;')
            print('      return ret;')
            print('    }')
            if field.fRequired == False:
              print('    return ' + prop.fDefault.toCpp() + ';');
            if field.fRequired == True:
              print('    // TODO(edisonn): warn about missing required field, assert for known good pdfs')
              print('    return ' + field.fBadDefault + ';');
            print('  }') 
            print
           
         

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
      
      print('  static bool map' + name + '(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {')
      print('    if (!isA' + name + '(podofoDoc, podofoObj)) return false;')
      print
      
      for sub in cls.fEnumSubclasses:
        print('    if (map' + enumToCls[sub].fName + '(podofoDoc, podofoObj, out)) return true;')

      print
      
      print('    *out = new SkPdf' + name + '(&podofoDoc, &podofoObj);')
      print('    return true;')        
      print('  }') 
      print
       
    for name in self.fClassesNamesInOrder:
      cls = self.fClasses[name]
      
      print('  static bool isA' + name + '(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj) {')
      
      cntMust = 0
      for field in cls.fFields:
        prop = field.fProp
        if prop.fHasMust:
          cntMust = cntMust + 1
          print('    ' + prop.fCppType + ' ' + prop.fCppName + ';')
          print('    if (!' + prop.fCppReader + '(&podofoDoc, podofoObj.GetDictionary(), \"' + prop.fName + '\", \"' + prop.fAbr + '\", &' + prop.fCppName + ')) return false;')
          print('    if (' + prop.fCppName + ' != ' + prop.fMustBe.toCpp() + ') return false;')
          print
      
      # hack, we only care about dictionaries now, so ret tru only if there is a match
      if cntMust != 0 or name == 'Object' or name == 'Dictionary':
        print('    return true;')
      else:
        print('    return false;')
              
      print('  }') 
      print    
    
    print('};') 
    print
    
    return

def generateCode():
  all = PdfClassManager()
  
  all.addClass('Object')
  all.addClass('Null')
  all.addClass('Boolean')
  all.addClass('Integer')
  all.addClass('Real')
  all.addClass('Name')
  all.addClass('Stream')
  all.addClass('Reference')
  all.addClass('Array')
  all.addClass('Dictionary').optional().field('Resources', '').dictionary("r") #.inherited_from_page_tree()

  all.addClass('Resources', 'Dictionary')

  all.addClass('XObject', 'Dictionary').required('""').field('Type').must(PdfName('XObject')).name('t')
  
  all.addClass('Image', 'XObject').required('""').field('Type').must(PdfName('XObject')).name('t').done()\
                                                            .done()\
                                  .required('""').field('Subtype').must(PdfName('Image')).name('s').done()\
                                                               .done()\
                                  .required('-1').field('Width', 'W').integer('w').done()\
                                                                   .done()\
                                  .required('-1').field('Height', 'H').integer('h').done()\
                                                                    .done()\
                                  .required('""').field('ColorSpace').name('cs').multiple([PdfName('/DeviceRGB', '/RGB'), PdfName('/DeviceGray', '/Gray')]).done()\
                                                                  .done()\
                                  .optional().field('BitsPerComponent', 'BPC').integer('bpc').multiple([PdfInteger(1), PdfInteger(2), PdfInteger(4), PdfInteger(8)])\
                                                                                .default(PdfInteger(1)).done()\
                                                                                .done()\
                                  .carbonCopyPrivate('SkBitmap bitmap;')

  all.addClass('Form', 'XObject').required('""').field('Type').must(PdfName('XObject')).name('t').done()\
                                                           .done()\
                                 .required('""').field('Subtype').must(PdfName('Form')).name('s').done()\
                                                              .done()\
                                 .carbonCopyPublic('void test() {}')



  all.addClass('SpecificToATrapNetworkAppearanceStream', 'Dictionary', 'Additional entries specific to a trap network appearance stream')\
      .required('NULL')\
          .field('PCM')\
          .name('PCM')\
          .type('name')\
          .comment('(Required) The name of the process color model that was assumed when this trap network was created; equivalent to the PostScript page device parameter ProcessColorModel (see Section 6.2.5 of the PostScript Language Reference, Third Edition). Valid values are DeviceGray, DeviceRGB, DeviceCMYK, DeviceCMY, DeviceRGBK, and DeviceN.')\
          .done().done()\
      .optional()\
          .field('SeparationColorNames')\
          .name('SeparationColorNames')\
          .type('array')\
          .comment('(Optional) An array of names identifying the colorants that were assumed when this network was created; equivalent to the Post- Script page device parameter of the same name (see Section 6.2.5 of the PostScript Language Reference, Third Edition). Colorants im- plied by the process color model PCM are available automatically and need not be explicitly declared. If this entry is absent, the colorants implied by PCM are assumed.')\
          .done().done()\
      .optional()\
          .field('TrapRegions')\
          .name('TrapRegions')\
          .type('array')\
          .comment('(Optional) An array of indirect references to TrapRegion objects defining the page\'s trapping zones and the associated trapping parameters, as described in Adobe Technical Note #5620, Portable Job Ticket Format. These references are to objects comprising portions of a PJTF job ticket that is embedded in the PDF file. When the trapping zones and parameters are defined by an external job ticket (or by some other means, such as with JDF), this entry is absent.')\
          .done().done()\
      .optional()\
          .field('TrapStyles')\
          .name('TrapStyles')\
          .type('text string')\
          .comment('(Optional) A human-readable text string that applications can use to describe this trap network to the user (for example, to allow switching between trap networks).')\
          .done().done()\
      .done()

  all.addClass('OpiVersionDictionary', 'Dictionary', 'Entry in an OPI version dictionary')\
      .required('NULL')\
          .field('version_number')\
          .name('version_number')\
          .type('dictionary')\
          .comment('(Required; PDF 1.2) An OPI dictionary specifying the attributes of this proxy (see Tables 9.50 and 9.51). The key for this entry must be the name 1.3 or 2.0, identifying the version of OPI to which the proxy corresponds.')\
          .done().done()\
      .done()



  all.write()
  
  return 1

if '__main__' == __name__:
  sys.exit(generateCode())

  