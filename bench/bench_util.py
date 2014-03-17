'''
Created on May 19, 2011

@author: bungeman
'''

import os
import re
import math

# bench representation algorithm constant names
ALGORITHM_AVERAGE = 'avg'
ALGORITHM_MEDIAN = 'med'
ALGORITHM_MINIMUM = 'min'
ALGORITHM_25TH_PERCENTILE = '25th'

# Regular expressions used throughout.
PER_SETTING_RE = '([^\s=]+)(?:=(\S+))?'
SETTINGS_RE = 'skia bench:((?:\s+' + PER_SETTING_RE + ')*)'
BENCH_RE = 'running bench (?:\[\d+ \d+\] )?\s*(\S+)'
TIME_RE = '(?:(\w*)msecs = )?\s*((?:\d+\.\d+)(?:,\s*\d+\.\d+)*)'
# non-per-tile benches have configs that don't end with ']' or '>'
CONFIG_RE = '(\S+[^\]>]):\s+((?:' + TIME_RE + '\s+)+)'
# per-tile bench lines are in the following format. Note that there are
# non-averaged bench numbers in separate lines, which we ignore now due to
# their inaccuracy.
TILE_RE = ('  tile_(\S+): tile \[\d+,\d+\] out of \[\d+,\d+\] <averaged>:'
           ' ((?:' + TIME_RE + '\s+)+)')
# for extracting tile layout
TILE_LAYOUT_RE = ' out of \[(\d+),(\d+)\] <averaged>: '

PER_SETTING_RE_COMPILED = re.compile(PER_SETTING_RE)
SETTINGS_RE_COMPILED = re.compile(SETTINGS_RE)
BENCH_RE_COMPILED = re.compile(BENCH_RE)
TIME_RE_COMPILED = re.compile(TIME_RE)
CONFIG_RE_COMPILED = re.compile(CONFIG_RE)
TILE_RE_COMPILED = re.compile(TILE_RE)
TILE_LAYOUT_RE_COMPILED = re.compile(TILE_LAYOUT_RE)

class BenchDataPoint:
    """A single data point produced by bench.
    """
    def __init__(self, bench, config, time_type, time, settings,
                 tile_layout='', per_tile_values=[], per_iter_time=[]):
        # string name of the benchmark to measure
        self.bench = bench
        # string name of the configurations to run
        self.config = config
        # type of the timer in string: '' (walltime), 'c' (cpu) or 'g' (gpu)
        self.time_type = time_type
        # float number of the bench time value
        self.time = time
        # dictionary of the run settings
        self.settings = settings
        # how tiles cover the whole picture: '5x3' means 5 columns and 3 rows
        self.tile_layout = tile_layout
        # list of float for per_tile bench values, if applicable
        self.per_tile_values = per_tile_values
        # list of float for per-iteration bench time, if applicable
        self.per_iter_time = per_iter_time

    def __repr__(self):
        return "BenchDataPoint(%s, %s, %s, %s, %s)" % (
                   str(self.bench),
                   str(self.config),
                   str(self.time_type),
                   str(self.time),
                   str(self.settings),
               )

class _ExtremeType(object):
    """Instances of this class compare greater or less than other objects."""
    def __init__(self, cmpr, rep):
        object.__init__(self)
        self._cmpr = cmpr
        self._rep = rep

    def __cmp__(self, other):
        if isinstance(other, self.__class__) and other._cmpr == self._cmpr:
            return 0
        return self._cmpr

    def __repr__(self):
        return self._rep

Max = _ExtremeType(1, "Max")
Min = _ExtremeType(-1, "Min")

class _ListAlgorithm(object):
    """Algorithm for selecting the representation value from a given list.
    representation is one of the ALGORITHM_XXX representation types."""
    def __init__(self, data, representation=None):
        if not representation:
            representation = ALGORITHM_AVERAGE  # default algorithm
        self._data = data
        self._len = len(data)
        if representation == ALGORITHM_AVERAGE:
            self._rep = sum(self._data) / self._len
        else:
            self._data.sort()
            if representation == ALGORITHM_MINIMUM:
                self._rep = self._data[0]
            else:
                # for percentiles, we use the value below which x% of values are
                # found, which allows for better detection of quantum behaviors.
                if representation == ALGORITHM_MEDIAN:
                    x = int(round(0.5 * self._len + 0.5))
                elif representation == ALGORITHM_25TH_PERCENTILE:
                    x = int(round(0.25 * self._len + 0.5))
                else:
                    raise Exception("invalid representation algorithm %s!" %
                                    representation)
                self._rep = self._data[x - 1]

    def compute(self):
        return self._rep

