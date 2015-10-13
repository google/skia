#!/usr/bin/env python2
#
#  Copyright (c) 2013 Insollo Entertainment, LLC.  All rights reserved.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#  IN THE SOFTWARE.
#
from __future__ import print_function
"""

This module generates state diagrams for all state machines found in nanomsg
source code.

Dependencies:

* python2.7
* python-clang
* graphviz (dot)

Invocation:

    make diagrams

To make code easier we make some assumtions about the code handing state
machine. We may lift some in the future. Important assumptions are:

* State machine code is handled by single function
* That function name is written literally in `nn_fsm_init`/`nn_fsm_init_root`
* Init call an function definition is in same source file
* State machine handled by nested switch statements
* Case labels contain `define`d constants written literally
* The `state` attribute is changed by assignment in the same file
* No `state` attributes are referenced in the function except FSM state


"""

import os
import sys
import subprocess
import errno

try:
    from clang.cindex import Index
except ImportError:
    sys.excepthook(*sys.exc_info())
    print(file=sys.stderr)
    print("It seems you don't have clang for python.", file=sys.stderr)
    print("You may try one of the following:", file=sys.stderr)
    print("    pip install clang", file=sys.stderr)
    print("    easy_install clang", file=sys.stderr)
    sys.exit(1)

HTML_HEADER = """
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>nanomsg</title>
  <style>
  body {font-family:sans-serif;}
  #toplist {
    padding-left: 0px;
  }
  #toplist li {
    display: inline;
    list-style-type: none;
    padding-right: 15px;
  }
  a {color:#000000;}
  </style>
</head>
<body>
<div style="width:50em">

<img src="/logo.png">

<b>
<ul id='toplist'>
<li><a href="index.html">Home</a></li>
<li><a href="download.html">Download</a></li>
<li><a href="documentation.html">Documentation</a></li>
<li><a href="development.html">Development</a></li>
<li><a href="community.html">Community</a></li>
</ul>
</b>

<h2>State diagrams</h2>
"""

HTML_FOOTER = """
</div>
</body>
</html>
"""


def mkstate(stname):
    if '_STATE_' in stname:
        return stname.split('_STATE_')[1]
    if stname.startswith('NN_'):
        return stname[3:]
    return stname


def mksrc(src):
    if src is None:
        return '*'
    if '_SRC_' in src:
        return src.split('_SRC_')[1]
    if src.startswith('NN_'):
        return src[3:]
    return src


def mkaction(action):
    if action is None:
        return '*'
    if '_ACTION_' in action:
        return action.split('_ACTION_')[1]
    if action.startswith('NN_'):
        return action[3:]
    return action


class Visitor(object):

    def run(self, cursor):
        self.visit(cursor)

    def visit(self, cursor):
        name = cursor.kind.name
        meth = getattr(self, 'enter_' + name, None)
        if meth is not None:
            res = meth(cursor)
            if res is not None:
                return res.run(cursor)  # overrides visitor for subtree
        for i in cursor.get_children():
            self.visit(i)
        meth = getattr(self, 'exit_' + name, None)
        if meth is not None:
            meth(cursor)


class SkipVisitor(Visitor):
    """Returned from enter_xxx to skip checking subtree"""

    def run(self, cursor):
        pass


SKIP = Visitor()


class FindFSM(Visitor):

    def __init__(self):
        self.fsms = []

    def enter_CALL_EXPR(self, cursor):
        if cursor.displayname not in ('nn_fsm_init_root', 'nn_fsm_init'):
            return SKIP
        fname = list(cursor.get_children())[2]
        if fname.displayname:
            self.fsms.append(fname)
        # else: NULL is used in core/pipe.c

    def add_fsm(self, node):
        self.fsms.append(node)


