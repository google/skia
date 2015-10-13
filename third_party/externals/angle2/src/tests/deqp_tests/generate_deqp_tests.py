import os
import re
import sys

def ReadFileAsLines(filename):
    """Reads a file, removing blank lines and lines that start with #"""
    file = open(filename, "r")
    raw_lines = file.readlines()
    file.close()
    lines = []
    for line in raw_lines:
        line = line.strip()
        if len(line) > 0 and not line.startswith("#"):
            lines.append(line)
    return lines

def GetCleanTestName(testName):
    replacements = { "dEQP-": "", ".*": "", ".":"_", }
    cleanName = testName
    for replaceKey in replacements:
        cleanName = cleanName.replace(replaceKey, replacements[replaceKey])
    return cleanName

def GenerateTests(outFile, testNames):
    ''' Remove duplicate tests '''
    testNames = list(set(testNames))

    outFile.write("#include \"deqp_tests.h\"\n\n")

    for test in testNames:
        outFile.write("TEST(deqp, " + GetCleanTestName(test) + ")\n")
        outFile.write("{\n")
        outFile.write("    RunDEQPTest(\"" + test + "\", GetCurrentConfig());\n")
        outFile.write("}\n\n")

def main(argv):
    tests = ReadFileAsLines(argv[0])
    output = open(argv[1], 'wb')
    GenerateTests(output, tests)
    output.close()
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
