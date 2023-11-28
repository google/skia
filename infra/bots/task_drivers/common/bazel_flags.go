// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"flag"
	"fmt"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/bazel/device_specific_configs"
)

// BazelFlags is a struct that holds common flags for task drivers that shell out to Bazel.
type BazelFlags struct {
	Label                *string
	Config               *string
	DeviceSpecificConfig *string
	AdditionalArgs       *[]string
	CacheDir             *string

	labelRequired                bool
	configRequired               bool
	deviceSpecificConfigRequired bool
}

// Validate performs common flag validation steps.
func (f *BazelFlags) Validate(ctx context.Context) {
	if f.labelRequired && *f.Label == "" {
		td.Fatal(ctx, fmt.Errorf("--bazel_label is required"))
	}
	if f.configRequired && *f.Config == "" {
		td.Fatal(ctx, fmt.Errorf("--bazel_config is required"))
	}
	if f.deviceSpecificConfigRequired {
		if *f.DeviceSpecificConfig == "" {
			td.Fatal(ctx, fmt.Errorf("--device_specific_bazel_config is required"))
		}
		if _, ok := device_specific_configs.Configs[*f.DeviceSpecificConfig]; !ok {
			td.Fatal(ctx, fmt.Errorf("unknown flag --device_specific_bazel_config value: %q", *f.DeviceSpecificConfig))
		}
	}
	if *f.CacheDir == "" {
		td.Fatal(ctx, fmt.Errorf("--bazel_cache_dir is required"))
	}
}

// MakeBazelFlagsOpts controls which flags are defiend by MakeBazelFlags.
type MakeBazelFlagsOpts struct {
	Label                bool
	Config               bool
	DeviceSpecificConfig bool
	AdditionalArgs       bool
}

// MakeBazelFlags declares common Bazel flags.
func MakeBazelFlags(opts MakeBazelFlagsOpts) *BazelFlags {
	bazelFlags := &BazelFlags{
		CacheDir: flag.String("bazel_cache_dir", "", "Path to the Bazel cache directory. This should be located on a large partition, as the cache can take tens of GB."),
	}
	if opts.Label {
		bazelFlags.Label = flag.String("bazel_label", "", "An fully qualified Bazel label to the target that should be built/tested/executed.")
		bazelFlags.labelRequired = true
	}
	if opts.Config {
		bazelFlags.Config = flag.String("bazel_config", "", "A custom Bazel configuration defined in //bazel/buildrc. This configuration potentially encapsulates many features and options.")
		bazelFlags.configRequired = true
	}
	if opts.DeviceSpecificConfig {
		bazelFlags.DeviceSpecificConfig = flag.String("device_specific_bazel_config", "", "A device-specific Bazel configuration defined in //bazel/devicesrc. This configuration determines various keys in any produced Gold/Perf traces, and has no effect in the built artifact.")
	}
	if opts.AdditionalArgs {
		bazelFlags.AdditionalArgs = common.NewMultiStringFlag("bazel_arg", nil, "Additional arguments that should be forwarded directly to the Bazel invocation.")
	}
	return bazelFlags
}
