
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

class PdfNumber:
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
    
class PdfDateNever:
  def toCpp(self):
    return 'SkPdfDate()'

class FileSpecNone:
  def toCpp(self):
    return 'SkPdfFileSpec()'

class PdfEmptyRect:
  def toCpp(self):
    return 'SkRect()'
    
class PdfEmptyStream:
  def toCpp(self):
    return 'SkPdfStream()'
    
class PdfArrayNone:
  def toCpp(self):
    return 'SkPdfArray()'
    
class PdfFunctionNone:
  def toCpp(self):
    return 'SkPdfFunction()'
    

    