'''
Created on May 16, 2011

@author: bungeman
'''
import sys
import getopt
import re

def parse(lines):
    """Takes iterable lines of bench output, returns {bench:{config:time}}."""
    
    benches = {}
    current_bench = None
    
    for line in lines:
        #see if this line starts a new bench
        new_bench = re.search('running bench \[\d+ \d+\] (.{28})', line)
        if new_bench:
            current_bench = new_bench.group(1)
        
        #add configs on this line to the current bench
        if current_bench:
            for new_config in re.finditer('  (.{4}): msecs = (\d+\.\d+)', line):
                current_config = new_config.group(1)
                current_time = float(new_config.group(2))
                if current_bench in benches:
                    benches[current_bench][current_config] = current_time
                else:
                    benches[current_bench] = {current_config : current_time}
    
    return benches

def usage():
    """Prints simple usage information."""
    
    print '-o <file> the old bench output file.'
    print '-n <file> the new bench output file.'
    print '-h causes headers to be output.'
    print '-f <fieldSpec> which fields to output and in what order.'
    print '   Not specifying is the same as -f "bcondp".'
    print '  b: bench'
    print '  c: config'
    print '  o: old time'
    print '  n: new time'
    print '  d: diff'
    print '  p: percent diff'
    
    
def main():
    """Parses command line and writes output."""
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "f:o:n:h")
    except getopt.GetoptError, err:
        print str(err) 
        usage()
        sys.exit(2)
    
    column_formats = {
        'b' : '{bench: >28} ',
        'c' : '{config: <4} ',
        'o' : '{old_time: >10.2f} ',
        'n' : '{new_time: >10.2f} ',
        'd' : '{diff: >+10.2f} ',
        'p' : '{diffp: >+7.1%} ',
    }
    header_formats = {
        'b' : '{bench: >28} ',
        'c' : '{config: <4} ',
        'o' : '{old_time: >10} ',
        'n' : '{new_time: >10} ',
        'd' : '{diff: >10} ',
        'p' : '{diffp: >7} ',
    }
    
    old = None
    new = None
    column_format = ""
    header_format = ""
    columns = 'bcondp'
    header = False
    
    for option, value in opts:
        if option == "-o":
            old = value
        elif option == "-n":
            new = value
        elif option == "-h":
            header = True
        elif option == "-f":
            columns = value
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
            , old_time='old'
            , new_time='new'
            , diff='diff'
            , diffp='diffP'
        )
    
    old_benches = parse(open(old, 'r'))
    new_benches = parse(open(new, 'r'))
    
    for old_bench, old_configs in old_benches.items():
        if old_bench in new_benches:
            new_configs = new_benches[old_bench]
            for old_config, old_time in old_configs.items():
                if old_config in new_configs:
                    new_time = new_configs[old_config]
                    old_time = old_configs[old_config]
                    print column_format.format(
                        bench=old_bench.strip()
                        , config=old_config.strip()
                        , old_time=old_time
                        , new_time=new_time
                        , diff=(old_time - new_time)
                        , diffp=((old_time-new_time)/old_time)
                    )
    
    
if __name__ == "__main__":
    main()
