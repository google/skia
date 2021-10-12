#!/usr/bin/env python3
#
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# GLSL ES2 conformance test files can be found at
# https://github.com/KhronosGroup/VK-GL-CTS/tree/master/data/gles2/shaders
#
# Usage:
#     cat ${TEST_FILES}/*.test | ./import_conformance_tests.py
#
# This will generate two directories, "pass" and "fail", containing finished runtime shaders.
#
# Not all ES2 test files are meaningful in SkSL. These input files are not supported:
# - linkage.test: Runtime Effects only handle fragment processing
# - invalid_texture_functions.test: no GL texture functions in Runtime Effects
# - preprocessor.test: no preprocessor in SkSL

import os
import pyparsing as pp
import re
import sys

# Each case can contain expected input/output values, sometimes in [bracketed|lists] and
# sometimes not.
# GLSL code appears in ""double-double quotes"" and is indicated as vert/frag-specific or "both".
# Some tests denote that they are expected to pass/fail. (Presumably, "pass" is the default.)
# We ignore descriptions and version fields.
wordWithUnderscores = pp.Word(pp.alphanums + '_')

pipeList = pp.delimited_list(pp.SkipTo(pp.Literal("|") | pp.Literal("]")), delim="|")
bracketedPipeList = pp.Group(pp.Literal("[").suppress() +
                             pipeList +
                             pp.Literal("]").suppress())
unbracketedValue = pp.Group(pp.SkipTo(";"))

valueList = (pp.Word(pp.alphanums) +  # type
             pp.Word(pp.alphanums) +  # varname
             pp.Literal("=").suppress() +
             (bracketedPipeList | unbracketedValue) +
             pp.Literal(";").suppress())
value = pp.Group((pp.Keyword("input") | pp.Keyword("output") | pp.Keyword("uniform")) +
                 valueList)
values = (pp.Keyword("values") +
          pp.Literal("{").suppress() +
          pp.ZeroOrMore(value) +
          pp.Literal("}").suppress())

expectation = (pp.Keyword("expect").suppress() + (pp.Keyword("compile_fail") |
                                                  pp.Keyword("pass")))

code = ((pp.Keyword("both") + pp.QuotedString('""', multiline=True)) |
        (pp.Keyword("vertex") + pp.QuotedString('""', multiline=True) +
         pp.Keyword("fragment") + pp.QuotedString('""', multiline=True)))

reqGlsl100 = pp.Keyword("require").suppress() + pp.Keyword("full_glsl_es_100_support")

desc = pp.Keyword("desc") + pp.QuotedString('"')
version100es = pp.Keyword("version") + pp.Keyword("100") + pp.Keyword("es")
ignoredCaseItem = (desc | version100es).suppress()

caseItem = pp.Group(values | expectation | code | reqGlsl100) | ignoredCaseItem

caseBody = pp.ZeroOrMore(caseItem)

blockEnd = pp.Keyword("end").suppress();

caseHeader = pp.Keyword("case") + wordWithUnderscores
case = pp.Group(caseHeader + caseBody + blockEnd)

# Groups can be nested to any depth (or can be absent), and may contain any number of cases.
# The names in the group header are ignored.
groupHeader = (pp.Keyword("group") + wordWithUnderscores + pp.QuotedString('"')).suppress()

group = pp.Forward()
group <<= pp.OneOrMore(case | (groupHeader + group + blockEnd))

# The full grammar is just the group specification, plus the fact that # indicates a comment.
grammar = group
group.ignore('#' + pp.restOfLine)

testCases = grammar.parse_string(sys.stdin.read(), parse_all=True)

# Write output files in subdirectories next to this script.
testDirectory = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
passDirectory = testDirectory + "/pass"
failDirectory = testDirectory + "/fail"
os.makedirs(passDirectory, exist_ok=True)
os.makedirs(failDirectory, exist_ok=True)
written = {}

