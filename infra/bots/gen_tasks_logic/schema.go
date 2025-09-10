// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

/*
   This file contains logic related to task/job name schemas.
*/

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"regexp"
	"strings"
)

// Parts represents the key/value pairs which make up task and job names.
type Parts map[string]string

// Equal returns true if the given part of this job's name equals any of the
// given values. Panics if no values are provided.
func (p Parts) Equal(part string, eq ...string) bool {
	if len(eq) == 0 {
		log.Fatal("No values provided for equal!")
	}
	v := p[part]
	for _, e := range eq {
		if v == e {
			return true
		}
	}
	return false
}

// Role returns true if the Role for this job equals any of the given values.
func (p Parts) Role(eq ...string) bool {
	return p.Equal("role", eq...)
}

// Os returns true if the OS for this job equals any of the given values.
func (p Parts) Os(eq ...string) bool {
	return p.Equal("os", eq...)
}

// Compiler returns true if the Compiler for this job equals any of the given
// values.
func (p Parts) Compiler(eq ...string) bool {
	return p.Equal("compiler", eq...)
}

// Model returns true if the Model for this job equals any of the given values.
func (p Parts) Model(eq ...string) bool {
	return p.Equal("model", eq...)
}

// Frequency returns true if the Frequency for this job equals any of the given
// values.
func (p Parts) Frequency(eq ...string) bool {
	return p.Equal("frequency", eq...)
}

// CPU returns true if the task's cpu_or_gpu is "CPU" and the CPU for this
// task equals any of the given values. If no values are provided, CPU returns
// true if this task runs on CPU.
func (p Parts) CPU(eq ...string) bool {
	if p["cpu_or_gpu"] == "CPU" {
		if len(eq) == 0 {
			return true
		}
		return p.Equal("cpu_or_gpu_value", eq...)
	}
	return false
}

// GPU returns true if the task's cpu_or_gpu is "GPU" and the GPU for this task
// equals any of the given values. If no values are provided, GPU returns true
// if this task runs on GPU.
func (p Parts) GPU(eq ...string) bool {
	if p["cpu_or_gpu"] == "GPU" {
		if len(eq) == 0 {
			return true
		}
		return p.Equal("cpu_or_gpu_value", eq...)
	}
	return false
}

// Arch returns true if the architecture for this job equals any of the
// given values.
func (p Parts) Arch(eq ...string) bool {
	return p.Equal("arch", eq...) || p.Equal("target_arch", eq...)
}

// ExtraConfig returns true if any of the extra_configs for this job equals
// any of the given values. If the extra_config starts with "SK_",
// it is considered to be a single config.
func (p Parts) ExtraConfig(eq ...string) bool {
	if len(eq) == 0 {
		log.Fatal("No values provided for extraConfig()!")
	}
	ec := p["extra_config"]
	if ec == "" {
		return false
	}
	splitCfgs := strings.Split(ec, "_")
	cfgs := make([]string, 0, len(splitCfgs))
	for idx, c := range splitCfgs {
		if c == "SK" {
			cfgs = append(cfgs, strings.Join(cfgs[idx:], "_"))
			break
		}
		cfgs = append(cfgs, c)
	}
	for _, c := range cfgs {
		for _, e := range eq {
			if c == e {
				return true
			}
		}
	}
	return false
}

// NoExtraConfig returns true if there are no extra_configs for this job.
func (p Parts) NoExtraConfig(eq ...string) bool {
	ec := p["extra_config"]
	return ec == ""
}

// MatchPart returns true if the given part of this job's name matches any of
// the given regular expressions. Note that a regular expression might match any
// substring, so if you need an exact match on the entire string you'll need to
// use `^` and `$`. Panics if no regular expressions are provided.
func (p Parts) MatchPart(part string, re ...string) bool {
	if len(re) == 0 {
		log.Fatal("No regular expressions provided for matchPart()!")
	}
	v := p[part]
	for _, r := range re {
		if regexp.MustCompile(r).MatchString(v) {
			return true
		}
	}
	return false
}

// MatchRole returns true if the role for this job matches any of the given
// regular expressions.
func (p Parts) MatchRole(re ...string) bool {
	return p.MatchPart("role", re...)
}

func (p Parts) Project(re ...string) bool {
	return p.MatchPart("project", re...)
}

// MatchOs returns true if the OS for this job matches any of the given regular
// expressions.
func (p Parts) MatchOs(re ...string) bool {
	return p.MatchPart("os", re...)
}

// MatchCompiler returns true if the compiler for this job matches any of the
// given regular expressions.
func (p Parts) MatchCompiler(re ...string) bool {
	return p.MatchPart("compiler", re...)
}

