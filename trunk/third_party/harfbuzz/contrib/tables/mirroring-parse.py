import sys

# http://www.unicode.org/Public/UNIDATA/auxiliary/BidiMirroring.txt

# This parses a file in the format of the above file and outputs a table
# suitable for bsearch(3). This table maps Unicode code points to their
# 'mirror'. (Mirroring is used when rendering RTL characters, see the Unicode
# standard). By convention, this mapping should be commutative, but this code
# doesn't enforce or check this.

def main(infile, outfile):
  pairs = []
  for line in infile:
    line = line[:-1]
    if len(line) == 0 or line[0] == '#':
      continue
    if '#' in line:
      (data, _) = line.split('#', 1)
    else:
      data = line
    if ';' not in data:
      continue
    (a, b) = data.split(';', 1)
    a = int(a, 16)
    b = int(b, 16)

    pairs.append((a, b))

  pairs.sort()

  print >>outfile, '// Generated from Unicode Bidi Mirroring tables\n'
  print >>outfile, '#ifndef MIRRORING_PROPERTY_H_'
  print >>outfile, '#define MIRRORING_PROPERTY_H_\n'
  print >>outfile, '#include <stdint.h>'
  print >>outfile, 'struct mirroring_property {'
  print >>outfile, '  uint32_t a;'
  print >>outfile, '  uint32_t b;'
  print >>outfile, '};\n'
  print >>outfile, 'static const struct mirroring_property mirroring_properties[] = {'
  for pair in pairs:
    print >>outfile, '  {0x%x, 0x%x},' % pair
  print >>outfile, '};\n'
  print >>outfile, 'static const unsigned mirroring_properties_count = %d;\n' % len(pairs)
  print >>outfile, '#endif  // MIRRORING_PROPERTY_H_'

if __name__ == '__main__':
  if len(sys.argv) != 3:
    print 'Usage: %s <input .txt> <output .h>' % sys.argv[0]
  else:
    main(file(sys.argv[1], 'r'), file(sys.argv[2], 'w+'))
