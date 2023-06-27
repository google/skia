// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package helpers

import (
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func Check(e error) {
	if e != nil {
		panic(e)
	}
}

func Abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

func SplitAsInts(str string, sep string) []int {
	arr := strings.Split(str, sep)
	var result []int
	for _, i := range arr {
		j, err := strconv.Atoi(i)
		if err != nil {
			return nil
		}
		result = append(result, j)
	}
	return result
}

func ExpandPath(path string) string {
	if strings.HasPrefix(path, "~/") {
		home, err := os.UserHomeDir()
		Check(err)
		return filepath.Join(home, (path)[2:])
	}
	return path
}

func WriteTextFile(fullFileName string, text string) {
	err := os.WriteFile(fullFileName, []byte(text), 0666)
	Check(err)
}