// MatchModel returns true if the model for this job matches any of the given
// regular expressions.
func (p Parts) MatchModel(re ...string) bool {
	return p.MatchPart("model", re...)
}

// MatchCpu returns true if the task's cpu_or_gpu is "CPU" and the CPU for this
// task matches any of the given regular expressions. If no regular expressions
// are provided, cpu returns true if this task runs on CPU.
func (p Parts) MatchCpu(re ...string) bool {
	if p["cpu_or_gpu"] == "CPU" {
		if len(re) == 0 {
			return true
		}
		return p.MatchPart("cpu_or_gpu_value", re...)
	}
	return false
}

// MatchGpu returns true if the task's cpu_or_gpu is "GPU" and the GPU for this task
// matches any of the given regular expressions. If no regular expressions are
// provided, gpu returns true if this task runs on GPU.
func (p Parts) MatchGpu(re ...string) bool {
	if p["cpu_or_gpu"] == "GPU" {
		if len(re) == 0 {
			return true
		}
		return p.MatchPart("cpu_or_gpu_value", re...)
	}
	return false
}

// MatchArch returns true if the architecture for this job matches any of the
// given regular expressions.
func (p Parts) MatchArch(re ...string) bool {
	return p.MatchPart("arch", re...) || p.MatchPart("target_arch", re...)
}

// MatchBazelHost returns true if the Bazel host for this job matches any of the
// given regular expressions.
func (p Parts) MatchBazelHost(re ...string) bool {
	return p.MatchPart("host", re...)
}

// MatchExtraConfig returns true if any of the extra_configs for this job matches
// any of the given regular expressions. If the extra_config starts with "SK_",
// it is considered to be a single config.
func (p Parts) MatchExtraConfig(re ...string) bool {
	if len(re) == 0 {
		log.Fatal("No regular expressions provided for matchExtraConfig()!")
	}
	ec := p["extra_config"]
	if ec == "" {
		return false
	}
	var cfgs []string
	if strings.HasPrefix(ec, "SK_") {
		cfgs = []string{ec}
	} else {
		cfgs = strings.Split(ec, "_")
	}
	compiled := make([]*regexp.Regexp, 0, len(re))
	for _, r := range re {
		compiled = append(compiled, regexp.MustCompile(r))
	}
	for _, c := range cfgs {
		for _, r := range compiled {
			if r.MatchString(c) {
				return true
			}
		}
	}
	return false
}

// Debug returns true if this task runs in Debug mode.
func (p Parts) Debug() bool {
	return p["configuration"] == "Debug"
}

// Release returns true if this task runs in Release mode.
func (p Parts) Release() bool {
	return p["configuration"] == "Release"
}

// IsLinux returns true if the task runs on Linux.
func (p Parts) IsLinux() bool {
	return p.MatchOs("Debian", "Ubuntu") ||
		p.MatchExtraConfig("Debian", "Ubuntu") ||
		p.MatchBazelHost("linux", "on_rpi") ||
		p.Role("Housekeeper", "Canary", "Upload")

}

// IsWindows returns true if the task runs on Windows.
func (p Parts) IsWindows() bool {
	return !p.Role("Upload") && (p.MatchOs("Win") || p.MatchExtraConfig("Win") || p.MatchBazelHost("windows"))
}

// IsMac returns true if the task runs on Mac.
func (p Parts) IsMac() bool {
	return !p.Role("Upload") && (p.MatchOs("Mac") || p.MatchExtraConfig("Mac") || p.MatchBazelHost("darwin"))
}

// BazelBuildParts returns all parts from the BazelBuild schema. All parts are required.
func (p Parts) BazelBuildParts() (label string, config string, host string) {
	return p["label"], p["config"], p["host"]
}

// BazelTestParts returns all parts from the BazelTest schema. task_driver, label, build_config,
// and host are required; test_config is optional.
func (p Parts) BazelTestParts() (taskDriver string, label string, buildConfig string, host string, testConfig string) {
	return p["task_driver"], p["label"], p["build_config"], p["host"], p["test_config"]
}

// TODO(borenet): The below really belongs in its own file, probably next to the
// builder_name_schema.json file.

// schema is a sub-struct of JobNameSchema.
type schema struct {
	Keys         []string `json:"keys"`
	OptionalKeys []string `json:"optional_keys"`
	RecurseRoles []string `json:"recurse_roles"`
}

// JobNameSchema is a struct used for (de)constructing Job names in a
// predictable format.
type JobNameSchema struct {
	Schema map[string]*schema `json:"builder_name_schema"`
	Sep    string             `json:"builder_name_sep"`
}