class StateFinder(Visitor):

    def __init__(self):
        self.states = []

    def state_found(self, name, cursor):
        self.states.append((name, cursor))

    def visit(self, cursor):
        super(StateFinder, self).visit(cursor)

    def enter_CALL_EXPR(self, cursor):
        fun = list(cursor.get_children())[0].get_definition()
        if fun:
            sf = StateFinder()
            sf.run(fun)
            for name, cursor in sf.states:
                self.state_found(name, cursor)

    def enter_BINARY_OPERATOR(self, cursor):
        children = list(cursor.get_children())
        if children[0].displayname == 'state':  # tiny heuristic
            # Operator is the text between two children
            # Any better way to find out an operator?
            sr = cursor.extent
            with open(sr.start.file.name, 'rt') as f:
                oplen = (children[1].extent.start.offset -
                    children[0].extent.end.offset)
                vallen = (children[1].extent.end.offset -
                    children[1].extent.start.offset)
                f.seek(children[0].extent.end.offset)
                op = f.read(oplen).strip()
                val = f.read(vallen).strip()
            if op == '=':
                self.state_found(val, cursor)


class FSMScanner(StateFinder):

    def __init__(self):
        self.edges = set()
        self.running = {
            'state': None,
            'src': None,
            'type': None,
            }
        self.switch_stack = []

    def enter_SWITCH_STMT(self, cursor):
        ch = list(cursor.get_children())
        dn = ch[0].displayname
        if dn in self.running:
            assert self.running[dn] is None, (dn, self.running[dn])
        self.switch_stack.append(dn)

    def exit_SWITCH_STMT(self, cursor):
        ch = list(cursor.get_children())
        dn = ch[0].displayname
        top = self.switch_stack.pop()
        self.running[top] = None
        assert top == dn, (top, dn)  # Checking consistency of switch visits

    def enter_CASE_STMT(self, cursor):
        typ = self.switch_stack[-1]
        if typ in self.running:
            sr = list(cursor.get_children())[0].extent
            with open(sr.start.file.name, 'rt') as f:
                f.seek(sr.start.offset)
                const = f.read(sr.end.offset - sr.start.offset)
            self.running[typ] = const

    def enter_DEFAULT_STMT(self, cursor):
        typ = self.switch_stack[-1]
        if typ in self.running:
            self.running[typ] = '*'

    def state_found(self, state, cursor):
        r = self.running
        edge = (r['state'], r['src'], r['type'], state)
        if r['src'] is None or r['type'] is None:
            print('    Undefined state or action at {0.start.file}:{0.start.line}'
                .format(cursor.extent),
                file=sys.stderr)
        self.edges.add(edge)



index = None


def parse_file(fn):
    tu = index.parse(fn, sys.argv[1:])
    for i in tu.diagnostics:
        print(i, file=sys.stderr)
    finder = FindFSM()
    finder.run(tu.cursor)
    if finder.fsms:
        for func in finder.fsms:
            scan = FSMScanner()
            scan.run(func.get_definition())
            targetfn = os.path.join('doc/diagrams', func.displayname + '.png')
            print("Writing", targetfn, 'from', fn, file=sys.stderr)
            print("<h3>", func.displayname, "</h3>")
            print("<p>Source file:", fn, "</p>")
            print('<p><img src="diagrams/{}.png" border=0></p>'
                .format(func.displayname))
            lines = []
            for fromstate, src, action, tostate in scan.edges:
                if fromstate is None: continue  # Not implemented well
                if src == 'NN_FSM_ACTION':
                    lines.append('{} -> {} [label="[{}]"]'.format(
                        mkstate(fromstate),
                        mkstate(tostate),
                        mkaction(action)))
                else:
                    lines.append('{} -> {} [label="{}:{}"]'.format(
                        mkstate(fromstate),
                        mkstate(tostate),
                        mksrc(src),
                        mkaction(action)))
            data = 'digraph G {' + '\n'.join(lines) + '}'
            try:
                subprocess.Popen(['dot', '-Tpng', '-o', targetfn],
                    stdin=subprocess.PIPE).communicate(data)
            except OSError as e:
                sys.excepthook(*sys.exc_info())
                if e.errno == errno.ENOENT:
                    print(file=sys.stderr)
                    print("It seems you dont have `dot`", file=sys.stderr)
                    print("You may wish to try:", file=sys.stderr)
                    print("    apt-get install graphviz", file=sys.stderr)
                sys.exit(1)


def main():
    global index

    index = Index.create()

    with open('doc/diagrams.html', 'wt') as f:
        sys.stdout = f
        print(HTML_HEADER)
        for dirpath, dirs, files in os.walk('src'):
            for f in files:
                if not f.endswith('.c'):
                    continue
                parse_file(os.path.join(dirpath, f))
        print(HTML_FOOTER)


if __name__ == '__main__':
    main()

