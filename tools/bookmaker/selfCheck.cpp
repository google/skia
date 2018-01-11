/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

// Check that summary contains all methods

// Check that mutiple like-named methods are under one Subtopic

// Check that all subtopics are in table of contents

// Check that all constructors are in a table of contents
//          should be 'creators' instead of constructors?

// Check that SeeAlso reference each other

// Would be nice to check if other classes have 'create' methods that are included
//          SkSurface::makeImageSnapShot should be referenced under SkImage 'creators'

class SelfChecker {
public:
    SelfChecker(const BmhParser& bmh)
        : fBmhParser(bmh)
        {}

    bool check() {
        for (const auto& topic : fBmhParser.fTopicMap) {
            Definition* topicDef = topic.second;
            if (topicDef->fParent) {
                continue;
            }
            if (!topicDef->isRoot()) {
                return fBmhParser.reportError<bool>("expected root topic");
            }
            fRoot = topicDef->asRoot();
            if (!this->checkMethodSummary()) {
                return false;
            }
            if (!this->checkMethodSubtopic()) {
                return false;
            }
            if (!this->checkSubtopicContents()) {
                return false;
            }
            if (!this->checkConstructors()) {
                return false;
            }
            if (!this->checkSeeAlso()) {
                return false;
            }
            if (!this->checkCreators()) {
                return false;
            }
        }
        return true;
    }

protected:
    bool checkConstructors() {
        return true;
    }

    bool checkCreators() {
        return true;
    }

    bool checkMethodSubtopic() {
        return true;
    }

    bool checkMethodSummary() {
        SkDebugf("");
        // look for struct or class in fChildren
        for (auto& rootChild : fRoot->fChildren) {
            if (MarkType::kStruct == rootChild->fMarkType ||
                    MarkType::kClass == rootChild->fMarkType) {
                auto& cs = rootChild;
                // expect Overview as Topic in every main class or struct
                Definition* overview = nullptr;
                for (auto& csChild : cs->fChildren) {
                    if ("Overview" == csChild->fName) {
                        if (!overview) {
                            return cs->reportError<bool>("expected only one Overview");
                        }
                        overview = csChild;
                    }
                }
                if (!overview) {
                    return cs->reportError<bool>("missing #Topic Overview");
                }
                Definition* memberFunctions = nullptr;
                for (auto& overChild : overview->fChildren) {
                    if ("Member_Functions" == overChild->fName) {
                        memberFunctions = overChild;
                        break;
                    }
                }
                if (!memberFunctions) {
                    return overview->reportError<bool>("missing #Subtopic Member_Functions");
                }
                if (MarkType::kSubtopic != memberFunctions->fMarkType) {
                    return memberFunctions->reportError<bool>("expected #Subtopic Member_Functions");
                }
                Definition* memberTable = nullptr;
                for (auto& memberChild : memberFunctions->fChildren) {
                    if (MarkType::kTable == memberChild->fMarkType &&
                            memberChild->fName == memberFunctions->fName) {
                        memberTable = memberChild;
                        break;
                    }
                }
                if (!memberTable) {
                    return memberFunctions->reportError<bool>("missing #Table in Member_Functions");
                }
                vector<string> overviewEntries; // build map of overview entries
                bool expectLegend = true;
                string prior = " ";  // expect entries to be alphabetical
                for (auto& memberRow : memberTable->fChildren) {
                    if (MarkType::kLegend == memberRow->fMarkType) {
                        if (!expectLegend) {
                            return memberRow->reportError<bool>("expect #Legend only once");
                        }
                        // todo: check if legend format matches table's rows' format
                        expectLegend = false;
                    } else if (expectLegend) {
                        return memberRow->reportError<bool>("expect #Legend first");
                    }
                    if (MarkType::kRow != memberRow->fMarkType) {
                        continue;  // let anything through for now; can tighten up in the future
                    }
                    // expect column 0 to point to function name
                    // todo: content end points past space; could tighten that up
                    Definition* column0 = memberRow->fChildren[0];
                    string name = string(column0->fContentStart,
                            column0->fTerminator - column0->fContentStart);
                    if (prior > name) {
                        return memberRow->reportError<bool>("expect alphabetical order");
                    }
                    if (prior == name) {
                        return memberRow->reportError<bool>("expect unique names");
                    }
                    // todo: error if name is all lower case and doesn't end in ()
                    overviewEntries.push_back(name);
                    prior = name;
                }
                // mark corresponding methods as visited (may be more than one per entry)
                for (auto& csChild : cs->fChildren) {
                    if (MarkType::kMethod != csChild->fMarkType) {
                        // only check methods for now
                        continue;
                    }
                    auto start = csChild->fName.find_last_of(':');
                    start = string::npos == start ? 0 : start + 1;
                    string name = csChild->fName.substr(start);
                    if (overviewEntries.end() ==
                            std::find(overviewEntries.begin(), overviewEntries.end(), name)) {
                        return csChild->reportError<bool>("missing in Overview");
                    }
                }
            }
        }
        return true;
    }

    bool checkSeeAlso() {
        return true;
    }

    bool checkSubtopicContents() {
        return true;
    }

private:
    const BmhParser& fBmhParser;
    RootDefinition* fRoot;
};

bool SelfCheck(const BmhParser& bmh) {
    SelfChecker checker(bmh);
    return checker.check();
}
