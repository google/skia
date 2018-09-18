/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#ifdef SK_BUILD_FOR_WIN
#include <windows.h>
#endif


 /* SkDebugf works in both visual studio and git shell, but
 in git shell output is not piped to grep.
 printf does not generate output in visual studio, but
 does in git shell and can be piped.
 */
#ifdef SK_BUILD_FOR_WIN
#define PRINTF(...)                 \
do {                                \
    if (IsDebuggerPresent()) {      \
        SkDebugf(__VA_ARGS__);      \
    } else {                        \
        printf(__VA_ARGS__);        \
    }                               \
} while (false)
#else
#define PRINTF(...)                 \
        printf(__VA_ARGS__)
#endif


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

    void checkMethod(string topic, const Definition* csChild, vector<string>* reported) {
        if (MarkType::kSubtopic == csChild->fMarkType) {
            for (auto child : csChild->fChildren) {
                checkMethod(topic, child, reported);
            }
            return;
        } else if (MarkType::kMethod != csChild->fMarkType) {
            // only check methods for now
            return;
        }
        bool containsMarkTypeIn = csChild->fDeprecated  // no markup for deprecated
                || Definition::MethodType::kConstructor == csChild->fMethodType
                || Definition::MethodType::kDestructor == csChild->fMethodType
                || Definition::MethodType::kOperator == csChild->fMethodType
                || csChild->fClone;
        for (auto child : csChild->fChildren) {
            if (MarkType::kIn == child->fMarkType) {
                containsMarkTypeIn = true;
                string subtopic(child->fContentStart,
                    child->fContentEnd - child->fContentStart);
                string fullname = topic + '_' + subtopic;
                auto topEnd = fBmhParser.fTopicMap.end();
                auto topFind = fBmhParser.fTopicMap.find(fullname);
                auto reportEnd = reported->end();
                auto reportFind = std::find(reported->begin(), reported->end(), subtopic);
                if (topEnd == topFind) {
                    if (reportEnd == reportFind) {
                        reported->push_back(subtopic);
                    }
                }
            }
        }
        if (!containsMarkTypeIn) {
            PRINTF("No #In: %s\n", csChild->fName.c_str());
        }
    }

	bool checkRelatedFunctions() {
		const Definition* cs = this->classOrStruct();
        if (!cs) {
            return true;
        }
        const Definition* topic = cs->fParent;
        SkASSERT(topic);
        SkASSERT(MarkType::kTopic == topic->fMarkType);
        string topicName = topic->fName;
        vector<string> methodNames;
        vector<string> reported;
		string prefix = cs->fName + "::";
		for (auto& csChild : cs->fChildren) {
            checkMethod(topicName, csChild, &reported);
		}
        for (auto missing : reported) {
            string fullname = topicName + '_' + missing;
            PRINTF("No #Subtopic: %s\n", fullname.c_str());
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
