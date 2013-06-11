import sys

class PdfName:
  def __init__(self, name, abr=''):
    self.fName = name
    self.fAbr = abr

class PdfInteger:
  def __init__(self, value):
    self.fValue = value

class PdfReal:
  def __init__(self, value):
    self.fValue = value

class PdfString:
  def __init__(self, value):
    self.fValue = value

class PdfBoolean:
  def __init__(self, value):
    self.fValue = value

class PdfField:
  def __init__(self, parent, name, abr):
    self.fParent = parent
    self.fName = name
    self.fAbr = abr
    
    self.fDefault = ''
    self.fType = ''

  def must(self, value):
    return self.fParent
    
  def default(self, value):
    self.fDefault = value
    return self
    
  def number(self):
    self.fType = 'number'
    return self
    
  def integer(self):
    self.fType = 'integer'
    return self

  def real(self):
    self.fType = 'real'
    return self

  def name(self):
    self.fType = 'name'
    return self
    
  def string(self):
    self.fType = 'string'
    return self
    
  def multiple(self, options):
    self.fType = 'multiple'
    self.fOptions = options
    return self
    
  def done(self):
    return self.fParent


class PdfClassField:
  def __init__(self, parent, required):
    self.fFields = []
    self.fIncludes = []
    self.fCC = []
    self.fParent = parent
    self.fRequired = required
    
  def hasField(self, name, abr=''):
    return PdfField(self, name, abr)
    
  def done(self):
    return self.fParent

class PdfClass:
  def __init__(self, name, base):
    self.fFields = []
    self.fIncludes = []
    self.fCC = []
    self.fName = name
    self.fBase = base
    
  def required(self):
    field = PdfClassField(self, True)
    self.fFields.append(field)
    return field
    
  def optional(self):
    field = PdfClassField(self, False)
    self.fFields.append(field)
    return field
    
  def include(self, path):
    self.fIncludes.append(path)
    return self
    
  def carbonCopy(self, cc):
    self.fCC.append(cc)
    return self 

class PdfClassManager:
  def __init__(self):
    self.fClasses = []

  def addClass(self, name, base=''):
    cls = PdfClass(name, base)
    self.fClasses.append(cls)
    return cls
  
  def write(self):
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
  all.addClass('Dictionary')

  all.addClass('XObject', 'Dictionary').required().hasField('/Type').must('/XObject')
  
  all.addClass('Image', 'XObject').required().hasField('/Type').must('/XObject').done()\
                                  .required().hasField('/Subtype').must('/Image').done()\
                                  .required().hasField('/Width', '/W').integer().done().done()\
                                  .required().hasField('/Height', '/H').integer().done().done()\
                                  .required().hasField('/ColorSpace').multiple([PdfName('/DeviceRGB', '/RGB'), PdfName('/DeviceGray', '/Gray')])\
                                                                                .done()\
                                                                     .done()\
                                  .optional().hasField('/BitsPerComponent', '/BPC').multiple([PdfInteger(1), PdfInteger(2), PdfInteger(4), PdfInteger(8)])\
                                                                                   .default(PdfInteger(1))\
                                                                                   .done().done()\
                                  .carbonCopy('SkBitmap bitmap;')

  all.addClass('Form', 'XObject').required().hasField('/Type').must('/XObject').done()\
                                 .required().hasField('/Subtype').must('/Form').done()


  all.write()
  
  return 1

if '__main__' == __name__:
  sys.exit(generateCode())