def _ParseAndStoreTimes(config_re_compiled, is_per_tile, line, bench,
                        value_dic, layout_dic):
    """Parses given bench time line with regex and adds data to value_dic.

    config_re_compiled: precompiled regular expression for parsing the config
        line.
    is_per_tile: boolean indicating whether this is a per-tile bench.
        If so, we add tile layout into layout_dic as well.
    line: input string line to parse.
    bench: name of bench for the time values.
    value_dic: dictionary to store bench values. See bench_dic in parse() below.
    layout_dic: dictionary to store tile layouts. See parse() for descriptions.
    """

    for config in config_re_compiled.finditer(line):
        current_config = config.group(1)
        tile_layout = ''
        if is_per_tile:  # per-tile bench, add name prefix
            current_config = 'tile_' + current_config
            layouts = TILE_LAYOUT_RE_COMPILED.search(line)
            if layouts and len(layouts.groups()) == 2:
              tile_layout = '%sx%s' % layouts.groups()
        times = config.group(2)
        for new_time in TIME_RE_COMPILED.finditer(times):
            current_time_type = new_time.group(1)
            iters = [float(i) for i in
                     new_time.group(2).strip().split(',')]
            value_dic.setdefault(bench, {}).setdefault(
                current_config, {}).setdefault(current_time_type, []).append(
                    iters)
            layout_dic.setdefault(bench, {}).setdefault(
                current_config, {}).setdefault(current_time_type, tile_layout)

def parse_skp_bench_data(directory, revision, rep, default_settings=None):
    """Parses all the skp bench data in the given directory.

    Args:
      directory: string of path to input data directory.
      revision: git hash revision that matches the data to process.
      rep: bench representation algorithm, see bench_util.py.
      default_settings: dictionary of other run settings. See writer.option() in
          bench/benchmain.cpp.

    Returns:
      A list of BenchDataPoint objects.
    """
    revision_data_points = []
    file_list = os.listdir(directory)
    file_list.sort()
    for bench_file in file_list:
        scalar_type = None
        # Scalar type, if any, is in the bench filename after 'scalar_'.
        if (bench_file.startswith('bench_' + revision + '_data_')):
            if bench_file.find('scalar_') > 0:
                components = bench_file.split('_')
                scalar_type = components[components.index('scalar') + 1]
        else:  # Skips non skp bench files.
            continue

        with open('/'.join([directory, bench_file]), 'r') as file_handle:
          settings = dict(default_settings or {})
          settings['scalar'] = scalar_type
          revision_data_points.extend(parse(settings, file_handle, rep))

    return revision_data_points

# TODO(bensong): switch to reading JSON output when available. This way we don't
# need the RE complexities.
def parse(settings, lines, representation=None):
    """Parses bench output into a useful data structure.

    ({str:str}, __iter__ -> str) -> [BenchDataPoint]
    representation is one of the ALGORITHM_XXX types."""

    benches = []
    current_bench = None
    # [bench][config][time_type] -> [[per-iter values]] where per-tile config
    # has per-iter value list for each tile [[<tile1_iter1>,<tile1_iter2>,...],
    # [<tile2_iter1>,<tile2_iter2>,...],...], while non-per-tile config only
    # contains one list of iterations [[iter1, iter2, ...]].
    bench_dic = {}
    # [bench][config][time_type] -> tile_layout
    layout_dic = {}

    for line in lines:

        # see if this line is a settings line
        settingsMatch = SETTINGS_RE_COMPILED.search(line)
        if (settingsMatch):
            settings = dict(settings)
            for settingMatch in PER_SETTING_RE_COMPILED.finditer(settingsMatch.group(1)):
                if (settingMatch.group(2)):
                    settings[settingMatch.group(1)] = settingMatch.group(2)
                else:
                    settings[settingMatch.group(1)] = True

        # see if this line starts a new bench
        new_bench = BENCH_RE_COMPILED.search(line)
        if new_bench:
            current_bench = new_bench.group(1)

        # add configs on this line to the bench_dic
        if current_bench:
            if line.startswith('  tile_') :
                _ParseAndStoreTimes(TILE_RE_COMPILED, True, line, current_bench,
                                    bench_dic, layout_dic)
            else:
                _ParseAndStoreTimes(CONFIG_RE_COMPILED, False, line,
                                    current_bench, bench_dic, layout_dic)

    # append benches to list
    for bench in bench_dic:
        for config in bench_dic[bench]:
            for time_type in bench_dic[bench][config]:
                tile_layout = ''
                per_tile_values = []  # empty for non-per-tile configs
                per_iter_time = []  # empty for per-tile configs
                bench_summary = None  # a single final bench value
                if len(bench_dic[bench][config][time_type]) > 1:
                    # per-tile config; compute representation for each tile
                    per_tile_values = [
                        _ListAlgorithm(iters, representation).compute()
                            for iters in bench_dic[bench][config][time_type]]
                    # use sum of each tile representation for total bench value
                    bench_summary = sum(per_tile_values)
                    # extract tile layout
                    tile_layout = layout_dic[bench][config][time_type]
                else:
                    # get the list of per-iteration values
                    per_iter_time = bench_dic[bench][config][time_type][0]
                    bench_summary = _ListAlgorithm(
                        per_iter_time, representation).compute()
                benches.append(BenchDataPoint(
                    bench,
                    config,
                    time_type,
                    bench_summary,
                    settings,
                    tile_layout,
                    per_tile_values,
                    per_iter_time))

    return benches

