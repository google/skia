import sys
from unicode_parse_common import *

# http://www.unicode.org/Public/5.1.0/ucd/Scripts.txt

script_to_harfbuzz = {
  # This is the list of HB_Script_* at the time of writing
  'Common': 'HB_Script_Common',
  'Greek': 'HB_Script_Greek',
  'Cyrillic': 'HB_Script_Cyrillic',
  'Armenian': 'HB_Script_Armenian',
  'Hebrew': 'HB_Script_Hebrew',
  'Arabic': 'HB_Script_Arabic',
  'Syriac': 'HB_Script_Syriac',
  'Thaana': 'HB_Script_Thaana',
  'Devanagari': 'HB_Script_Devanagari',
  'Bengali': 'HB_Script_Bengali',
  'Gurmukhi': 'HB_Script_Gurmukhi',
  'Gujarati': 'HB_Script_Gujarati',
  'Oriya': 'HB_Script_Oriya',
  'Tamil': 'HB_Script_Tamil',
  'Telugu': 'HB_Script_Telugu',
  'Kannada': 'HB_Script_Kannada',
  'Malayalam': 'HB_Script_Malayalam',
  'Sinhala': 'HB_Script_Sinhala',
  'Thai': 'HB_Script_Thai',
  'Lao': 'HB_Script_Lao',
  'Tibetan': 'HB_Script_Tibetan',
  'Myanmar': 'HB_Script_Myanmar',
  'Georgian': 'HB_Script_Georgian',
  'Hangul': 'HB_Script_Hangul',
  'Ogham': 'HB_Script_Ogham',
  'Runic': 'HB_Script_Runic',
  'Khmer': 'HB_Script_Khmer',
  'Inherited': 'HB_Script_Inherited',
}

class ScriptDict(object):
  def __init__(self, base):
    self.base = base

  def __getitem__(self, key):
    r = self.base.get(key, None)
    if r is None:
      return 'HB_Script_Common'
    return r

def main(infile, outfile):
  ranges = unicode_file_parse(infile,
                              ScriptDict(script_to_harfbuzz),
                              'HB_Script_Common')
  ranges = sort_and_merge(ranges)

  print >>outfile, '// Generated from Unicode script tables\n'
  print >>outfile, '#ifndef SCRIPT_PROPERTIES_H_'
  print >>outfile, '#define SCRIPT_PROPERTIES_H_\n'
  print >>outfile, '#include <stdint.h>'
  print >>outfile, '#include "harfbuzz-shaper.h"\n'
  print >>outfile, 'struct script_property {'
  print >>outfile, '  uint32_t range_start;'
  print >>outfile, '  uint32_t range_end;'
  print >>outfile, '  HB_Script script;'
  print >>outfile, '};\n'
  print >>outfile, 'static const struct script_property script_properties[] = {'
  for (start, end, value) in ranges:
    print >>outfile, '  {0x%x, 0x%x, %s},' % (start, end, value)
  print >>outfile, '};\n'
  print >>outfile, 'static const unsigned script_properties_count = %d;\n' % len(ranges)
  print >>outfile, '#endif  // SCRIPT_PROPERTIES_H_'

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: %s <input .txt> <output .h>' % sys.argv[0]
  else:
    main(file(sys.argv[1], 'r'), file(sys.argv[2], 'w+'))
