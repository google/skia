/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSPath.h"

#include "bmhParser.h"
#include "fiddleParser.h"
#include "mdOut.h"
#include "includeWriter.h"
#include "selfCheck.h"

static DEFINE_string2(status, a, "", "File containing status of documentation. (Use in place of -b -i)");
static DEFINE_string2(bmh, b, "", "Path to a *.bmh file or a directory.");
static DEFINE_bool2(catalog, c, false, "Write example catalog.htm. (Requires -b -f -r)");
static DEFINE_string2(examples, e, "", "File of fiddlecli input, usually fiddle.json (For now, disables -r -f -s)");
static DEFINE_bool2(extract, E, false, "Extract examples into fiddle.json");
static DEFINE_string2(fiddle, f, "", "File of fiddlecli output, usually fiddleout.json.");
static DEFINE_bool2(hack, H, false, "Do a find/replace hack to update all *.bmh files. (Requires -b)");
// h is reserved for help
static DEFINE_string2(include, i, "", "Path to a *.h file or a directory.");
static DEFINE_bool2(selfcheck, k, false, "Check bmh against itself. (Requires -b)");
static DEFINE_bool2(stdout, o, false, "Write file out to standard out.");
static DEFINE_bool2(populate, p, false, "Populate include from bmh. (Requires -b -i)");
// q is reserved for quiet
static DEFINE_string2(ref, r, "", "Resolve refs and write *.md files to path. (Requires -b -f)");
static DEFINE_string2(spellcheck, s, "", "Spell-check [once, all, mispelling]. (Requires -b)");
static DEFINE_bool2(tokens, t, false, "Write bmh from include. (Requires -i)");
static DEFINE_bool2(crosscheck, x, false, "Check bmh against includes. (Requires -b -i)");
// v is reserved for verbose
static DEFINE_bool2(validate, V, false, "Validate that all anchor references have definitions. (Requires -r)");
static DEFINE_bool2(skip, z, false, "Skip degenerate missed in legacy preprocessor.");

// -b docs -i include/core/SkRect.h -f fiddleout.json -r site/user/api
// -b docs/SkIRect_Reference.bmh -H
/* todos:

if #Subtopic contains #SeeAlso or #Example generate horizontal rule at end
constexpr populated with filter inside subtopic does not have definition body

#List needs '# content ##', formatting
rewrap text to fit in some number of columns
#Literal is inflexible, making the entire #Code block link-less (see $Literal in SkImageInfo)
     would rather keep links for body above #Literal, and/or make it a block and not a one-liner
add check to require #Const to contain #Code block if defining const or constexpr (enum consts have
     #Code blocks inside the #Enum def)
subclasses (e.g. Iter in SkPath) need to check for #Line and generate overview
     subclass methods should also disallow #In

It's awkward that phrase param is a child of the phrase def. Since phrase refs may also be children,
there is special case code to skip phrase def when looking for additional substitutions in the
phrase def. Could put it in the token list instead I guess, or make a definition subclass used
by phrase def with an additional slot...

rearrange const out for md so that const / value / short description comes first in a table,
followed by more elaborate descriptions, examples, seealso. In md.cpp, look to see if #Subtopic
has #Const children. If so, generate a summary table first.
Or, only allow #Line and moderate text description in #Const. Put more verbose text, example,
seealso, in subsequent #SubTopic. Alpha_Type does this and it looks good.

IPoint is awkward. SkPoint and SkIPoint are named things; Point is a topic, which
refers to float points or integer points. There needn't be an IPoint topic.
One way to resolve this would be to combine SkPoint_Reference and SkIPoint_Reference into
Point_Reference that then contains both structs (or just move SKIPoint into SkPoint_Reference).
Most Point references would be replaced with SkPoint / SkIPoint (if that's what they mean),
or remain Point if the text indicates the concept rather one of the C structs.

see head of selfCheck.cpp for additional todos
see head of spellCheck.cpp for additional todos
 */

