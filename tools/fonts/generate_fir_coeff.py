#!/usr/bin/python

'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import math
import pprint

def withinStdDev(n):
  """Returns the percent of samples within n std deviations of the normal."""
  return math.erf(n / math.sqrt(2))

def withinStdDevRange(a, b):
  """Returns the percent of samples within the std deviation range a, b"""
  if b < a:
    return 0;

  if a < 0:
    if b < 0:
      return (withinStdDev(-a) - withinStdDev(-b)) / 2;
    else:
      return (withinStdDev(-a) + withinStdDev(b)) / 2;
  else:
    return (withinStdDev(b) - withinStdDev(a)) / 2;


# We have some smudged samples which represent the average coverage of a range.
# We have a 'center' which may not line up with those samples.
# From center make a normal where 5 sample widths out is at 3 std deviations.
# The first and last samples may not be fully covered.

# This is the sub-sample shift for each set of FIR coefficients
#   (the centers of the lcds in the samples)
# Each subpxl takes up 1/3 of a pixel,
#   so they are centered at x=(i/n+1/2n), or 1/6, 3/6, 5/6 of a pixel.
# Each sample takes up 1/4 of a pixel,
#   so the results fall at (x*4)%1, or 2/3, 0, 1/3 of a sample.
samples_per_pixel = 4
subpxls_per_pixel = 3
#sample_offsets is (frac, int) in sample units.
sample_offsets = [
  math.modf(
    (float(subpxl_index)/subpxls_per_pixel + 1.0/(2.0*subpxls_per_pixel))
    * samples_per_pixel
  ) for subpxl_index in range(subpxls_per_pixel)
]

#How many samples to consider to the left and right of the subpxl center.
sample_units_width = 5

#The std deviation at sample_units_width.
std_dev_max = 3

#The target sum is in some fixed point representation.
#Values larger the 1 in fixed point simulate ink spread.
target_sum = 0x110

for sample_offset, sample_align in sample_offsets:
  coeffs = []
  coeffs_rounded = []

  #We start at sample_offset - sample_units_width
  current_sample_left = sample_offset - sample_units_width
  current_std_dev_left = -std_dev_max

  done = False
  while not done:
    current_sample_right = math.floor(current_sample_left + 1)
    if current_sample_right > sample_offset + sample_units_width:
      done = True
      current_sample_right = sample_offset + sample_units_width
    current_std_dev_right = current_std_dev_left + (
      (current_sample_right - current_sample_left) / sample_units_width
    ) * std_dev_max

    coverage = withinStdDevRange(current_std_dev_left, current_std_dev_right)
    coeffs.append(coverage * target_sum)
    coeffs_rounded.append(int(round(coverage * target_sum)))

    current_sample_left = current_sample_right
    current_std_dev_left = current_std_dev_right

  # Have the numbers, but rounding needs to add up to target_sum.
  delta = 0
  coeffs_rounded_sum = sum(coeffs_rounded)
  if coeffs_rounded_sum > target_sum:
    # The coeffs add up to too much.
    # Subtract 1 from the ones which were rounded up the most.
    delta = -1

  if coeffs_rounded_sum < target_sum:
    # The coeffs add up to too little.
    # Add 1 to the ones which were rounded down the most.
    delta = 1

  if delta:
    print "Initial sum is 0x%0.2X, adjusting." % (coeffs_rounded_sum,)
    coeff_diff = [(coeff_rounded - coeff) * delta
                  for coeff, coeff_rounded in zip(coeffs, coeffs_rounded)]

    class IndexTracker:
      def __init__(self, index, item):
        self.index = index
        self.item = item
      def __lt__(self, other):
        return self.item < other.item
      def __repr__(self):
        return "arr[%d] == %s" % (self.index, repr(self.item))

    coeff_pkg = [IndexTracker(i, diff) for i, diff in enumerate(coeff_diff)]
    coeff_pkg.sort()

    # num_elements_to_force_round better be < (2 * sample_units_width + 1) or
    # * our math was wildy wrong
    # * an awful lot of the curve is out side our sample
    # either is pretty bad, and probably means the results will not be useful.
    num_elements_to_force_round = abs(coeffs_rounded_sum - target_sum)
    for i in xrange(num_elements_to_force_round):
      print "Adding %d to index %d to force round %f." % (
          delta, coeff_pkg[i].index, coeffs[coeff_pkg[i].index])
      coeffs_rounded[coeff_pkg[i].index] += delta

  print "Prepending %d 0x00 for allignment." % (sample_align,)
  coeffs_rounded_aligned = ([0] * int(sample_align)) + coeffs_rounded

  print ', '.join(["0x%0.2X" % coeff_rounded
                   for coeff_rounded in coeffs_rounded_aligned])
  print sum(coeffs), hex(sum(coeffs_rounded))
  print
