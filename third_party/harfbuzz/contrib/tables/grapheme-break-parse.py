import sys
from unicode_parse_common import *

# http://www.unicode.org/Public/UNIDATA/auxiliary/GraphemeBreakProperty.txt

property_to_harfbuzz = {
  'CR': 'HB_Grapheme_CR',
  'LF': 'HB_Grapheme_LF',
  'Control': 'HB_Grapheme_Control',
  'Extend': 'HB_Grapheme_Extend',
  'Prepend': 'HB_Grapheme_Other',
  'SpacingMark': 'HB_Grapheme_Other',
  'L': 'HB_Grapheme_L',
  'V': 'HB_Grapheme_V',
  'T': 'HB_Grapheme_T',
  'LV': 'HB_Grapheme_LV',
  'LVT': 'HB_Grapheme_LVT',
}

def main(infile, outfile):
  ranges = unicode_file_parse(infile, property_to_harfbuzz)
  ranges.sort()

  print >>outfile, '// Generated from Unicode Grapheme break tables\n'
  print >>outfile, '#ifndef GRAPHEME_BREAK_PROPERTY_H_'
  print >>outfile, '#define GRAPHEME_BREAK_PROPERTY_H_\n'
  print >>outfile, '#include <stdint.h>'
  print >>outfile, '#include "harfbuzz-external.h"\n'
  print >>outfile, 'struct grapheme_break_property {'
  print >>outfile, '  uint32_t range_start;'
  print >>outfile, '  uint32_t range_end;'
  print >>outfile, '  HB_GraphemeClass klass;'
  print >>outfile, '};\n'
  print >>outfile, 'static const struct grapheme_break_property grapheme_break_properties[] = {'
  for (start, end, value) in ranges:
    print >>outfile, '  {0x%x, 0x%x, %s},' % (start, end, value)
  print >>outfile, '};\n'
  print >>outfile, 'static const unsigned grapheme_break_properties_count = %d;\n' % len(ranges)
  print >>outfile, '#endif  // GRAPHEME_BREAK_PROPERTY_H_'

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: %s <input .txt> <output .h>' % sys.argv[0]
  else:
    main(file(sys.argv[1], 'r'), file(sys.argv[2], 'w+'))
