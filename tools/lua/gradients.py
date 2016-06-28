#!/usr/bin/env python
#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sqlite3

def create_database(inpath, outpath):
    with sqlite3.connect(outpath) as conn:
        c = conn.cursor();
        c.execute('''CREATE TABLE IF NOT EXISTS gradients (
                        ColorCount   INTEGER,
                        GradientType TEXT,
                        TileMode     TEXT,
                        EvenlySpaced INTEGER,
                        HardStops    INTEGER,
                        Positions    TEXT
                     )''');
        c.execute("DELETE FROM gradients");

        with open(inpath, "r") as results:
            gradients = []
            for line in [line.strip() for line in results]:
                gradients.append(line.split());

            c.executemany("INSERT INTO gradients VALUES (?, ?, ?, ?, ?, ?)",
                          gradients);

            conn.commit();


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = "Transform Lua script output to a SQL DB");
    parser.add_argument("inpath",  help="Path to Lua script output file");
    parser.add_argument("outpath", help="Path to SQL DB");
    args = parser.parse_args();

    create_database(args.inpath, args.outpath);
