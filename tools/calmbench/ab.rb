#!/usr/bin/ruby
# encoding: utf-8

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This is an A/B test utility script used by calmbench.py
#
# For each bench, we get a distribution of min_ms measurements from nanobench.
# From that, we try to recover the 1/3 and 2/3 quantiles of the distribution.
# If range (1/3 quantile, 2/3 quantile) is completely disjoint between A and B,
# we report that as a regression.
#
# The more measurements we have for a bench, the more accurate our quantiles are. However,
# taking more measurements is time consuming. Hence we'll prune out benches and only
# take more measurements for benches whose current quantile ranges are disjoint.

require 'thread'

HELP = <<EOS
\e[31m
Please call calmbench.py to drive this script if you're not doing so.
This script is not supposed to be used by itself. (At least, it's not easy to use by itself.)
\e[0m
EOS

if ARGV.length < 12
    puts HELP
    exit
end

SKIADIR = ARGV[0]
OUTDIR  = ARGV[1]
A       = ARGV[2]
B       = ARGV[3]
NANO_A  = ARGV[4]
NANO_B  = ARGV[5]
ARG_A   = ARGV[6]
ARG_B   = ARGV[7]
REPEAT  = ARGV[8].to_i
SKIP_B  = ARGV[9] == "true"
CONFIG  = ARGV[10]
THREADS = ARGV[11].to_i
NOINIT  = ARGV[12] == "true"

FACTOR  = 3 # lower/upper quantile factor
DIFF_T  = 0.99 # difference enough threshold
TERM    = 10 # terminate after this number of iterations without suspect changes
MAXTRY  = 30 # max number of nanobench tries to narrow down suspects

UNITS   = %w(ns µs ms s)

$names   = []
$files   = []
$timesA  = {}
$timesB  = {}

def append_dict_sorted_array(dict_array, key, value)
    dict_array[key] = [] if dict_array[key].nil?
    dict_array[key].push(value)
    dict_array[key].sort!
end

def add_time(name, bench, t, unit)
    normalized_t = t * 1000 ** UNITS.index(unit);
    if name.start_with? "#{A}"
        append_dict_sorted_array($timesA, bench, normalized_t)
    else
        append_dict_sorted_array($timesB, bench, normalized_t)
    end
end

def append_times_from_file(name, filename)
    lines = File.readlines(filename)
    for line in lines
        items = line.split
        if items.length > 10
            bench = "#{items[10]}" # "(#{items[9]}) #{items[10]}"
            matches = /([+-]?\d*.?\d+)(s|ms|µs|ns)/.match(items[3])
            next if (not matches or items[9] != CONFIG)
            time_num = matches[1]
            time_unit = matches[2]
            add_time(name, bench, time_num.to_f, time_unit)
        end
    end
end

# Simplest and stupidiest threaded executer
class ThreadRunner
    def initialize
        @threads = []
        @waitCnt = 0
    end

    def add(fn)
        wait() if @threads.length >= THREADS
        @threads.push(Thread.start { fn.call })
    end

    def wait()
        @waitCnt += 1
        currentWait = @waitCnt
        Thread.start{
            i = 0
            spinners = [".  ", ".. ", "..."]
            while @threads.length > 0 && currentWait == @waitCnt
                $timesLock.synchronize{
                    print "\r" +
                        "#{spinners[i % spinners.length]} (#{@threads.length} threads running)" +
                        "           \r" # spaces for erasing previous characters
                }
                sleep(0.5)
                i += 1
            end
        }
        for t in @threads
            t.join()
        end
        @threads = []
    end
end

$threadRunner = ThreadRunner.new()
$timesLock = Mutex.new

def run(name, nano, arg, i)
    $threadRunner.add(lambda {
        name_i = "#{name}_#{i}"
        file_i = "#{OUTDIR}/#{name}.out#{i}"

        should_run = !NOINIT && !(name == B && SKIP_B)
        should_run = true if i <= 0 # always run for suspects

        if should_run
            $timesLock.synchronize{ puts "Init run #{i} for #{name}..." if i > 0 }
            `touch #{file_i} && #{nano} #{arg} --config #{CONFIG} &> #{file_i}`
        end

        $timesLock.synchronize { append_times_from_file(name, file_i) }
    })
end

def init_run
    Dir.chdir(SKIADIR) {
        for i in 1..[REPEAT, THREADS / 2].max
            run(A, NANO_A, ARG_A, i)
            run(B, NANO_B, ARG_B, i)
        end
    }
    $threadRunner.wait()
end

def get_lower_upper(values)
    i = [0, (values.length - 1) / FACTOR].max
    return values[i], values[-i - 1]
end

def different_enough(lower1, upper2)
    return upper2 < DIFF_T * lower1
end

def get_suspects
    suspects = []
    for bench in $timesA.keys
        next if $timesB[bench].nil?
        lowerA, upperA = get_lower_upper($timesA[bench])
        lowerB, upperB = get_lower_upper($timesB[bench])
        if different_enough(lowerA, upperB) or different_enough(lowerB, upperA)
            suspects.push(bench)
        end
    end
    return suspects
end

def process_bench_pattern(s)
    if s.include? ".skp" # skp bench won't match to their exact names...
        return "^\"#{s[0..(s.index(".skp") + 3)]}\""
    else
        return "^\"#{s}\"$"
    end
end

def suspects_arg(suspects)
    patterns = suspects.map{ |s| process_bench_pattern(s) }
    return " --match " + patterns.join(" ")
end

def median(array)
    return array[array.length / 2]
end

def regression(bench)
    a = median($timesA[bench])
    b = median($timesB[bench])
    return b / a
end

def percentage(x)
    return (x - 1) * 100
end

def test
    init_run()
    last_unchanged_iter = 0
    last_suspect_number = -1
    tryCnt = 0
    iter = 0
    while tryCnt < MAXTRY
        iter += 1
        suspects = get_suspects()
        if suspects.length != last_suspect_number
            last_suspect_number = suspects.length
            last_unchanged_iter = iter
        end
        break if (suspects.length == 0 || iter - last_unchanged_iter >= TERM)

        puts "Number of suspects at iteration #{iter}: #{suspects.length}"
        for j in 1..([1, THREADS / 2].max)
            run(A, NANO_A, ARG_A + suspects_arg(suspects), -j)
            run(B, NANO_B, ARG_B + suspects_arg(suspects), -j)
            tryCnt += 1
        end
        $threadRunner.wait()
    end

    suspects = get_suspects()
    if suspects.length == 0
        puts "#{A} and #{B} does not seem to have significant performance differences."
    else
        suspects.sort! {|left, right| regression(left) <=> regression(right)}
        puts "#{A} (compared to #{B}) is likely"
        for suspect in suspects
            r = regression(suspect)
            if r < 1
                puts ("\e[31m  #{'%6.2f' % percentage(1/r)}\% slower in #{suspect}\e[0m")
            else
                puts ("\e[32m  #{'%6.2f' % percentage(r)}\% faster in #{suspect}\e[0m")
            end
        end
    end

    File.open("#{OUTDIR}/bench_#{A}_#{B}.csv", 'w') { |outfile|
        outfile.puts "bench, significant?, raw regresion, #{A} quantile (ns), " +
                     "#{B} quantile (ns), #{A} (ns), #{B} (ns)"
        for bench in suspects + $timesA.keys
            ta = $timesA[bench]
            tb = $timesB[bench]
            next if (not ta or not tb)
            outfile.puts(
                "#{bench}, #{suspects.include? bench}, #{regression(bench)}, " +
                get_lower_upper(ta).join(' ') + ", " +
                get_lower_upper(tb).join(' ') + ", " +
                "#{ta.join(' ')}, #{tb.join(' ')}"
            )
        end
        puts    "\e[36m" +
                    "Compared #{($timesA.keys & $timesB.keys).length} benches. " +
                    "#{suspects.length} of them seem to be significantly differrent." +
                "\e[0m"
        puts "\e[36mPlease see detailed bench results in #{outfile.path}\e[0m"
    }
end

if __FILE__ == $0
    begin
        test()
    rescue Interrupt
        raise
    rescue Exception => e
        puts "Something went wrong:"
        puts e
        puts e.backtrace
        puts HELP
    end
end
