// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"fmt"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
)

const (
	ruleOnlyRepoPattern = `^(@\w+)$`
	rulePattern         = `^(?P<repo>@[^/]+)?/(?P<path>[^:]+)(?P<target>:[^:]+)?$`
	locationPattern     = `^(?P<path>[^:]+):(?P<line>[^:]+):(?P<pos>[^:]+)$`
)

var (
	ruleOnlyRepoRegex = regexp.MustCompile(ruleOnlyRepoPattern)
	ruleRegex         = regexp.MustCompile(rulePattern)
	locRegex          = regexp.MustCompile(locationPattern)
)

// Return true if the given rule name represents an external repository.
func isExternalRule(name string) bool {
	return name[0] == '@'
}

// Given a Bazel rule name find that rule from within the
// query results. Returns nil if the given rule is not present.
func findRule(qr *analysis_v2.CqueryResult, name string) *build.Rule {
	for _, result := range qr.GetResults() {
		r := result.GetTarget().GetRule()
		if r.GetName() == name {
			return r
		}
	}
	return nil
}

// Parse a rule into its constituent parts.
// https://docs.bazel.build/versions/main/guide.html#specifying-targets-to-build
//
// For example, the input rule `//foo/bar:baz` will return:
//
//	repo: ""
//	path: "/foo/bar"
//	target: "baz"
func parseRule(rule string) (repo string, path string, target string, err error) {
	match := ruleOnlyRepoRegex.FindStringSubmatch(rule)
	if match != nil {
		return match[1], "/", strings.TrimPrefix(match[1], "@"), nil
	}

	match = ruleRegex.FindStringSubmatch(rule)
	if match == nil {
		return "", "", "", skerr.Fmt(`Unable to match rule %q`, rule)
	}

	if len(match[3]) > 0 {
		target = strings.TrimPrefix(match[3], ":")
	} else {
		// No explicit target, so use directory name as default target.
		target = filepath.Base(match[2])
	}

	return match[1], match[2], target, nil
}

// Parse a file location into its three constituent parts.
//
// A location is of the form:
//
//	/full/path/to/BUILD.bazel:33:20
func parseLocation(location string) (path string, line int, pos int, err error) {
	match := locRegex.FindStringSubmatch(location)
	if match == nil {
		return "", 0, 0, skerr.Fmt(`unable to match file location %q`, location)
	}
	path = match[1]
	line, err = strconv.Atoi(match[2])
	if err != nil {
		return "", 0, 0, skerr.Fmt(`unable to parse line no. %q`, match[2])
	}
	pos, err = strconv.Atoi(match[3])
	if err != nil {
		return "", 0, 0, skerr.Fmt(`unable to parse pos. %q`, match[3])
	}
	return path, line, pos, nil
}

// Return the directory containing the file in the location string.
func getLocationDir(location string) (string, error) {
	filePath, _, _, err := parseLocation(location)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	return filepath.Dir(filePath), nil
}

func makeCanonicalRuleName(bazelRuleName string) (string, error) {
	repo, path, target, err := parseRule(bazelRuleName)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	return fmt.Sprintf("%s/%s:%s", repo, path, target), nil
}

// Determine if a target refers to a file, or a rule. target is of
// the form:
//
// file: //include/private:SingleOwner.h
// rule: //bazel/common_config_settings:has_gpu_backend
func isFileTarget(target string) bool {
	_, _, target, err := parseRule(target)
	if err != nil {
		return false
	}
	return strings.Contains(target, ".")
}

// Create a string that uniquely identifies the rule and can be used
// in the exported project file as a valid name.
func getRuleSimpleName(bazelRuleName string) (string, error) {
	s, err := makeCanonicalRuleName(bazelRuleName)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	s = strings.TrimPrefix(s, "//:")
	s = strings.TrimPrefix(s, "//")
	s = strings.ReplaceAll(s, "//", "_")
	s = strings.ReplaceAll(s, "@", "at_")
	s = strings.ReplaceAll(s, "/", "_")
	s = strings.ReplaceAll(s, ":", "_")
	s = strings.ReplaceAll(s, "__", "_")
	return s, nil
}

// Append all elements to the slice if not already present in the slice.
func appendUnique(slice []string, elems ...string) []string {
	for _, elem := range elems {
		if !util.In(elem, slice) {
			slice = append(slice, elem)
		}
	}
	return slice
}

// Retrieve (if present) a slice of string attribute values from the given
// rule and attribute name. A nil slice will be returned if the attribute
// does not exist in the rule. A slice of strings (possibly empty) will be
// returned if the attribute is empty. An error will be returned if the
// attribute is not a list type.
func getRuleStringArrayAttribute(r *build.Rule, name string) ([]string, error) {
	for _, attrib := range r.Attribute {
		if attrib.GetName() != name {
			continue
		}
		if attrib.GetType() != build.Attribute_LABEL_LIST &&
			attrib.GetType() != build.Attribute_STRING_LIST {
			return nil, skerr.Fmt(`%s in rule %q is not a list`, name, r.GetName())
		}
		return attrib.GetStringListValue(), nil
	}
	return nil, nil
}

// Given an input rule target return the workspace relative file path.
// For example, an input of `//src/core:source.cpp` will return
// `src/core/source.cpp`.
func getFilePathFromFileTarget(target string) (string, error) {
	_, path, t, err := parseRule(target)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	if !isFileTarget(target) {
		return "", skerr.Fmt("Target %q is not a file target.", target)
	}
	return filepath.Join(strings.TrimPrefix(path, "/"), t), nil
}
