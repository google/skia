'''
Created on May 16, 2011

@author: bungeman
'''
import sys
import getopt
import bench_util

def usage():
    """Prints simple usage information."""
    
    print '-o <file> the old bench output file.'
    print '-n <file> the new bench output file.'
    print '-h causes headers to be output.'
    print '-s <stat> the type of statistical analysis used'
    print '   Not specifying is the same as -s "avg".'
    print '  avg: average of all data points'
    print '  min: minimum of all data points'
    print '  med: median of all data points'
    print '  25th: twenty-fifth percentile for all data points'
    print '-f <fieldSpec> which fields to output and in what order.'
    print '   Not specifying is the same as -f "bctondp".'
    print '  b: bench'
    print '  c: config'
    print '  t: time type'
    print '  o: old time'
    print '  n: new time'
    print '  d: diff'
    print '  p: percent diff'
    
class BenchDiff:
    """A compare between data points produced by bench.
    
    (BenchDataPoint, BenchDataPoint)"""
    def __init__(self, old, new):
        self.old = old
        self.new = new
        self.diff = old.time - new.time
        diffp = 0
        if old.time != 0:
            diffp = self.diff / old.time
        self.diffp = diffp
    
    def __repr__(self):
        return "BenchDiff(%s, %s)" % (
                   str(self.new),
                   str(self.old),
               )
        
def main():
    """Parses command line and writes output."""
    
    try:
        opts, _ = getopt.getopt(sys.argv[1:], "f:o:n:s:h")
    except getopt.GetoptError, err:
        print str(err) 
        usage()
        sys.exit(2)
    
    column_formats = {
        'b' : '{bench: >28} ',
        'c' : '{config: <4} ',
        't' : '{time_type: <4} ',
        'o' : '{old_time: >10.2f} ',
        'n' : '{new_time: >10.2f} ',
        'd' : '{diff: >+10.2f} ',
        'p' : '{diffp: >+8.1%} ',
    }
    header_formats = {
        'b' : '{bench: >28} ',
        'c' : '{config: <4} ',
        't' : '{time_type: <4} ',
        'o' : '{old_time: >10} ',
        'n' : '{new_time: >10} ',
        'd' : '{diff: >10} ',
        'p' : '{diffp: >8} ',
    }
    
    old = None
    new = None
    column_format = ""
    header_format = ""
    columns = 'bctondp'
    header = False
    stat_type = "avg"
    
    for option, value in opts:
        if option == "-o":
            old = value
        elif option == "-n":
            new = value
        elif option == "-h":
            header = True
        elif option == "-f":
            columns = value
        elif option == "-s":
            stat_type = value
        else:
            usage()
            assert False, "unhandled option"
    
    if old is None or new is None:
        usage()
        sys.exit(2)
    
    for column_char in columns:
        if column_formats[column_char]:
            column_format += column_formats[column_char]
            header_format += header_formats[column_char]
        else:
            usage()
            sys.exit(2)
    
    if header:
        print header_format.format(
            bench='bench'
            , config='conf'
            , time_type='time'
            , old_time='old'
            , new_time='new'
            , diff='diff'
            , diffp='diffP'
        )
    
    old_benches = bench_util.parse({}, open(old, 'r'), stat_type)
    new_benches = bench_util.parse({}, open(new, 'r'), stat_type)
    
    bench_diffs = []
    for old_bench in old_benches:
        #filter new_benches for benches that match old_bench
        new_bench_match = [bench for bench in new_benches
            if old_bench.bench == bench.bench and
               old_bench.config == bench.config and
               old_bench.time_type == bench.time_type
        ]
        if (len(new_bench_match) < 1):
            continue
        bench_diffs.append(BenchDiff(old_bench, new_bench_match[0]))
    
    bench_diffs.sort(key=lambda d : [d.diffp,
                                     d.old.bench,
                                     d.old.config,
                                     d.old.time_type,
                                    ])
    for bench_diff in bench_diffs:
        print column_format.format(
            bench=bench_diff.old.bench.strip()
            , config=bench_diff.old.config.strip()
            , time_type=bench_diff.old.time_type
            , old_time=bench_diff.old.time
            , new_time=bench_diff.new.time
            , diff=bench_diff.diff
            , diffp=bench_diff.diffp
        )
    
if __name__ == "__main__":
    main()
