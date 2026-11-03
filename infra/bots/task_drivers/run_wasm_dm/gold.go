// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/gold-client/go/auth"
	"go.skia.org/infra/gold-client/go/goldclient"
	"go.skia.org/infra/golden/go/jsonio"
	"go.skia.org/infra/golden/go/types"
	"go.skia.org/infra/task_driver/go/td"
)

type goldUploadProps struct {
	resultsDir     string
	goldResults    *jsonio.GoldResults
	digestOnly     []jsonio.Result
	imageAndDigest []jsonio.Result
}

type goldClient struct {
	ctx context.Context
	gc  *goldclient.CloudClient
}

type goldClientProps struct {
	DryRun  bool
	WorkDir string
}

func newGoldClient(ctx context.Context, props goldClientProps) (*goldClient, error) {
	ctx = td.StartStep(ctx, td.Props("set up gold client").Infra())
	defer td.EndStep(ctx)

	if err := os.MkdirAll(props.WorkDir, 0o755); err != nil {
		return nil, td.FailStep(ctx, err)
	}

	var err error
	if props.DryRun {
		err = auth.InitNoAuth(props.WorkDir)
	} else {
		err = auth.InitLUCIAuth(props.WorkDir)
	}
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	auth, err := auth.LoadAuthOpt(props.WorkDir)
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	auth.SetDryRun(props.DryRun)
	up, err := auth.GetGCSUploader(ctx)
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	hc, err := auth.GetHTTPClient()
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	dl, err := auth.GetImageDownloader()
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	errors := td.NewLogStream(ctx, "goldclient error", td.SeverityError)
	logs := td.NewLogStream(ctx, "goldclient info", td.SeverityInfo)
	ctx = context.WithValue(ctx, goldclient.ErrorWriterKey, errors)
	ctx = context.WithValue(ctx, goldclient.LogWriterKey, logs)

	gctx := goldclient.WithContext(ctx, up, hc, dl)
	gc, err := goldclient.NewCloudClient(goldclient.GoldClientConfig{
		WorkDir:         props.WorkDir,
		InstanceID:      "skia",
		OverrideGoldURL: "https://gold.skia.org",
		OverrideBucket:  "skia-infra-gm",
		UploadOnly:      true,
	})
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	return &goldClient{
		ctx: gctx,
		gc:  gc,
	}, nil
}

func (g *goldClient) Init(gr *jsonio.GoldResults, digestOnly []jsonio.Result) error {
	// This initializes the gold client with:
	// - shared keys
	// - commit/tryjob info
	// - results that reference existing digests (i.e. results do not have image
	//   data attached), whether preexisting on gold or by a test image that will
	//   be uploaded before the next call to Finalize.
	shared := *gr
	shared.Results = digestOnly
	shared.CodeReviewSystem = "gerrit"
	shared.ContinuousIntegrationSystem = "buildbucket"
	if err := g.gc.SetSharedConfig(g.ctx, shared, false); err != nil {
		return fmt.Errorf("gold client SetSharedConfig: %w", err)
	}

	return nil
}

func (g *goldClient) UploadImage(
	name types.TestName,
	pngPath string,
	digest types.Digest,
	additionalKeys map[string]string,
	optionalKeys map[string]string,
) error {
	_, err := g.gc.Test(g.ctx, name, pngPath, digest, additionalKeys, optionalKeys)
	return err
}

func (g *goldClient) Finalize() error {
	return g.gc.Finalize(g.ctx)
}

func gatherImagesForUpload(
	ctx context.Context,
	resultsDir string,
	knownHashes []byte,
) (*goldUploadProps, error) {
	ctx = td.StartStep(ctx, td.Props("gather images for upload").Infra())
	defer td.EndStep(ctx)

	// Read dm results file
	gr, err := loadDmJson(filepath.Join(resultsDir, "dm.json"))
	if err != nil {
		return nil, td.FailStep(ctx, err)
	}

	// Index the known hashes
	known := map[types.Digest]struct{}{}
	for _, digest := range strings.Fields(string(knownHashes)) {
		known[types.Digest(digest)] = struct{}{}
	}

	// Sort the results into those with new images (imageAndDigest) and those without (digestOnly)
	digestOnly := []jsonio.Result{}
	imageAndDigest := []jsonio.Result{}
	willUpload := map[types.Digest]struct{}{}
	for _, r := range gr.Results {
		_, isKnown := known[r.Digest]     // gold already has this image
		_, isSeen := willUpload[r.Digest] // another result will already upload
		if isKnown || isSeen {
			digestOnly = append(digestOnly, r)
		} else {
			imageAndDigest = append(imageAndDigest, r)
			willUpload[r.Digest] = struct{}{}
		}
	}

	td.StepText(ctx, "# new images", strconv.Itoa(len(imageAndDigest)))
	return &goldUploadProps{
		resultsDir:     resultsDir,
		goldResults:    gr,
		digestOnly:     digestOnly,
		imageAndDigest: imageAndDigest,
	}, nil
}

func uploadImages(ctx context.Context, props *goldUploadProps, dryRun bool, workDir string) error {
	ctx = td.StartStep(ctx, td.Props("upload images to gold").Infra())
	defer td.EndStep(ctx)

	gold, err := newGoldClient(ctx, goldClientProps{
		DryRun:  dryRun,
		WorkDir: workDir,
	})
	if err != nil {
		return td.FailStep(ctx, err)
	}

	if err := gold.Init(props.goldResults, props.digestOnly); err != nil {
		return td.FailStep(ctx, err)
	}

	if err := td.Do(ctx, td.Props("upload and finalize").Infra(), func(ctx context.Context) error {
		for _, r := range props.imageAndDigest {
			name := types.TestName(r.Key[types.PrimaryKeyField])
			pngPath := filepath.Join(props.resultsDir, string(r.Digest)+".png")
			if err := gold.UploadImage(name, pngPath, r.Digest, r.Key, r.Options); err != nil {
				return td.FailStep(ctx, skerr.Wrapf(err, "upload %s (%s)", name, string(r.Digest)))
			}
		}

		if err := gold.Finalize(); err != nil {
			return td.FailStep(ctx, err)
		}

		return nil
	}); err != nil {
		return td.FailStep(ctx, err)
	}

	td.StepText(ctx, "# new images uploaded", strconv.Itoa(len(props.imageAndDigest)))
	return nil
}
