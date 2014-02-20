'''
Created on May 16, 2011

@author: bungeman
'''
import bench_util
import getopt
import httplib
import itertools
import json
import os
import re
import sys
import urllib
import urllib2
import xml.sax.saxutils

# Maximum expected number of characters we expect in an svn revision.
MAX_SVN_REV_LENGTH = 5

# Indices for getting elements from bench expectation files.
# See bench_expectations_<builder>.txt for details.
EXPECTED_IDX = -3
LB_IDX = -2
UB_IDX = -1

# Indices of the tuple of dictionaries containing slower and faster alerts.
SLOWER = 0
FASTER = 1

def usage():
    """Prints simple usage information."""

    print '-a <representation_alg> bench representation algorithm to use. '
    print '   Defaults to "25th". See bench_util.py for details.'
    print '-b <builder> name of the builder whose bench data we are checking.'
    print '-d <dir> a directory containing bench_<revision>_<scalar> files.'
    print '-e <file> file containing expected bench builder values/ranges.'
    print '   Will raise exception if actual bench values are out of range.'
    print '   See bench_expectations_<builder>.txt for data format / examples.'
    print '-r <revision> the git commit hash or svn revision for checking '
    print '   bench values.'


class Label:
    """The information in a label.

    (str, str, str, str, {str:str})"""
    def __init__(self, bench, config, time_type, settings):
        self.bench = bench
        self.config = config
        self.time_type = time_type
        self.settings = settings

    def __repr__(self):
        return "Label(%s, %s, %s, %s)" % (
                   str(self.bench),
                   str(self.config),
                   str(self.time_type),
                   str(self.settings),
               )

    def __str__(self):
        return "%s_%s_%s_%s" % (
                   str(self.bench),
                   str(self.config),
                   str(self.time_type),
                   str(self.settings),
               )

    def __eq__(self, other):
        return (self.bench == other.bench and
                self.config == other.config and
                self.time_type == other.time_type and
                self.settings == other.settings)

    def __hash__(self):
        return (hash(self.bench) ^
                hash(self.config) ^
                hash(self.time_type) ^
                hash(frozenset(self.settings.iteritems())))

def parse_dir(directory, default_settings, revision, rep):
    """Parses bench data from bench logs files.
       revision can be either svn revision or git commit hash.
    """
    revision_data_points = []  # list of BenchDataPoint
    file_list = os.listdir(directory)
    file_list.sort()
    for bench_file in file_list:
        scalar_type = None
        # Scalar type, if any, is in the bench filename after revision
        if (len(revision) > MAX_SVN_REV_LENGTH and
            bench_file.startswith('bench_' + revision + '_')):
            # The revision is GIT commit hash.
            scalar_type = bench_file[len(revision) + len('bench_') + 1:]
        elif (bench_file.startswith('bench_r' + revision + '_') and
              revision.isdigit()):
            # The revision is SVN number
            scalar_type = bench_file[len(revision) + len('bench_r') + 1:]
        else:
            continue

        file_handle = open(directory + '/' + bench_file, 'r')

        default_settings['scalar'] = scalar_type
        revision_data_points.extend(
                        bench_util.parse(default_settings, file_handle, rep))
        file_handle.close()
    return revision_data_points

def create_bench_dict(revision_data_points):
    """Convert current revision data into a dictionary of line data.

    Args:
      revision_data_points: a list of bench data points

    Returns:
      a dictionary of this form:
          keys = Label objects
          values = the corresponding bench value
    """
    bench_dict = {}
    for point in revision_data_points:
        point_name = Label(point.bench,point.config,point.time_type,
                           point.settings)
        if point_name not in bench_dict:
            bench_dict[point_name] = point.time
        else:
            raise Exception('Duplicate expectation entry: ' + str(point_name))

    return bench_dict