for c in testCases:
    # Parse the case header
    assert c[0] == 'case'
    c.pop(0)

    testName = c[0]
    assert isinstance(testName, str)
    c.pop(0)

    # Parse the case body
    skipTest = ''
    expectPass = True
    testCode = ''
    inputs = []
    outputs = []

    for b in c:
        caseItem = b[0]
        b.pop(0)

        if caseItem == 'compile_fail':
            expectPass = False

        elif caseItem == 'pass':
            expectPass = True

        elif caseItem == 'vertex' or caseItem == 'fragment':
            skipTest = 'Uses vertex'

        elif caseItem == 'both':
            testCode = b[0]
            assert isinstance(testCode, str)

        elif caseItem == 'values':
            for v in b:
                valueType = v[0]
                v.pop(0)

                if valueType == 'uniform':
                    skipTest = 'Uses uniform'
                elif valueType == 'input':
                    inputs.append(v.asList())
                elif valueType == 'output':
                    outputs.append(v.asList())

        elif caseItem == 'full_glsl_es_100_support':
            skipTest = 'Uses while loop'

        else:
            assert 0

    if skipTest == '':
        if "void main" not in testCode:
            skipTest = 'Missing main'

    if skipTest != '':
        print("skipped %s (%s)" % (testName, skipTest))
        continue

    # SkSL does not support casts which discard elements such as `float(myFloat4)`.
    # Switch these tests to a "fail" expectation instead of "pass."
    if (re.fullmatch('(vec|bvec|ivec)[234]_to_(float|int|bool)', testName) or
        re.fullmatch('(vec|bvec|ivec)[34]_to_(vec|bvec|ivec)2', testName) or
        re.fullmatch('(vec|bvec|ivec)[4]_to_(vec|bvec|ivec)3', testName) or
    # SkSL rejects code that fails to return a value; GLSL ES2 allows it.
        testName == 'missing_returns'):
        assert expectPass
        expectPass = False
        print("moved %s to fail" % testName)

    # Apply fixups to the test code.
    # SkSL doesn't support the `precision` keyword, so comment it out if it appears.
    testCode = testCode.replace("precision highp ",   "// precision highp ");
    testCode = testCode.replace("precision mediump ", "// precision mediump ");
    testCode = testCode.replace("precision lowp ",    "// precision lowp ");

    # SkSL doesn't support the `#version` declaration.
    testCode = testCode.replace("#version", "// #version");

    # Rename the `main` function to `execute_test`.
    testCode = testCode.replace("void main", "bool execute_test");

    # Replace ${POSITION_FRAG_COLOR} with a scratch variable.
    if "${POSITION_FRAG_COLOR}" in testCode:
        testCode = testCode.replace("${POSITION_FRAG_COLOR}", "PositionFragColor");
        if "${DECLARATIONS}" in testCode:
            testCode = testCode.replace("${DECLARATIONS}",
                                        "vec4 PositionFragColor;\n${DECLARATIONS}");
        else:
            testCode = "vec4 PositionFragColor;\n" + testCode

    # Create a runnable SkSL test by returning green or red based on the test result.
    testCode += "\n"
    testCode += "half4 main(float2 coords) {\n"
    testCode += "    return execute_test() ? half4(0,1,0,1) : half4(1,0,0,1);\n"
    testCode += "}\n"

    testDirectory = passDirectory
    if not expectPass:
        testDirectory = failDirectory

    # Find the total number of input/output fields.
    numVariables = 0
    for v in inputs + outputs:
        numVariables = max(numVariables, len(v[2]))

    if numVariables > 0:
        assert "${DECLARATIONS}" in testCode
        assert "${OUTPUT}" in testCode
        for varIndex in range(0, numVariables):
            testSpecialization = testCode

            # Assemble input variable declarations for ${DECLARATIONS}.
            declarations = ""
            for v in inputs:
                if len(v[2]) > varIndex:
                    declarations += "%s %s = %s;\n" % (v[0], v[1], v[2][varIndex]);

            # Assemble output variable declarations for ${DECLARATIONS}.
            for v in outputs:
                declarations += "%s %s;\n" % (v[0], v[1]);

            # Verify output values inside ${OUTPUT}.
            outputChecks = "return true"
            for v in outputs:
                if len(v[2]) > varIndex:
                    outputChecks += " && (%s == %s)" % (v[1], v[2][varIndex])

            outputChecks += ";\n"

            # Apply fixups to the test code.
            testSpecialization = testSpecialization.replace("${DECLARATIONS}", declarations)
            testSpecialization = testSpecialization.replace("${SETUP}",        '')
            testSpecialization = testSpecialization.replace("${OUTPUT}",       outputChecks)

            # Generate an SkSL test file.
            path = "%s/%s_%d.rts" % (testDirectory, testName, varIndex)
            assert path not in written
            written[path] = True
            f = open(path, "w")
            f.write(testSpecialization)

    else: # not (numVariables > 0)
        testCode = testCode.replace("${DECLARATIONS}", '')
        testCode = testCode.replace("${SETUP}",        '')
        testCode = testCode.replace("${OUTPUT}",       'return true;')

        # Generate an SkSL test file.
        path = "%s/%s.rts" % (testDirectory, testName)
        assert path not in written
        written[path] = True
        f = open(path, "w")
        f.write(testCode)
