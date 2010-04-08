def lines_get(f):
  '''Parse a file like object, removing comments and returning a list of
     lines.'''
  def cut_comment(line):
    first_hash = line.find('#')
    if first_hash == -1:
      return line
    return line[:first_hash]

  return [x for x in [cut_comment(x[:-1]) for x in f.readlines()] if len(x)]

def line_split(line):
  '''Split a line based on a semicolon separator.'''
  def normalise(word):
    return word.lstrip().rstrip()
  return [normalise(x) for x in line.split(';')]

def codepoints_parse(token):
  '''Parse a Unicode style code-point range. Return either a single value or a
     tuple of (start, end) for a range of code-points.'''
  def fromHex(token):
    return int(token, 16)
  parts = token.split('..')
  if len(parts) == 2:
    return (fromHex(parts[0]), fromHex(parts[1]))
  elif len(parts) == 1:
    return fromHex(parts[0])
  else:
    raise ValueError(token)

def unicode_file_parse(input, map, default_value = None):
  '''Parse a file like object, @input where the first column is a code-point
     range and the second column is mapped via the given dict, @map.'''
  ranges = []
  tokens = [line_split(x) for x in lines_get(input)]
  for line in tokens:
    if len(line) == 2:
      codepoints = codepoints_parse(line[0])
      value = map[line[1]]
      if value == default_value:
        continue

      if type(codepoints) == int:
        codepoints = (codepoints, codepoints)

      ranges.append((codepoints[0], codepoints[1], value))
    else:
      raise ValueError(line)

  return ranges

def sort_and_merge(ranges):
  '''Given a list of (start, end, value), merge elements where the ranges are
     continuous and the values are the same.'''
  output = []
  ranges.sort()
  current = None
  for v in ranges:
    if current is None:
      current = v
      continue
    if current[1] + 1 == v[0] and current[2] == v[2]:
      current = (current[0], v[1], v[2])
    else:
      output.append(current)
      current = v
  if current is not None:
    output.append(current)

  return output
