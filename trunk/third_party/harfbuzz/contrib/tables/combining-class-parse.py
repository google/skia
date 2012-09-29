import sys
from unicode_parse_common import *

# http://www.unicode.org/Public/5.1.0/ucd/extracted/DerivedCombiningClass.txt

class IdentityMap(object):
  def __getitem__(_, key):
    return key

def main(infile, outfile):
  ranges = unicode_file_parse(infile, IdentityMap(), '0')
  ranges = sort_and_merge(ranges)

  print >>outfile, '// Generated from Unicode tables\n'
  print >>outfile, '#ifndef COMBINING_PROPERTIES_H_'
  print >>outfile, '#define COMBINING_PROPERTIES_H_\n'
  print >>outfile, '#include <stdint.h>'
  print >>outfile, 'struct combining_property {'
  print >>outfile, '  uint32_t range_start;'
  print >>outfile, '  uint32_t range_end;'
  print >>outfile, '  uint8_t klass;'
  print >>outfile, '};\n'
  print >>outfile, 'static const struct combining_property combining_properties[] = {'
  for (start, end, value) in ranges:
    print >>outfile, '  {0x%x, 0x%x, %s},' % (start, end, value)
  print >>outfile, '};\n'
  print >>outfile, 'static const unsigned combining_properties_count = %d;\n' % len(ranges)
  print >>outfile, '#endif  // COMBINING_PROPERTIES_H_'

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: %s <input .txt> <output .h>' % sys.argv[0]
  else:
    main(file(sys.argv[1], 'r'), file(sys.argv[2], 'w+'))
