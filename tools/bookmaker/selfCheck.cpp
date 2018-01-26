/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"


// Check that mutiple like-named methods are under one Subtopic

// Check that all subtopics are in table of contents

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
            if (!this->checkSubtopicSummary()) {
                return false;
            }
            if (!this->checkConstructorsSummary()) {
                return false;
            }
            if (!this->checkOperatorsSummary()) {
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
    // Check that all constructors are in a table of contents
    //          should be 'creators' instead of constructors?
    bool checkConstructorsSummary() {
        for (auto& rootChild : fRoot->fChildren) {
            if (!this->isStructOrClass(rootChild)) {
                continue;
            }
            auto& cs = rootChild;
			auto constructors = this->findTopic("Constructors", Optional::kYes);
			if (constructors && MarkType::kSubtopic != constructors->fMarkType) {
                return constructors->reportError<bool>("expected #Subtopic Constructors");
            }
            vector<string> constructorEntries;
            if (constructors) {
                if (!this->collectEntries(constructors, &constructorEntries)) {
                    return false;
                }
            }
            // mark corresponding methods as visited (may be more than one per entry)
            for (auto& csChild : cs->fChildren) {
                if (MarkType::kMethod != csChild->fMarkType) {
                    // only check methods for now
                    continue;
                }
                string name;
                if (!this->childName(csChild, &name)) {
                    return false;
                }
                string returnType;
                if (Definition::MethodType::kConstructor != csChild->fMethodType &&
                        Definition::MethodType::kDestructor != csChild->fMethodType) {
                    string makeCheck = name.substr(0, 4);
                    if ("Make" != makeCheck && "make" != makeCheck) {
                        continue;
                    }
                    // for now, assume return type of interest is first word to start Sk
                    string search(csChild->fStart, csChild->fContentStart - csChild->fStart);
                    auto end = search.find(makeCheck);
                    if (string::npos == end) {
                        return csChild->reportError<bool>("expected Make in content");
                    }
                    search = search.substr(0, end);
                    if (string::npos == search.find(cs->fName)) {
                        // if return value doesn't match current struct or class, look in
                        // returned struct / class instead
                        auto sk = search.find("Sk");
                        if (string::npos != sk) {
                            // todo: build class name, find it, search for match in its overview
                            continue;
                        }
                    }
                }
                if (constructorEntries.end() ==
                        std::find(constructorEntries.begin(), constructorEntries.end(), name)) {
                    return csChild->reportError<bool>("missing constructor in Constructors");
                }
            }
        }
        return true;
    }

    bool checkCreators() {
        return true;
    }

    bool checkMethodSubtopic() {
        return true;
    }

    // Check that summary contains all methods
    bool checkMethodSummary() {
        // look for struct or class in fChildren
		Definition* cs = nullptr;
		for (auto& rootChild : fRoot->fChildren) {
			if (!this->isStructOrClass(rootChild)) {
				continue;
			}
			cs = rootChild;
			// expect Overview as Topic in every main class or struct or its parent
		}
		if (!cs) {
			return true;  // topics may not have included classes or structs
		}
		auto memberFunctions = this->findTopic("Member_Functions", Optional::kNo);
        if (MarkType::kSubtopic != memberFunctions->fMarkType) {
            return memberFunctions->reportError<bool>("expected #Subtopic Member_Functions");
        }
        vector<string> methodEntries; // build map of overview entries
        if (!this->collectEntries(memberFunctions, &methodEntries)) {
            return false;
        }
        // mark corresponding methods as visited (may be more than one per entry)
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
            string name;
            if (!this->childName(csChild, &name)) {
                return false;
            }
            if (methodEntries.end() ==
                    std::find(methodEntries.begin(), methodEntries.end(), name)) {
                return csChild->reportError<bool>("missing method in Member_Functions");
            }
        }
        return true;
    }

    // Check that all operators are in a table of contents
    bool checkOperatorsSummary() {
        for (auto& rootChild : fRoot->fChildren) {
            if (!this->isStructOrClass(rootChild)) {
                continue;
            }
            auto& cs = rootChild;
            const Definition* operators = this->findTopic("Operators", Optional::kYes);
            if (operators && MarkType::kSubtopic != operators->fMarkType) {
                return operators->reportError<bool>("expected #Subtopic Operators");
            }
            vector<string> operatorEntries;
            if (operators) {
                if (!this->collectEntries(operators, &operatorEntries)) {
                    return false;
                }
            }
            for (auto& csChild : cs->fChildren) {
                if (Definition::MethodType::kOperator != csChild->fMethodType) {
                    continue;
                }
                string name;
                if (!this->childName(csChild, &name)) {
                    return false;
                }
                bool found = false;
                for (auto str : operatorEntries) {
                    if (string::npos != str.find(name)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return csChild->reportError<bool>("missing operator in Operators");
                }
            }
        }
        return true;
    }

    bool checkSeeAlso() {
        return true;
    }

    bool checkSubtopicSummary() {
        for (auto& rootChild : fRoot->fChildren) {
            if (!this->isStructOrClass(rootChild)) {
                continue;
            }
            auto& cs = rootChild;
            auto overview = this->findOverview(cs);
            if (!overview) {
                return false;
            }
            const Definition* subtopics = this->findTopic("Subtopics", Optional::kNo);
            if (MarkType::kSubtopic != subtopics->fMarkType) {
                return subtopics->reportError<bool>("expected #Subtopic Subtopics");
            }
			const Definition* relatedFunctions = this->findTopic("Related_Functions", Optional::kYes);
			if (relatedFunctions && MarkType::kSubtopic != relatedFunctions->fMarkType) {
                return relatedFunctions->reportError<bool>("expected #Subtopic Related_Functions");
            }
            vector<string> subtopicEntries;
            if (!this->collectEntries(subtopics, &subtopicEntries)) {
                return false;
            }
            if (relatedFunctions && !this->collectEntries(relatedFunctions, &subtopicEntries)) {
                return false;
            }
            for (auto& csChild : cs->fChildren) {
                if (MarkType::kSubtopic != csChild->fMarkType) {
                    continue;
                }
                string name;
                if (!this->childName(csChild, &name)) {
                    return false;
                }
                bool found = false;
                for (auto str : subtopicEntries) {
                    if (string::npos != str.find(name)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return csChild->reportError<bool>("missing SubTopic in SubTopics");
                }
            }
        }
        return true;
    }

    bool childName(const Definition* def, string* name) {
        auto start = def->fName.find_last_of(':');
        start = string::npos == start ? 0 : start + 1;
        *name = def->fName.substr(start);
        if (def->fClone) {
            auto lastUnderline = name->find_last_of('_');
            if (string::npos == lastUnderline) {
                return def->reportError<bool>("expect _ in name");
            }
            if (lastUnderline + 1 >= name->length()) {
                return def->reportError<bool>("expect char after _ in name");
            }
            for (auto index = lastUnderline + 1; index < name->length(); ++index) {
                if (!isdigit((*name)[index])) {
                    return def->reportError<bool>("expect digit after _ in name");
                }
            }
            *name = name->substr(0, lastUnderline);
            bool allLower = true;
            for (auto ch : *name) {
                allLower &= (bool) islower(ch);
            }
            if (allLower) {
                *name += "()";
            }
        }
        return true;
    }

	static const Definition* overview_def(const Definition* parent) {
		Definition* overview = nullptr;
		if (parent) {
			for (auto& csChild : parent->fChildren) {
				if ("Overview" == csChild->fName) {
					if (overview) {
						return csChild->reportError<const Definition*>("expected only one Overview");
					}
					overview = csChild;
				}
			}
		}
		return overview;
	}

    const Definition* findOverview(const Definition* parent) {
        // expect Overview as Topic in every main class or struct
        const Definition* overview = overview_def(parent);
		const Definition* parentOverview = overview_def(parent->fParent);
		if (overview && parentOverview) {
			return overview->reportError<const Definition*>("expected only one Overview 2");
		}
		overview = overview ? overview : parentOverview;
        if (!overview) {
            return parent->reportError<const Definition*>("missing #Topic Overview");
        }
        return overview;
    }

	enum class Optional {
		kNo,
		kYes,
	};

	const Definition* findTopic(string name, Optional optional) {
		string topicKey = fRoot->fName + '_' + name;
		auto topicKeyIter = fBmhParser.fTopicMap.find(topicKey);
		if (fBmhParser.fTopicMap.end() == topicKeyIter) {
			// TODO: remove this and require member functions outside of overview
			topicKey = fRoot->fName + "_Overview_" + name;  // legacy form for now
			topicKeyIter = fBmhParser.fTopicMap.find(topicKey);
			if (fBmhParser.fTopicMap.end() == topicKeyIter) {
				if (Optional::kNo == optional) {
					return fRoot->reportError<Definition*>("missing subtopic");
				}
				return nullptr;
			}
		}
		return topicKeyIter->second;
	}

    bool collectEntries(const Definition* entries, vector<string>* strings) {
        const Definition* table = nullptr;
        for (auto& child : entries->fChildren) {
            if (MarkType::kTable == child->fMarkType && child->fName == entries->fName) {
                table = child;
                break;
            }
        }
        if (!table) {
            return entries->reportError<bool>("missing #Table in Overview Subtopic");
        }
        bool expectLegend = true;
        string prior = " ";  // expect entries to be alphabetical
        for (auto& row : table->fChildren) {
            if (MarkType::kLegend == row->fMarkType) {
                if (!expectLegend) {
                    return row->reportError<bool>("expect #Legend only once");
                }
                // todo: check if legend format matches table's rows' format
                expectLegend = false;
            } else if (expectLegend) {
                return row->reportError<bool>("expect #Legend first");
            }
            if (MarkType::kRow != row->fMarkType) {
                continue;  // let anything through for now; can tighten up in the future
            }
            // expect column 0 to point to function name
            Definition* column0 = row->fChildren[0];
            string name = string(column0->fContentStart,
                    column0->fContentEnd - column0->fContentStart);
            if (prior > name) {
                return row->reportError<bool>("expect alphabetical order");
            }
            if (prior == name) {
                return row->reportError<bool>("expect unique names");
            }
            // todo: error if name is all lower case and doesn't end in ()
            strings->push_back(name);
            prior = name;
        }
        return true;
    }

    bool isStructOrClass(const Definition* definition) {
        if (MarkType::kStruct != definition->fMarkType &&
                MarkType::kClass != definition->fMarkType) {
            return false;
        }
        if (string::npos != definition->fFileName.find("undocumented.bmh")) {
            return false;
        }
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