def read_expectations(expectations, filename):
    """Reads expectations data from file and put in expectations dict."""
    for expectation in open(filename).readlines():
        elements = expectation.strip().split(',')
        if not elements[0] or elements[0].startswith('#'):
            continue
        if len(elements) != 5:
            raise Exception("Invalid expectation line format: %s" %
                            expectation)
        bench_entry = elements[0] + ',' + elements[1]
        if bench_entry in expectations:
            raise Exception("Dup entries for bench expectation %s" %
                            bench_entry)
        # [<Bench_BmpConfig_TimeType>,<Platform-Alg>] -> (LB, UB, EXPECTED)
        expectations[bench_entry] = (float(elements[LB_IDX]),
                                     float(elements[UB_IDX]),
                                     float(elements[EXPECTED_IDX]))

def check_expectations(lines, expectations, key_suffix):
    """Check if any bench results are outside of expected range.

    For each input line in lines, checks the expectations dictionary to see if
    the bench is out of the given range.

    Args:
      lines: dictionary mapping Label objects to the bench values.
      expectations: dictionary returned by read_expectations().
      key_suffix: string of <Platform>-<Alg> containing the bot platform and the
        bench representation algorithm.

    Returns:
      No return value.

    Raises:
      Exception containing bench data that are out of range, if any.
    """
    # The platform for this bot, to pass to the dashboard plot.
    platform = key_suffix[ : key_suffix.rfind('-')]
    # Tuple of dictionaries recording exceptions that are slower and faster,
    # respectively. Each dictionary maps off_ratio (ratio of actual to expected)
    # to a list of corresponding exception messages.
    exceptions = ({}, {})
    for line in lines:
        line_str = str(line)
        line_str = line_str[ : line_str.find('_{')]
        bench_platform_key = line_str + ',' + key_suffix
        if bench_platform_key not in expectations:
            continue
        this_bench_value = lines[line]
        this_min, this_max, this_expected = expectations[bench_platform_key]
        if this_bench_value < this_min or this_bench_value > this_max:
            off_ratio = this_bench_value / this_expected
            exception = 'Bench %s out of range [%s, %s] (%s vs %s, %s%%).' % (
                bench_platform_key, this_min, this_max, this_bench_value,
                this_expected, (off_ratio - 1) * 100)
            if off_ratio > 1:  # Bench is slower.
                exceptions[SLOWER].setdefault(off_ratio, []).append(exception)
            else:
                exceptions[FASTER].setdefault(off_ratio, []).append(exception)
    outputs = []
    for i in [SLOWER, FASTER]:
      if exceptions[i]:
          ratios = exceptions[i].keys()
          ratios.sort(reverse=True)
          li = []
          for ratio in ratios:
              li.extend(exceptions[i][ratio])
          header = '%s benches got slower (sorted by %% difference):' % len(li)
          if i == FASTER:
              header = header.replace('slower', 'faster')
          outputs.extend(['', header] + li)

    if outputs:
        raise Exception('\n'.join(outputs))

def main():
    """Parses command line and checks bench expectations."""
    try:
        opts, _ = getopt.getopt(sys.argv[1:],
                                "a:b:d:e:r:",
                                "default-setting=")
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)

    directory = None
    bench_expectations = {}
    rep = '25th'  # bench representation algorithm, default to 25th
    rev = None  # git commit hash or svn revision number
    bot = None

    try:
        for option, value in opts:
            if option == "-a":
                rep = value
            elif option == "-b":
                bot = value
            elif option == "-d":
                directory = value
            elif option == "-e":
                read_expectations(bench_expectations, value)
            elif option == "-r":
                rev = value
            else:
                usage()
                assert False, "unhandled option"
    except ValueError:
        usage()
        sys.exit(2)

    if directory is None or bot is None or rev is None:
        usage()
        sys.exit(2)

    platform_and_alg = bot + '-' + rep

    data_points = parse_dir(directory,
                            {},  # Sets default settings to empty.
                            rev,
                            rep)

    bench_dict = create_bench_dict(data_points)

    if bench_expectations:
        check_expectations(bench_dict, bench_expectations, platform_and_alg)


if __name__ == "__main__":
    main()
