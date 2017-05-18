# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import os
import re
import requests

from os import listdir
from os.path import isfile, join

default_ops = [
    "enable_gpu",
    "post",
    "info",
    "cmd",
    "img",
    "batchList"
]

def Check(request):
    assert(request.status_code == 200)
    return request

def WriteJson(request, path):
    # Writes out pretty printed json
    with open(path, 'wb+') as fd:
        json.dump(request.json(), fd, sort_keys=True, indent=2,
                  separators=(',', ': '))
    return request

def WritePng(request, path):
    with open(path, 'wb+') as fd:
        fd.write(request.content)


# A simple class to drive testing
class SkiaServeTester:
    def __init__(self, url, output_dir):
        self.url = url
        self.output_dir = output_dir

        # skp properties
        self.skp = ''
        self.skp_name = ''

    def set_skp(self, skp_dir, skp_name):
        self.skp = skp_dir + '/' + skp_name
        self.skp_name = skp_name

    def info(self):
        return Check(requests.get(self.url + '/info'))

    def post(self):
        with open(self.skp, 'rb') as payload:
            files = {'file': payload}
    
            # upload skp
            return Check(requests.post(self.url + '/new', files=files))

    def cmd(self):
        path = self.output_dir + '/' + self.skp_name + '.cmd.json'
        return WriteJson(Check(requests.get(self.url + '/cmd')), path)

    def img(self):
        opcount = self.opcount()
        url = self.url + '/img/' + str(opcount)
        path = self.output_dir + '/' + self.skp_name + '.png'
        return WritePng(Check(requests.get(url)), path)

    def enable_gpu(self):
        return Check(requests.post(self.url + '/enableGPU/1'))
    
    def disable_gpu(self):
        return Check(requests.post(self.url + '/enableGPU/0'))

    def opcount(self):
        r = self.cmd()
        return len(r.json()['commands']) - 1 # why the minus 1 here?

    def batchList(self):
        path = self.output_dir + '/' + self.skp_name + '.batches.json'
        return WriteJson(Check(requests.get(self.url + '/batches')), path)

def main():
    parser = argparse.ArgumentParser(description='Tester for SkiaServe')
    parser.add_argument('--skp_dir', default='skps', type=str)
    parser.add_argument('--url', default='http://localhost:8888', type=str)
    parser.add_argument('--output_dir', default='results', type=str)
    parser.add_argument('--match', default='.*', type=str)
    parser.add_argument('--ops', nargs='+', default=default_ops)

    args = parser.parse_args()
    skp_dir = args.skp_dir
    url = args.url
    output_dir = args.output_dir
    ops = args.ops

    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)

    skps = []
    for skp in listdir(skp_dir):
        if isfile(join(skp_dir, skp)) and re.match(args.match, skp):
            skps.append(skp)

    tester = SkiaServeTester(url, output_dir)

    for skp_name in skps:
        tester.set_skp(skp_dir, skp_name)
        for op in ops:
            getattr(tester, op)()

if __name__ == "__main__":
    main()
