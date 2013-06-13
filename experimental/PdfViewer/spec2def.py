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
columnValues = ['', '', '']
mustFollowTableHeader = False

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
      
  print spec
  print specs
  print required
  print inheritable
  print version
  print columnValues
  
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
  commitRow()
  tableHeaderFound = False
    

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
  print lines

if '__main__' == __name__:
  sys.exit(generateDef())