/*
  class contains named struct, enum, enum-member, method, topic, subtopic
     everything contained by class is uniquely named
     contained names may be reused by other classes
  method contains named parameters
     parameters may be reused in other methods
 */


// pass one: parse text, collect definitions
// pass two: lookup references

static int count_children(const Definition& def, MarkType markType) {
    int count = 0;
    if (markType == def.fMarkType) {
        ++count;
    }
    for (auto& child : def.fChildren ) {
        count += count_children(*child, markType);
    }
    return count;
}

int main(int argc, char** const argv) {
    BmhParser bmhParser(FLAGS_skip);
    bmhParser.validate();

    CommandLineFlags::SetUsage(
            "Common Usage: bookmaker -b path/to/bmh_files -i path/to/include.h -t\n"
            "              bookmaker -b path/to/bmh_files -e fiddle.json\n"
            "              ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json\n"
            "              bookmaker -b path/to/bmh_files -f fiddleout.json -r path/to/md_files\n"
            "              bookmaker -a path/to/status.json -x\n"
            "              bookmaker -a path/to/status.json -p\n");
    bool help = false;
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            help = true;
            for (int j = i + 1; j < argc; j++) {
                if (SkStrStartsWith(argv[j], '-')) {
                    break;
                }
                help = false;
            }
            break;
        }
    }
    if (!help) {
        CommandLineFlags::Parse(argc, argv);
    } else {
        CommandLineFlags::PrintUsage();
        const char* const commands[] = { "", "-h", "bmh", "-h", "examples", "-h", "include",
            "-h", "fiddle", "-h", "ref", "-h", "status", "-h", "tokens",
            "-h", "crosscheck", "-h", "populate", "-h", "spellcheck" };
        CommandLineFlags::Parse(SK_ARRAY_COUNT(commands), commands);
        return 0;
    }
    bool runAll = false;
    if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty() && FLAGS_status.isEmpty()) {
        FLAGS_status.set(0, "docs/status.json");
        if (FLAGS_extract) {
            FLAGS_examples.set(0, "fiddle.json");
        } else {
            FLAGS_fiddle.set(0, "fiddleout.json");
            FLAGS_ref.set(0, "site/user/api");
            runAll = true;
        }
    }
    if (!FLAGS_bmh.isEmpty() && !FLAGS_status.isEmpty()) {
        SkDebugf("requires -b or -a but not both\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (!FLAGS_include.isEmpty() && !FLAGS_status.isEmpty()) {
        SkDebugf("requires -i or -a but not both\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && FLAGS_catalog) {
         SkDebugf("-c requires -b or -a\n");
         CommandLineFlags::PrintUsage();
         return 1;
    }
    if ((FLAGS_fiddle.isEmpty() || FLAGS_ref.isEmpty()) && FLAGS_catalog) {
        SkDebugf("-c requires -f -r\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_examples.isEmpty()) {
        SkDebugf("-e requires -b or -a\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_status.isEmpty() &&
            FLAGS_populate) {
        SkDebugf("-p requires -b -i or -a\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_ref.isEmpty()) {
        SkDebugf("-r requires -b or -a\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_spellcheck.isEmpty()) {
        SkDebugf("-s requires -b or -a\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_include.isEmpty() && FLAGS_tokens) {
        SkDebugf("-t requires -b -i\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_status.isEmpty() &&
            FLAGS_crosscheck) {
        SkDebugf("-x requires -b -i or -a\n");
        CommandLineFlags::PrintUsage();
        return 1;
    }
    bmhParser.reset();
    if (!FLAGS_bmh.isEmpty()) {
        if (!bmhParser.parseFile(FLAGS_bmh[0], ".bmh", ParserCommon::OneFile::kNo)) {
            return -1;
        }
    } else if (!FLAGS_status.isEmpty()) {
        if (!bmhParser.parseStatus(FLAGS_status[0], ".bmh", StatusFilter::kInProgress)) {
            return -1;
        }
    }
    if (FLAGS_hack) {
        if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty()) {
            SkDebugf("-H or --hack requires -a or -b\n");
            CommandLineFlags::PrintUsage();
            return 1;
        }
        HackParser hacker(bmhParser);
        hacker.fDebugOut = FLAGS_stdout;
        if (!FLAGS_status.isEmpty() && !hacker.parseStatus(FLAGS_status[0], ".bmh",
                StatusFilter::kInProgress)) {
            SkDebugf("hack failed\n");
            return -1;
        }
        if (!FLAGS_bmh.isEmpty() && !hacker.parseFile(FLAGS_bmh[0], ".bmh",
                ParserCommon::OneFile::kNo)) {
            SkDebugf("hack failed\n");
            return -1;
        }
        return 0;
    }
    if (FLAGS_selfcheck && !SelfCheck(bmhParser)) {
        return -1;
    }
    if (!FLAGS_fiddle.isEmpty() && FLAGS_examples.isEmpty()) {
        FiddleParser fparser(&bmhParser);
        if (!fparser.parseFromFile(FLAGS_fiddle[0])) {
            return -1;
        }
    }
    if (runAll || (!FLAGS_catalog && !FLAGS_ref.isEmpty())) {
        IncludeParser includeParser;
        includeParser.validate();
        if (!FLAGS_status.isEmpty() && !includeParser.parseStatus(FLAGS_status[0], ".h",
                StatusFilter::kCompleted)) {
            return -1;
        }
        if (!FLAGS_include.isEmpty() && !includeParser.parseFile(FLAGS_include[0], ".h",
                ParserCommon::OneFile::kYes)) {
            return -1;
        }
        includeParser.fDebugWriteCodeBlock = FLAGS_stdout;
        includeParser.writeCodeBlock();
        MdOut mdOut(bmhParser, includeParser);
        mdOut.fDebugOut = FLAGS_stdout;
        mdOut.fDebugWriteCodeBlock = FLAGS_stdout;
        mdOut.fValidate = FLAGS_validate;
        if (!FLAGS_bmh.isEmpty() && mdOut.buildReferences(FLAGS_bmh[0], FLAGS_ref[0])) {
            bmhParser.fWroteOut = true;
        }
        if (!FLAGS_status.isEmpty() && mdOut.buildStatus(FLAGS_status[0], FLAGS_ref[0])) {
            bmhParser.fWroteOut = true;
        }
        if (FLAGS_validate) {
            mdOut.checkAnchors();
        }
    }
    if (runAll || (FLAGS_catalog && !FLAGS_ref.isEmpty())) {
        Catalog cparser(&bmhParser);
        cparser.fDebugOut = FLAGS_stdout;
        if (!FLAGS_bmh.isEmpty() && !cparser.openCatalog(FLAGS_bmh[0])) {
            return -1;
        }
        if (!FLAGS_status.isEmpty() && !cparser.openStatus(FLAGS_status[0])) {
            return -1;
        }
        if (!cparser.parseFile(FLAGS_fiddle[0], ".txt", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        if (!cparser.closeCatalog(FLAGS_ref[0])) {
            return -1;
        }
        bmhParser.fWroteOut = true;
    }
    if (FLAGS_tokens) {
        IncludeParser::RemoveFile(nullptr, FLAGS_include[0]);
        IncludeParser includeParser;
        includeParser.validate();
        if (!includeParser.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        includeParser.fDebugOut = FLAGS_stdout;
        if (includeParser.dumpTokens()) {
            bmhParser.fWroteOut = true;
        }
    }
    if (runAll || FLAGS_crosscheck) {
        IncludeParser includeParser;
        includeParser.validate();
        if (!FLAGS_include.isEmpty() &&
                !includeParser.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        if (!FLAGS_status.isEmpty() && !includeParser.parseStatus(FLAGS_status[0], ".h",
                StatusFilter::kCompleted)) {
            return -1;
        }
        if (!includeParser.crossCheck(bmhParser)) {
            return -1;
        }
    }
    if (runAll || FLAGS_populate) {
        IncludeWriter includeWriter;
        includeWriter.validate();
        if (!FLAGS_include.isEmpty() &&
                !includeWriter.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        if (!FLAGS_status.isEmpty() && !includeWriter.parseStatus(FLAGS_status[0], ".h",
                StatusFilter::kCompleted)) {
            return -1;
        }
        includeWriter.fDebugOut = FLAGS_stdout;
        if (!includeWriter.populate(bmhParser)) {
            return -1;
        }
        bmhParser.fWroteOut = true;
    }
    if (!FLAGS_spellcheck.isEmpty()) {
        if (!FLAGS_bmh.isEmpty()) {
            bmhParser.spellCheck(FLAGS_bmh[0], FLAGS_spellcheck);
        }
        if (!FLAGS_status.isEmpty()) {
            bmhParser.spellStatus(FLAGS_status[0], FLAGS_spellcheck);
        }
        bmhParser.fWroteOut = true;
    }
    if (!FLAGS_examples.isEmpty()) {
        // check to see if examples have duplicate names
        if (!bmhParser.checkExamples()) {
            return -1;
        }
        bmhParser.fDebugOut = FLAGS_stdout;
        if (!bmhParser.dumpExamples(FLAGS_examples[0])) {
            return -1;
        }
        return 0;
    }
    if (!bmhParser.fWroteOut) {
        int examples = 0;
        int methods = 0;
        int topics = 0;
        for (const auto& topic : bmhParser.fTopicMap) {
            if (topic.second->fParent) {
                continue;
            }
            examples += count_children(*topic.second, MarkType::kExample);
            methods += count_children(*topic.second, MarkType::kMethod);
            topics += count_children(*topic.second, MarkType::kSubtopic);
            topics += count_children(*topic.second, MarkType::kTopic);
        }
        SkDebugf("topics=%d classes=%d methods=%d examples=%d\n",
                bmhParser.fTopicMap.size(), bmhParser.fClassMap.size(),
                methods, examples);
    }
    return 0;
}

void NameMap::copyToParent(NameMap* parent) const {
    size_t colons = fName.rfind("::");
    string topName = string::npos == colons ? fName : fName.substr(colons + 2);
    for (auto& entry : fRefMap) {
        string scoped = topName + "::" + entry.first;
        SkASSERT(parent->fRefMap.end() == parent->fRefMap.find(scoped));
        parent->fRefMap[scoped] = entry.second;
        auto scopedLinkIter = fLinkMap.find(entry.first);
        if (fLinkMap.end() != scopedLinkIter) {
            SkASSERT(parent->fLinkMap.end() == parent->fLinkMap.find(scoped));
            parent->fLinkMap[scoped] = scopedLinkIter->second;
        }
    }
}

void NameMap::setParams(Definition* bmhDef, Definition* iMethod) {
    Definition* pParent = bmhDef->csParent();
    string parentName;
    if (pParent) {
        parentName = pParent->fName + "::";
        fParent = &pParent->asRoot()->fNames;
    }
    fName = parentName + iMethod->fName;
    TextParser methParams(iMethod);
    for (auto& param : iMethod->fTokens) {
        if (MarkType::kComment != param.fMarkType) {
            continue;
        }
        TextParser paramParser(&param);
        if (!paramParser.skipExact("@param ")) { // write parameters, if any
            continue;
        }
        paramParser.skipSpace();
        const char* start = paramParser.fChar;
        paramParser.skipToSpace();
        string paramName(start, paramParser.fChar - start);
    #ifdef SK_DEBUG
        for (char c : paramName) {
            SkASSERT(isalnum(c) || '_' == c);
        }
    #endif
        if (!methParams.containsWord(paramName.c_str(), methParams.fEnd, nullptr)) {
            param.reportError<void>("mismatched param name");
        }
        fRefMap[paramName] = &param;
        fLinkMap[paramName] = '#' + bmhDef->fFiddle + '_' + paramName;
    }
}