// NewJobNameSchema returns a JobNameSchema instance based on the given JSON
// file.
func NewJobNameSchema(jsonFile string) (*JobNameSchema, error) {
	var rv JobNameSchema
	f, err := os.Open(jsonFile)
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := f.Close(); err != nil {
			log.Println(fmt.Sprintf("Failed to close %s: %s", jsonFile, err))
		}
	}()
	if err := json.NewDecoder(f).Decode(&rv); err != nil {
		return nil, err
	}
	return &rv, nil
}

// ParseJobName splits the given Job name into its component parts, according
// to the schema.
func (s *JobNameSchema) ParseJobName(n string) (map[string]string, error) {
	popFront := func(items []string) (string, []string, error) {
		if len(items) == 0 {
			return "", nil, fmt.Errorf("Invalid job name: %s (not enough parts)", n)
		}
		return items[0], items[1:], nil
	}

	result := map[string]string{}

	var parse func(int, string, []string) ([]string, error)
	parse = func(depth int, role string, parts []string) ([]string, error) {
		s, ok := s.Schema[role]
		if !ok {
			return nil, fmt.Errorf("Invalid job name; %q is not a valid role.", role)
		}
		if depth == 0 {
			result["role"] = role
		} else {
			result[fmt.Sprintf("sub-role-%d", depth)] = role
		}
		var err error
		for _, key := range s.Keys {
			var value string
			value, parts, err = popFront(parts)
			if err != nil {
				return nil, err
			}
			result[key] = value
		}
		for _, subRole := range s.RecurseRoles {
			if len(parts) > 0 && parts[0] == subRole {
				parts, err = parse(depth+1, parts[0], parts[1:])
				if err != nil {
					return nil, err
				}
			}
		}
		for _, key := range s.OptionalKeys {
			if len(parts) > 0 {
				var value string
				value, parts, err = popFront(parts)
				if err != nil {
					return nil, err
				}
				result[key] = value
			}
		}
		if len(parts) > 0 {
			return nil, fmt.Errorf("Invalid job name: %s (too many parts)", n)
		}
		return parts, nil
	}

	split := strings.Split(n, s.Sep)
	if len(split) < 2 {
		return nil, fmt.Errorf("Invalid job name: %s (not enough parts)", n)
	}
	role := split[0]
	split = split[1:]
	_, err := parse(0, role, split)
	return result, err
}

// MakeJobName assembles the given parts of a Job name, according to the schema.
func (s *JobNameSchema) MakeJobName(parts map[string]string) (string, error) {
	rvParts := make([]string, 0, len(parts))

	var process func(int, map[string]string) (map[string]string, error)
	process = func(depth int, parts map[string]string) (map[string]string, error) {
		roleKey := "role"
		if depth != 0 {
			roleKey = fmt.Sprintf("sub-role-%d", depth)
		}
		role, ok := parts[roleKey]
		if !ok {
			return nil, fmt.Errorf("Invalid job parts; missing key %q", roleKey)
		}

		s, ok := s.Schema[role]
		if !ok {
			return nil, fmt.Errorf("Invalid job parts; unknown role %q", role)
		}
		rvParts = append(rvParts, role)
		delete(parts, roleKey)

		for _, key := range s.Keys {
			value, ok := parts[key]
			if !ok {
				return nil, fmt.Errorf("Invalid job parts; missing %q", key)
			}
			rvParts = append(rvParts, value)
			delete(parts, key)
		}

		if len(s.RecurseRoles) > 0 {
			subRoleKey := fmt.Sprintf("sub-role-%d", depth+1)
			subRole, ok := parts[subRoleKey]
			if !ok {
				return nil, fmt.Errorf("Invalid job parts; missing %q", subRoleKey)
			}
			rvParts = append(rvParts, subRole)
			delete(parts, subRoleKey)
			found := false
			for _, recurseRole := range s.RecurseRoles {
				if recurseRole == subRole {
					found = true
					var err error
					parts, err = process(depth+1, parts)
					if err != nil {
						return nil, err
					}
					break
				}
			}
			if !found {
				return nil, fmt.Errorf("Invalid job parts; unknown sub-role %q", subRole)
			}
		}
		for _, key := range s.OptionalKeys {
			if value, ok := parts[key]; ok {
				rvParts = append(rvParts, value)
				delete(parts, key)
			}
		}
		if len(parts) > 0 {
			return nil, fmt.Errorf("Invalid job parts: too many parts: %v", parts)
		}
		return parts, nil
	}

	// Copy the parts map, so that we can modify at will.
	partsCpy := make(map[string]string, len(parts))
	for k, v := range parts {
		partsCpy[k] = v
	}
	if _, err := process(0, partsCpy); err != nil {
		return "", err
	}
	return strings.Join(rvParts, s.Sep), nil
}
