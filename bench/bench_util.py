'''
Created on May 19, 2011

@author: bungeman
'''

import re
import math

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
    representation is one of 'avg', 'min', 'med', '25th' (average, minimum,
    median, 25th percentile)"""
    def __init__(self, data, representation=None):
        if not representation:
            representation = 'avg'  # default algorithm is average
        self._data = data
        self._len = len(data)
        if representation == 'avg':
            self._rep = sum(self._data) / self._len
        else:
            self._data.sort()
            if representation == 'min':
                self._rep = self._data[0]
            else:
                # for percentiles, we use the value below which x% of values are
                # found, which allows for better detection of quantum behaviors.
                if representation == 'med':
                    x = int(round(0.5 * self._len + 0.5))
                elif representation == '25th':
                    x = int(round(0.25 * self._len + 0.5))
                else:
                    raise Exception("invalid representation algorithm %s!" %
                                    representation)
                self._rep = self._data[x - 1]

    def compute(self):
        return self._rep

def parse(settings, lines, representation='avg'):
    """Parses bench output into a useful data structure.
    
    ({str:str}, __iter__ -> str) -> [BenchDataPoint]
    representation should match one of those defined in class _ListAlgorithm."""
    
    benches = []
    current_bench = None
    setting_re = '([^\s=]+)(?:=(\S+))?'
    settings_re = 'skia bench:((?:\s+' + setting_re + ')*)'
    bench_re = 'running bench (?:\[\d+ \d+\] )?\s*(\S+)'
    time_re = '(?:(\w*)msecs = )?\s*((?:\d+\.\d+)(?:,\d+\.\d+)*)'
    config_re = '(\S+): ((?:' + time_re + '\s+)+)'
    
    for line in lines:
        
        #see if this line is a settings line
        settingsMatch = re.search(settings_re, line)
        if (settingsMatch):
            settings = dict(settings)
            for settingMatch in re.finditer(setting_re, settingsMatch.group(1)):
                if (settingMatch.group(2)):
                    settings[settingMatch.group(1)] = settingMatch.group(2)
                else:
                    settings[settingMatch.group(1)] = True
                
        #see if this line starts a new bench
        new_bench = re.search(bench_re, line)
        if new_bench:
            current_bench = new_bench.group(1)
        
        #add configs on this line to the current bench
        if current_bench:
            for new_config in re.finditer(config_re, line):
                current_config = new_config.group(1)
                times = new_config.group(2)
                for new_time in re.finditer(time_re, times):
                    current_time_type = new_time.group(1)
                    iters = [float(i) for i in
                             new_time.group(2).strip().split(',')]
                    benches.append(BenchDataPoint(
                            current_bench
                            , current_config
                            , current_time_type
                            , _ListAlgorithm(iters, representation).compute()
                            , settings))
    
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
