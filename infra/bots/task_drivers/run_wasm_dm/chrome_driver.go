// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"errors"

	"github.com/chromedp/chromedp"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"
)

type chromeDriver struct {
	ctx      context.Context
	shutdown context.CancelFunc
}

func newChromeDriver(ctx context.Context, cacheDir string) (*chromeDriver, error) {
	ctx = td.StartStep(ctx, td.Props("set up chrome").Infra())
	defer td.EndStep(ctx)

	chromeBin, err := installChrome(ctx, cacheDir)
	if err != nil {
		return nil, skerr.Wrapf(err, "failed to install chrome")
	}

	ctx, shutdown := newChromeContext(ctx, chromeBin)
	return &chromeDriver{
		ctx:      ctx,
		shutdown: shutdown,
	}, nil
}

func (cd *chromeDriver) Shutdown() {
	cd.shutdown()
}

func newChromeContext(ctx context.Context, chromeBin string) (context.Context, context.CancelFunc) {
	stepCtx := td.StartStep(ctx, td.Props("create chrome devtools context").Infra())
	defer td.EndStep(stepCtx)

	execOpts := append(chromedp.DefaultExecAllocatorOptions[:],
		chromedp.ExecPath(chromeBin),
		chromedp.NoSandbox, // req'd for modern Ubuntus where we don't have sudo/root
	)
	chromeCtx, cancelChrome := chromedp.NewExecAllocator(ctx, execOpts...)

	opts := []chromedp.ContextOption{chromedp.WithErrorf(sklog.Errorf)}
	tabCtx, cancelTab := chromedp.NewContext(chromeCtx, opts...)

	// An empty Do starts the browser
	if err := chromedp.Do(tabCtx); err != nil {
		cancelTab()
		cancelChrome()
		td.Fatal(stepCtx, skerr.Wrapf(err, "failed to start chrome"))
	}

	cancelEcho := echoBrowserConsole(tabCtx)

	return tabCtx, func() {
		if err := chromedp.Cancel(tabCtx); err != nil {
			sklog.Warningf("chromedp cancel: %s", err)
		}
		cancelEcho()
		cancelTab()
		cancelChrome()
	}
}

func echoBrowserConsole(ctx context.Context) context.CancelFunc {
	ctx, cancel := context.WithCancel(ctx)
	console := chromedp.Console(ctx)
	done := make(chan struct{})
	go func() {
		defer close(done)
		// This loops over a stream, so won't terminate until canceled.
		for m, err := range console {
			if err != nil {
				if !errors.Is(err, context.Canceled) {
					sklog.Errorf("browser console error: %v", err)
				}
				return
			}

			switch m.Type {
			case chromedp.ConsoleWarning:
				sklog.Warningf("[js console]: %s", m.Text)
			case chromedp.ConsoleError:
				sklog.Errorf("[js console]: %s", m.Text)
			case chromedp.ConsoleException:
				sklog.Errorf("[js exception]: %s (%s:%d:%d)", m.Text, m.URL, m.Line, m.Column)
			default:
				sklog.Infof("[js console]: %s", m.Text)
			}
		}
	}()

	return func() {
		cancel()
		<-done // drain any last messages
	}
}
