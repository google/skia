
# Convert between Visual Studio 2008 and 2005 Project Files
# (with thanks to Tommi Vainikainen)

l05 = "# Visual Studio 2005\n"
l08 = "# Visual Studio 2008\n"
l09 = "Microsoft Visual Studio Solution File, Format Version 9.00\n"
l10 = "Microsoft Visual Studio Solution File, Format Version 10.00\n"

import os, shutil, string, fileinput, sys

def vcproj_convert(sp) :
  for l in fileinput.input(sp, inplace = 1) :
    p8 = l.find("Version=\"8.00\"")
    p9 = l.find("Version=\"9.00\"")
    if p8 != -1 or p9 != -1 :
      if p8 != -1 :
        l = l[ : p8 + 9] + '9' + l[ p8 + 10 : ]
      else :
        l = l[ : p9 + 9] + '8' + l[ p9 + 10 : ]
    sys.stdout.write(l)

def sln_convert(sp) :
  cnt = 0
  for l in fileinput.input(sp, inplace = 1) :
    cnt = cnt + 1
    if cnt < 3 :
      p09 = l.find(l09)
      p10 = l.find(l10)
      if p09 != -1 or p10 != -1 :
        if p09 != -1 :
          l = l10
        else :
          l = l09
      p05 = l.find(l05)
      p08 = l.find(l08)
      if p05 != -1 or p08 != -1 :
        if p05 != -1 :
          l = l08
        else :
          l = l05
    sys.stdout.write(l)

if os.getcwd().endswith('Mkfiles\\vc9') :
  for root, dirs, files in os.walk("./") :
    for file in files :
      if file.endswith(".sln") :
        sln_convert(os.path.join(root, file))
      if file.endswith(".vcproj") :
        vcproj_convert(os.path.join(root, file))        
else :
  print "This script must be run in the 'Mkfiles\vc9' directory"