class LinearRegression:
    """Linear regression data based on a set of data points.

    ([(Number,Number)])
    There must be at least two points for this to make sense."""
    def __init__(self, points):
        n = len(points)
        max_x = Min
        min_x = Max

        Sx = 0.0
        Sy = 0.0
        Sxx = 0.0
        Sxy = 0.0
        Syy = 0.0
        for point in points:
            x = point[0]
            y = point[1]
            max_x = max(max_x, x)
            min_x = min(min_x, x)

            Sx += x
            Sy += y
            Sxx += x*x
            Sxy += x*y
            Syy += y*y

        denom = n*Sxx - Sx*Sx
        if (denom != 0.0):
            B = (n*Sxy - Sx*Sy) / denom
        else:
            B = 0.0
        a = (1.0/n)*(Sy - B*Sx)

        se2 = 0
        sB2 = 0
        sa2 = 0
        if (n >= 3 and denom != 0.0):
            se2 = (1.0/(n*(n-2)) * (n*Syy - Sy*Sy - B*B*denom))
            sB2 = (n*se2) / denom
            sa2 = sB2 * (1.0/n) * Sxx


        self.slope = B
        self.intercept = a
        self.serror = math.sqrt(max(0, se2))
        self.serror_slope = math.sqrt(max(0, sB2))
        self.serror_intercept = math.sqrt(max(0, sa2))
        self.max_x = max_x
        self.min_x = min_x

    def __repr__(self):
        return "LinearRegression(%s, %s, %s, %s, %s)" % (
                   str(self.slope),
                   str(self.intercept),
                   str(self.serror),
                   str(self.serror_slope),
                   str(self.serror_intercept),
               )

    def find_min_slope(self):
        """Finds the minimal slope given one standard deviation."""
        slope = self.slope
        intercept = self.intercept
        error = self.serror
        regr_start = self.min_x
        regr_end = self.max_x
        regr_width = regr_end - regr_start

        if slope < 0:
            lower_left_y = slope*regr_start + intercept - error
            upper_right_y = slope*regr_end + intercept + error
            return min(0, (upper_right_y - lower_left_y) / regr_width)

        elif slope > 0:
            upper_left_y = slope*regr_start + intercept + error
            lower_right_y = slope*regr_end + intercept - error
            return max(0, (lower_right_y - upper_left_y) / regr_width)

        return 0

def CreateRevisionLink(revision_number):
    """Returns HTML displaying the given revision number and linking to
    that revision's change page at code.google.com, e.g.
    http://code.google.com/p/skia/source/detail?r=2056
    """
    return '<a href="http://code.google.com/p/skia/source/detail?r=%s">%s</a>'%(
        revision_number, revision_number)

def main():
    foo = [[0.0, 0.0], [0.0, 1.0], [0.0, 2.0], [0.0, 3.0]]
    LinearRegression(foo)

if __name__ == "__main__":
    main()
