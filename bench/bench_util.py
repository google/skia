'''
Created on May 19, 2011

@author: bungeman
'''

import re
import math

# bench representation algorithm constant names
ALGORITHM_AVERAGE = 'avg'
ALGORITHM_MEDIAN = 'med'
ALGORITHM_MINIMUM = 'min'
ALGORITHM_25TH_PERCENTILE = '25th'

class BenchDataPoint:
    """A single data point produced by bench.

    (str, str, str, float, {str:str})"""
    def __init__(self, bench, config, time_type, time, settings):
        self.bench = bench
        self.config = config
        self.time_type = time_type
        self.time = time
        self.settings = settings

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

def _ParseAndStoreTimes(config_re, time_re, line, bench, dic,
                        representation=None):
    """Parses given bench time line with regex and adds data to the given dic.
    config_re: regular expression for parsing the config line.
    time_re: regular expression for parsing bench time.
    line: input string line to parse.
    bench: name of bench for the time values.
    dic: dictionary to store bench values. See bench_dic in parse() below.
    representation: should match one of the ALGORITHM_XXX types."""

    for config in re.finditer(config_re, line):
        current_config = config.group(1)
        if config_re.startswith('  tile_'):  # per-tile bench, add name prefix
            current_config = 'tile_' + current_config
        times = config.group(2)
        for new_time in re.finditer(time_re, times):
            current_time_type = new_time.group(1)
            iters = [float(i) for i in
                     new_time.group(2).strip().split(',')]
            dic.setdefault(bench, {}).setdefault(current_config, {}).setdefault(
                current_time_type, []).append(_ListAlgorithm(
                    iters, representation).compute())

def parse(settings, lines, representation=None):
    """Parses bench output into a useful data structure.

    ({str:str}, __iter__ -> str) -> [BenchDataPoint]
    representation is one of the ALGORITHM_XXX types."""

    benches = []
    current_bench = None
    bench_dic = {}  # [bench][config][time_type] -> [list of bench values]
    setting_re = '([^\s=]+)(?:=(\S+))?'
    settings_re = 'skia bench:((?:\s+' + setting_re + ')*)'
    bench_re = 'running bench (?:\[\d+ \d+\] )?\s*(\S+)'
    time_re = '(?:(\w*)msecs = )?\s*((?:\d+\.\d+)(?:,\d+\.\d+)*)'
    # non-per-tile benches have configs that don't end with ']'
    config_re = '(\S+[^\]]): ((?:' + time_re + '\s+)+)'
    # per-tile bench lines are in the following format. Note that there are
    # non-averaged bench numbers in separate lines, which we ignore now due to
    # their inaccuracy.
    tile_re = ('  tile_(\S+): tile \[\d+,\d+\] out of \[\d+,\d+\] <averaged>:'
               ' ((?:' + time_re + '\s+)+)')

    for line in lines:

        # see if this line is a settings line
        settingsMatch = re.search(settings_re, line)
        if (settingsMatch):
            settings = dict(settings)
            for settingMatch in re.finditer(setting_re, settingsMatch.group(1)):
                if (settingMatch.group(2)):
                    settings[settingMatch.group(1)] = settingMatch.group(2)
                else:
                    settings[settingMatch.group(1)] = True

        # see if this line starts a new bench
        new_bench = re.search(bench_re, line)
        if new_bench:
            current_bench = new_bench.group(1)

        # add configs on this line to the bench_dic
        if current_bench:
            for regex in [config_re, tile_re]:
                 _ParseAndStoreTimes(regex, time_re, line, current_bench,
                                     bench_dic, representation)

    # append benches to list, use the total time as final bench value.
    for bench in bench_dic:
        for config in bench_dic[bench]:
            for time_type in bench_dic[bench][config]:
                benches.append(BenchDataPoint(
                    bench,
                    config,
                    time_type,
                    sum(bench_dic[bench][config][time_type]),
                    settings))

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
