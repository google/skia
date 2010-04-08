import sys
from unicode_parse_common import *

# http://www.unicode.org/Public/5.1.0/ucd/extracted/DerivedGeneralCategory.txt

category_to_harfbuzz = {
  'Mn': 'HB_Mark_NonSpacing',
  'Mc': 'HB_Mark_SpacingCombining',
  'Me': 'HB_Mark_Enclosing',

  'Nd': 'HB_Number_DecimalDigit',
  'Nl': 'HB_Number_Letter',
  'No': 'HB_Number_Other',

  'Zs': 'HB_Separator_Space',
  'Zl': 'HB_Separator_Line',
  'Zp': 'HB_Separator_Paragraph',

  'Cc': 'HB_Other_Control',
  'Cf': 'HB_Other_Format',
  'Cs': 'HB_Other_Surrogate',
  'Co': 'HB_Other_PrivateUse',
  'Cn': 'HB_Other_NotAssigned',

  'Lu': 'HB_Letter_Uppercase',
  'Ll': 'HB_Letter_Lowercase',
  'Lt': 'HB_Letter_Titlecase',
  'Lm': 'HB_Letter_Modifier',
  'Lo': 'HB_Letter_Other',

  'Pc': 'HB_Punctuation_Connector',
  'Pd': 'HB_Punctuation_Dash',
  'Ps': 'HB_Punctuation_Open',
  'Pe': 'HB_Punctuation_Close',
  'Pi': 'HB_Punctuation_InitialQuote',
  'Pf': 'HB_Punctuation_FinalQuote',
  'Po': 'HB_Punctuation_Other',

  'Sm': 'HB_Symbol_Math',
  'Sc': 'HB_Symbol_Currency',
  'Sk': 'HB_Symbol_Modifier',
  'So': 'HB_Symbol_Other',
}

def main(infile, outfile):
  ranges = unicode_file_parse(infile, category_to_harfbuzz)
  ranges = sort_and_merge(ranges)

  print >>outfile, '// Generated from Unicode script tables\n'
  print >>outfile, '#ifndef CATEGORY_PROPERTIES_H_'
  print >>outfile, '#define CATEGORY_PROPERTIES_H_\n'
  print >>outfile, '#include <stdint.h>'
  print >>outfile, '#include "harfbuzz-external.h"\n'
  print >>outfile, 'struct category_property {'
  print >>outfile, '  uint32_t range_start;'
  print >>outfile, '  uint32_t range_end;'
  print >>outfile, '  HB_CharCategory category;'
  print >>outfile, '};\n'
  print >>outfile, 'static const struct category_property category_properties[] = {'
  for (start, end, value) in ranges:
    print >>outfile, '  {0x%x, 0x%x, %s},' % (start, end, value)
  print >>outfile, '};\n'
  print >>outfile, 'static const unsigned category_properties_count = %d;\n' % len(ranges)
  print >>outfile, '#endif  // CATEGORY_PROPERTIES_H_'

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: %s <input .txt> <output .h>' % sys.argv[0]
  else:
    main(file(sys.argv[1], 'r'), file(sys.argv[2], 'w+'))
