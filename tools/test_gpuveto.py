#!/usr/bin/env python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script to test out suitableForGpuRasterization (via gpuveto)"""

import argparse
import glob
import os
import re
import subprocess
import sys

# Set the PYTHONPATH to include the tools directory.
sys.path.append(
    os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))
import find_run_binary

def list_files(dir_or_file):
    """Returns a list of all the files from the provided argument

    @param dir_or_file: either a directory or skp file

    @returns a list containing the files in the directory or a single file
    """
    files = []
    for globbedpath in glob.iglob(dir_or_file): # useful on win32
        if os.path.isdir(globbedpath):
            for filename in os.listdir(globbedpath):
                newpath = os.path.join(globbedpath, filename)
                if os.path.isfile(newpath):
                    files.append(newpath)
        elif os.path.isfile(globbedpath):
            files.append(globbedpath)
    return files


def execute_program(args):
    """Executes a process and waits for it to complete.

    @param args: is passed into subprocess.Popen().

    @returns a tuple of the process output (returncode, output)
    """
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, 
                            stderr=subprocess.STDOUT)
    output, _ = proc.communicate()
    errcode = proc.returncode
    return (errcode, output)


class GpuVeto(object):

    def __init__(self):
        self.bench_pictures = find_run_binary.find_path_to_program(
            'bench_pictures')
        sys.stdout.write('Running: %s\n' % (self.bench_pictures))
        self.gpuveto = find_run_binary.find_path_to_program('gpuveto')
        assert os.path.isfile(self.bench_pictures)
        assert os.path.isfile(self.gpuveto)
        self.indeterminate = 0
        self.truePositives = 0
        self.falsePositives = 0
        self.trueNegatives = 0
        self.falseNegatives = 0

    def process_skps(self, dir_or_file):
        for skp in enumerate(dir_or_file):
            self.process_skp(skp[1])

        sys.stdout.write('TP %d FP %d TN %d FN %d IND %d\n' % (self.truePositives,
                                                               self.falsePositives,
                                                               self.trueNegatives,
                                                               self.falseNegatives,
                                                               self.indeterminate))


    def process_skp(self, skp_file):
        assert os.path.isfile(skp_file)
        #print skp_file

        # run gpuveto on the skp
        args = [self.gpuveto, '-r', skp_file]
        returncode, output = execute_program(args)
        if (returncode != 0):
            return

        if ('unsuitable' in output):
            suitable = False
        else:
            assert 'suitable' in output
            suitable = True

        # run raster config
        args = [self.bench_pictures, '-r', skp_file, 
                                     '--repeat', '20',
                                     '--timers', 'w',
                                     '--config', '8888']
        returncode, output = execute_program(args)
        if (returncode != 0):
            return

        matches = re.findall('[\d]+\.[\d]+', output)
        if len(matches) != 1:
            return

        rasterTime = float(matches[0])

        # run gpu config
        args2 = [self.bench_pictures, '-r', skp_file, 
                                      '--repeat', '20',
                                      '--timers', 'w',
                                      '--config', 'gpu']
        returncode, output = execute_program(args2)
        if (returncode != 0):
            return

        matches = re.findall('[\d]+\.[\d]+', output)
        if len(matches) != 1:
            return

        gpuTime = float(matches[0])

        # happens if page is too big it will not render
        if 0 == gpuTime:
            return

        tolerance = 0.05
        tol_range = tolerance * gpuTime


        if rasterTime > gpuTime - tol_range and rasterTime < gpuTime + tol_range:
            result = "NONE"
            self.indeterminate += 1
        elif suitable:
            if gpuTime < rasterTime:
                self.truePositives += 1
                result = "TP"
            else:
                self.falsePositives += 1
                result = "FP"
        else:
            if gpuTime < rasterTime:
                self.falseNegatives += 1
                result = "FN"
            else:
                self.trueNegatives += 1
                result = "TN"
        

        sys.stdout.write('%s: gpuveto: %d raster %.2f gpu: %.2f  Result: %s\n' % (
            skp_file, suitable, rasterTime, gpuTime, result))

def main(main_argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--skp_path',
                        help='Path to the SKP(s). Can either be a directory ' \
                        'containing SKPs or a single SKP.',
                        required=True)

    args = parser.parse_args()
    GpuVeto().process_skps(list_files(args.skp_path))

if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
