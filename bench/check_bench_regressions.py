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
        # [<Bench_BmpConfig_TimeType>,<Platform-Alg>] -> (LB, UB)
        expectations[bench_entry] = (float(elements[-2]),
                                     float(elements[-1]))

def check_expectations(lines, expectations, revision, key_suffix):
    """Check if there are benches in the given revising out of range.
    """
    # The platform for this bot, to pass to the dashboard plot.
    platform = key_suffix[ : key_suffix.rfind('-')]
    exceptions = []
    for line in lines:
        line_str = str(line)
        line_str = line_str[ : line_str.find('_{')]
        bench_platform_key = line_str + ',' + key_suffix
        if bench_platform_key not in expectations:
            continue
        this_bench_value = lines[line]
        this_min, this_max = expectations[bench_platform_key]
        if this_bench_value < this_min or this_bench_value > this_max:
            exception = 'Bench %s value %s out of range [%s, %s].' % (
                bench_platform_key, this_bench_value, this_min, this_max)
            exceptions.append(exception)
    if exceptions:
        raise Exception('Bench values out of range:\n' +
                        '\n'.join(exceptions))

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
        check_expectations(bench_dict, bench_expectations, rev,
                           platform_and_alg)


if __name__ == "__main__":
    main()
