// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package gen_tasks_logic

import (
	"log"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"go.skia.org/infra/go/cas/rbe"
	"go.skia.org/infra/task_scheduler/go/specs"
)

const (
	// If a parent path contains more than this many immediate child paths (ie.
	// files and dirs which are directly inside it as opposed to indirect
	// descendants), we will include the parent in the isolate file instead of
	// the children. This results in a simpler CasSpec which should need to be
	// changed less often.
	combinePathsThreshold = 3
)

var (
	// Any files in Git which match these patterns will be included, either
	// directly or indirectly via a parent dir.
	pathRegexes = []*regexp.Regexp{
		regexp.MustCompile(`.*\.c$`),
		regexp.MustCompile(`.*\.cc$`),
		regexp.MustCompile(`.*\.cpp$`),
		regexp.MustCompile(`.*\.gn$`),
		regexp.MustCompile(`.*\.gni$`),
		regexp.MustCompile(`.*\.h$`),
		regexp.MustCompile(`.*\.mm$`),
		regexp.MustCompile(`.*\.storyboard$`),
	}

	// These paths are always added to the inclusion list. Note that they may
	// not appear in the CasSpec if they are included indirectly via a parent
	// dir.
	explicitPaths = []string{
		".bazelrc",
		".bazelversion",
		".clang-format",
		".clang-tidy",
		".vpython3",
		"BUILD.bazel",
		"DEPS", // Needed by bin/fetch-ninja
		"WORKSPACE.bazel",
		"bazel",
		"bin/activate-emsdk",
		"bin/fetch-clang-format",
		"bin/fetch-gn",
		"bin/fetch-ninja",
		"buildtools",
		"example",
		"go_repositories.bzl",
		"infra/bots/assets/android_ndk_darwin/VERSION",
		"infra/bots/assets/android_ndk_linux/VERSION",
		"infra/bots/assets/android_ndk_windows/VERSION",
		"infra/bots/assets/cast_toolchain/VERSION",
		"infra/bots/assets/clang_linux/VERSION",
		"infra/bots/assets/clang_win/VERSION",
		"infra/bots/run_recipe.py",
		"infra/bots/task_drivers",
		"infra/canvaskit",
		"infra/pathkit",
		"package.json",
		"package-lock.json",
		"requirements.txt",
		"resources",
		"third_party/externals",
		"toolchain",
	}
)

// getAllCheckedInPaths returns every path checked in to the repo.
func getAllCheckedInPaths(cfg *Config) []string {
	cmd := exec.Command("git", "ls-files")
	// Use cfg.PathToSkia to get to the Skia checkout, in case this is used by
	// another repo.
	cmd.Dir = filepath.Join(CheckoutRoot(), cfg.PathToSkia)
	output, err := cmd.CombinedOutput()
	if err != nil {
		log.Fatal(err)
	}
	split := strings.Split(string(output), "\n")
	rv := make([]string, 0, len(split))
	for _, line := range split {
		if line != "" {
			rv = append(rv, line)
		}
	}
	return rv
}

// getRelevantPaths returns all paths needed by compile tasks.
func getRelevantPaths(cfg *Config) []string {
	rv := []string{}
	for _, path := range getAllCheckedInPaths(cfg) {
		for _, regex := range pathRegexes {
			if regex.MatchString(path) {
				rv = append(rv, path)
				break
			}
		}
	}
	return append(rv, explicitPaths...)
}

// node is a single node in a directory tree of task inputs.
type node struct {
	children map[string]*node
	name     string
	isLeaf   bool
}

// newNode returns a node instance.
func newNode(name string) *node {
	return &node{
		children: map[string]*node{},
		name:     name,
		isLeaf:   false,
	}
}

// isRoot returns true iff this is the root node.
func (n *node) isRoot() bool {
	return n.name == ""
}

// add the given entry (given as a slice of path components) to the node.
func (n *node) add(entry []string) {
	// Remove the first element if we're not the root node.
	if !n.isRoot() {
		if entry[0] != n.name {
			log.Fatalf("Failed to compute compile CAS inputs; attempting to add entry %v to node %q", entry, n.name)
		}
		entry = entry[1:]

		// If the entry is now empty, this node is a leaf.
		if len(entry) == 0 {
			n.isLeaf = true
			return
		}
	}

	// Add a child node.
	if !n.isLeaf {
		name := entry[0]
		child, ok := n.children[name]
		if !ok {
			child = newNode(name)
			n.children[name] = child
		}
		child.add(entry)

		// If we have more than combinePathsThreshold immediate children,
		// combine them into this node.
		immediateChilden := 0
		for _, child := range n.children {
			if child.isLeaf {
				immediateChilden++
			}
			if !n.isRoot() && immediateChilden >= combinePathsThreshold {
				n.isLeaf = true
				n.children = map[string]*node{}
			}
		}
	}
}

// entries returns the entries represented by this node and its children.
// Will not return children in the following cases:
//   - This Node is a leaf, ie. it represents an entry which was explicitly
//     inserted into the Tree, as opposed to only part of a path to other
//     entries.
//   - This Node has immediate children exceeding combinePathsThreshold and
//     thus has been upgraded to a leaf node.
func (n *node) entries() [][]string {
	if n.isLeaf {
		return [][]string{{n.name}}
	}
	rv := [][]string{}
	for _, child := range n.children {
		for _, entry := range child.entries() {
			if !n.isRoot() {
				entry = append([]string{n.name}, entry...)
			}
			rv = append(rv, entry)
		}
	}
	return rv
}

// tree represents a directory tree of task inputs.
type tree struct {
	root *node
}

// newTree returns a tree instance.
func newTree() *tree {
	return &tree{
		root: newNode(""),
	}
}

// add the given path to the tree. Entries may be combined as defined by
// combinePathsThreshold.
func (t *tree) add(p string) {
	split := strings.Split(p, "/")
	t.root.add(split)
}

// entries returns all entries in the tree. Entries may be combined as defined
// by combinePathsThreshold.
func (t *tree) entries() []string {
	entries := t.root.entries()
	rv := make([]string, 0, len(entries))
	for _, entry := range entries {
		rv = append(rv, strings.Join(append([]string{"skia"}, entry...), "/"))
	}
	sort.Strings(rv)
	return rv
}

// generateCompileCAS creates the CasSpec used for tasks which build Skia.
func generateCompileCAS(b *specs.TasksCfgBuilder, cfg *Config) {
	t := newTree()
	for _, path := range getRelevantPaths(cfg) {
		t.add(path)
	}
	spec := &specs.CasSpec{
		Root:     "..",
		Paths:    t.entries(),
		Excludes: []string{rbe.ExcludeGitDir},
	}
	b.MustAddCasSpec(CAS_COMPILE, spec)
}
