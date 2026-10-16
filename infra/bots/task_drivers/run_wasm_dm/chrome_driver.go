// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"errors"
	"io"

	"github.com/chromedp/chromedp"
	"github.com/jcgregorio/logger"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"
)

type chromeDriver struct {
	ctx      context.Context
	shutdown context.CancelFunc
}

func newChromeDriver(
	ctx context.Context,
	cacheDir string,
	verboseLogs bool,
) (*chromeDriver, error) {
	ctx = td.StartStep(ctx, td.Props("set up chrome").Infra())
	defer td.EndStep(ctx)

	chromeBin, err := installChrome(ctx, cacheDir)
	if err != nil {
		return nil, skerr.Wrapf(err, "failed to install chrome")
	}

	ctx, shutdown := newChromeContext(ctx, chromeBin, verboseLogs)
	return &chromeDriver{
		ctx:      ctx,
		shutdown: shutdown,
	}, nil
}

func (cd *chromeDriver) Shutdown() {
	cd.shutdown()
}

func (cd *chromeDriver) AttachConsole(w io.Writer) context.CancelFunc {
	return attachConsole(cd.ctx, newLogger(w))
}

func newChromeContext(
	ctx context.Context,
	chromeBin string,
	verboseLogs bool,
) (context.Context, context.CancelFunc) {
	stepCtx := td.StartStep(ctx, td.Props("create chrome devtools context").Infra())
	defer td.EndStep(stepCtx)

	execOpts := append(chromedp.DefaultExecAllocatorOptions[:],
		chromedp.ExecPath(chromeBin),
		chromedp.NoSandbox, // req'd for modern Ubuntus where we don't have sudo/root
	)
	chromeCtx, cancelChrome := chromedp.NewExecAllocator(ctx, execOpts...)

	opts := []chromedp.ContextOption{chromedp.WithErrorf(sklog.Errorf)}
	tabCtx, cancelTab := chromedp.NewContext(chromeCtx, opts...)

	cancelEcho := func() {}
	if verboseLogs {
		// Log stream in parent context so it survives beyond this step.
		logger := newLogger(td.NewLogStream(ctx, "chrome", td.SeverityInfo))
		cancelEcho = attachConsole(tabCtx, logger)
	}

	// An empty Do starts the browser
	if err := chromedp.Do(tabCtx); err != nil {
		cancelTab()
		cancelChrome()
		td.Fatal(stepCtx, skerr.Wrapf(err, "failed to start chrome"))
	}

	return tabCtx, func() {
		if err := chromedp.Cancel(tabCtx); err != nil {
			sklog.Warningf("chromedp cancel: %s", err)
		}
		cancelEcho()
		cancelTab()
		cancelChrome()
	}
}

func attachConsole(ctx context.Context, logger *logger.Logger) context.CancelFunc {
	ctx, cancel := context.WithCancel(ctx)
	console := chromedp.Console(ctx)
	done := make(chan struct{})
	go func() {
		defer close(done)
		// This loops over a stream, so won't terminate until canceled.
		for m, err := range console {
			if err != nil {
				if !errors.Is(err, context.Canceled) {
					logger.Errorf("browser console error: %v", err)
				}
				return
			}

			switch m.Type {
			case chromedp.ConsoleWarning:
				logger.Warningf("[js console]: %s", m.Text)
			case chromedp.ConsoleError:
				logger.Errorf("[js console]: %s", m.Text)
			case chromedp.ConsoleException:
				logger.Errorf("[js exception]: %s (%s:%d:%d)", m.Text, m.URL, m.Line, m.Column)
			default:
				logger.Infof("[js console]: %s", m.Text)
			}
		}
	}()

	return func() {
		cancel()
		<-done // drain any last messages
	}
}
