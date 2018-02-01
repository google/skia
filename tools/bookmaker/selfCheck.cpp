/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

// Check that mutiple like-named methods are under one Subtopic

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
            if (!this->checkSeeAlso()) {
                return false;
            }
            // report functions that are not covered by related hierarchy
			if (!this->checkRelatedFunctions()) {
				return false;
			}
        }
        return true;
    }

protected:

	bool checkRelatedFunctions() {
		const Definition* cs = this->classOrStruct();
		vector<string> methodNames;
		if (cs) {
			string prefix = cs->fName + "::";
			for (auto& csChild : cs->fChildren) {
				if (MarkType::kMethod != csChild->fMarkType) {
					// only check methods for now
					continue;
				}
				if (Definition::MethodType::kConstructor == csChild->fMethodType) {
					continue;
				}
				if (Definition::MethodType::kDestructor == csChild->fMethodType) {
					continue;
				}
				if (Definition::MethodType::kOperator == csChild->fMethodType) {
					continue;
				}
				if (csChild->fClone) {
					// FIXME: check to see if all cloned methods are in table
					// since format of clones is in flux, defer this check for now
					continue;
				}
                bool containsMarkTypeIn = false;
                for (auto child : csChild->fChildren) {
                    if (MarkType::kIn == child->fMarkType) {
                        containsMarkTypeIn = true;
                        break;
                    }
                }
                if (!containsMarkTypeIn) {
                    SkDebugf("No #In: %s\n", csChild->fName.c_str());
                }
			}
		}
		return true;
	}

    bool checkSeeAlso() {
        return true;
    }

	const Definition* classOrStruct() {
		for (auto& rootChild : fRoot->fChildren) {
			if (rootChild->isStructOrClass()) {
				return rootChild;
			}
		}
		return nullptr;
	}

	enum class Optional {
		kNo,
		kYes,
	};

private:
    const BmhParser& fBmhParser;
    RootDefinition* fRoot;
};

bool SelfCheck(const BmhParser& bmh) {
    SelfChecker checker(bmh);
    return checker.check();
}
