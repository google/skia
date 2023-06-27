// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable generates a comparison table in HTML format
// for all ICU implementations presented in a given root folder

package main

import (
	"errors"
	"flag"
	"fmt"
	"go.skia.org/skia/tools/unicode_comparison/go/helpers"
	"html/template"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

type Range struct {
	Start int
	End   int
	Type  string
}

// Main numeric type
type Ratio struct {
	Num   int
	Total int
}

type FloatRatio struct {
	Top    float64
	Bottom float64
}

func (r Ratio) Divide() string {
	if r.Num == 0 {
		return "        "
	} else {
		return fmt.Sprintf("%.6f", float64(r.Num)/float64(r.Total))
	}
}
func (fr FloatRatio) Percents() string {
	if fr.Top == fr.Bottom {
		return "        "
	} else {
		return fmt.Sprintf("%.6f", (float64(fr.Top)/float64(fr.Bottom)-1)*100)
	}
}

func (r *Ratio) Add(other Ratio) {
	if other.Num > 0 {
		r.Num += other.Num
		r.Total += other.Total
	}
}

type CalculatedDelta struct {
	Memory      float64
	Performance FloatRatio
	Disk        float64
	RowCount    int
	DiffCount   int
	Graphemes   Ratio
	SoftBreaks  Ratio
	HardBreaks  Ratio
	Whitespaces Ratio
	Words       Ratio
	Controls    Ratio
	Data        RangedData
}

func NewCalculatedDelta() CalculatedDelta {
	return CalculatedDelta{
		Performance: FloatRatio{0, 1},
		Graphemes:   Ratio{0, 1},
		SoftBreaks:  Ratio{0, 1},
		HardBreaks:  Ratio{0, 1},
		Whitespaces: Ratio{0, 1},
		Words:       Ratio{0, 1},
		Controls:    Ratio{0, 1},
	}
}

func (cd *CalculatedDelta) Add(other CalculatedDelta) {
	cd.Performance.Top += other.Performance.Top
	cd.Performance.Bottom += other.Performance.Bottom
	cd.Memory += other.Memory
	cd.Disk += other.Disk
	cd.RowCount += 1
	cd.DiffCount += other.DiffCount
	cd.Graphemes.Add(other.Graphemes)
	cd.SoftBreaks.Add(other.SoftBreaks)
	cd.HardBreaks.Add(other.HardBreaks)
	cd.Whitespaces.Add(other.Whitespaces)
	cd.Words.Add(other.Words)
	cd.Controls.Add(other.Controls)
}

type RangeDataSet struct {
	Graphemes   []Range
	SoftBreaks  []Range
	HardBreaks  []Range
	Whitespaces []Range
	Words       []Range
	Controls    []Range
}
type RangedData struct {
	Missing RangeDataSet
	Extra   RangeDataSet
}

type ParsedData struct {
	Count       int
	Time        float64
	Memory      float64
	Graphemes   []int
	SoftBreaks  []int
	HardBreaks  []int
	Whitespaces []int
	Words       []int
	Controls    []int
}

func NewParsedData() ParsedData {
	return ParsedData{}
}

// Row type
type Row struct {
	Id       string
	Num      string
	ParentId string
	Names    []string
	Text     string
	IsFile   bool
	Delta    CalculatedDelta
	Children []Row
}

func NewImpl(impl string) *Row {
	return &Row{Names: []string{impl}, IsFile: false, Delta: NewCalculatedDelta(), Children: nil}
}

func NewLocale(impl string, locale string) *Row {
	return &Row{Names: []string{impl, locale}, IsFile: false, Delta: NewCalculatedDelta(), Children: nil}
}

func NewSize(names ...string) *Row {
	return &Row{Names: names, IsFile: false, Delta: NewCalculatedDelta(), Children: nil}
}

func NewRow(text string, delta CalculatedDelta, names ...string) *Row {
	return &Row{Names: names, IsFile: true, Text: text, Delta: delta, Children: nil}
}

type Chunk struct {
	Text    string
	Classes string
	Indexes Range
}

func (row Row) FormattedChunks(ranges []Range, name string, chunkType string, includeRange bool) []Chunk {

	var results []Chunk
	gap := 0
	for i, r := range ranges {
		if i == 0 {
			continue
		}

		if r.Start > gap {
			text := row.Text[gap:r.Start]
			results = append(results, Chunk{text, "", Range{gap, r.Start, ""}})
		}
		if includeRange {
			text := row.Text[r.Start:r.End]
			if name == "whitespace" {
				corrected := ""
				for _, t := range text {
					if t == ' ' {
						corrected += "nbsp;"
					} else {
						corrected += string(t)
					}
				}
				text = corrected
			}
			results = append(results, Chunk{text, name, Range{r.Start, r.End, chunkType}})
			gap = r.End
		} else {
			results = append(results, Chunk{"\u200B", name, Range{r.Start, r.Start, chunkType}})
			gap = r.Start
		}
	}
	if gap < len(row.Text) {
		text := row.Text[gap:]
		results = append(results, Chunk{text, "", Range{gap, len(row.Text), ""}})
	}
	return results
}

func (r Row) FormattedMissingGraphemes() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.Graphemes, "grapheme", "missing", true)
}

func (r Row) FormattedExtraGraphemes() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.Graphemes, "grapheme", "extra", true)
}

func (r Row) FormattedMissingSoftBreaks() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.SoftBreaks, "softBreak", "missing", false)
}

func (r Row) FormattedExtraSoftBreaks() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.SoftBreaks, "softBreak", "extra", false)
}

func (r Row) FormattedMissingHardBreaks() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.HardBreaks, "hardBreak", "missing", false)
}

func (r Row) FormattedExtraHardBreaks() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.HardBreaks, "hardBreak", "extra", false)
}

func (r Row) FormattedMissingWords() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.Words, "word", "missing", false)
}

func (r Row) FormattedExtraWords() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.Words, "word", "extra", false)
}

func (r Row) FormattedMissingWhitespaces() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.Whitespaces, "whitespace", "missing", true)
}

func (r Row) FormattedExtraWhitespaces() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.Whitespaces, "whitespace", "extra", true)
}

func (r Row) FormattedMissingControls() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Missing.Controls, "control", "missing", false)
}

func (r Row) FormattedExtraControls() []Chunk {
	return r.FormattedChunks(r.Delta.Data.Extra.Controls, "control", "extra", false)
}

func (r *Row) Add(child Row) {
	r.Delta.Add(child.Delta)
	r.Children = append(r.Children, child)
}

func (r Row) Name() string {
	if r.IsFile {
		return r.Names[len(r.Names)-1]
	} else {
		names := ""
		for i, name := range r.Names {
			if i > 0 {
				names += "."
			}
			names += name
		}
		return names
	}
}

func (r Row) ParentName() string {
	names := ""
	for i, name := range r.Names {
		if i == len(r.Names)-1 {
			break
		}
		if i > 0 {
			names += "."
		}
		names += name
	}
	return names
}

func (r Row) Implementation() string {
	return r.Names[0]
}

func (r Row) Level() string {
	return fmt.Sprintf("l%d", len(r.Names))
}

func (r Row) HasText() bool {
	return len(r.Text) != 0
}

func (r Row) HasChildren() bool {
	return len(r.Children) > 0
}

func (r Row) IsImplementation() bool {
	return len(r.Names) == 1
}

func (r Row) HasNoDifferences() bool {
	return len(r.Delta.Data.Missing.Graphemes) == 0 &&
		len(r.Delta.Data.Missing.SoftBreaks) == 0 &&
		len(r.Delta.Data.Missing.HardBreaks) == 0 &&
		len(r.Delta.Data.Missing.Words) == 0 &&
		len(r.Delta.Data.Missing.Whitespaces) == 0 &&
		len(r.Delta.Data.Missing.Controls) == 0 &&
		len(r.Delta.Data.Extra.Graphemes) == 0 &&
		len(r.Delta.Data.Extra.SoftBreaks) == 0 &&
		len(r.Delta.Data.Extra.HardBreaks) == 0 &&
		len(r.Delta.Data.Extra.Words) == 0 &&
		len(r.Delta.Data.Extra.Whitespaces) == 0 &&
		len(r.Delta.Data.Extra.Controls) == 0
}

func (r Row) Differences() int {
	return len(r.Delta.Data.Missing.Graphemes) +
		len(r.Delta.Data.Missing.SoftBreaks) +
		len(r.Delta.Data.Missing.HardBreaks) +
		len(r.Delta.Data.Missing.Words) +
		len(r.Delta.Data.Missing.Whitespaces) +
		len(r.Delta.Data.Missing.Controls) +
		len(r.Delta.Data.Extra.Graphemes) +
		len(r.Delta.Data.Extra.SoftBreaks) +
		len(r.Delta.Data.Extra.HardBreaks) +
		len(r.Delta.Data.Extra.Words) +
		len(r.Delta.Data.Extra.Whitespaces) +
		len(r.Delta.Data.Extra.Controls)
}

func (r Row) HasMissingGraphemes() bool {
	return len(r.Delta.Data.Missing.Graphemes) > 0
}

func (r Row) HasExtraGraphemes() bool {
	return len(r.Delta.Data.Extra.Graphemes) > 0
}

func (r Row) HasNoGraphemes() bool {
	return !r.HasMissingGraphemes() && !r.HasExtraGraphemes()
}

func (r Row) HasMissingSoftBreaks() bool {
	return len(r.Delta.Data.Missing.SoftBreaks) > 0
}

func (r Row) HasExtraSoftBreaks() bool {
	return len(r.Delta.Data.Extra.SoftBreaks) > 0
}

func (r Row) HasNoSoftBreaks() bool {
	return !r.HasMissingSoftBreaks() && !r.HasExtraSoftBreaks()
}

func (r Row) HasMissingHardBreaks() bool {
	return len(r.Delta.Data.Missing.HardBreaks) > 0
}

func (r Row) HasExtraHardBreaks() bool {
	return len(r.Delta.Data.Extra.HardBreaks) > 0
}

func (r Row) HasNoHardBreaks() bool {
	return !r.HasMissingHardBreaks() && !r.HasExtraHardBreaks()
}

func (r Row) HasMissingWhitespaces() bool {
	return len(r.Delta.Data.Missing.Whitespaces) > 0
}

func (r Row) HasExtraWhitespaces() bool {
	return len(r.Delta.Data.Extra.Whitespaces) > 0
}

func (r Row) HasNoWhitespaces() bool {
	return !r.HasMissingWhitespaces() && !r.HasExtraWhitespaces()
}

func (r Row) HasMissingWords() bool {
	return len(r.Delta.Data.Missing.Words) > 0
}

func (r Row) HasExtraWords() bool {
	return len(r.Delta.Data.Extra.Words) > 0
}

func (r Row) HasNoWords() bool {
	return !r.HasMissingWords() && !r.HasExtraWords()
}

func (r Row) HasMissingControls() bool {
	return len(r.Delta.Data.Missing.Controls) > 0
}

func (r Row) HasExtraControls() bool {
	return len(r.Delta.Data.Extra.Controls) > 0
}
func (r Row) HasNoControls() bool {
	return !r.HasMissingControls() && !r.HasExtraControls()
}

func (r Row) MissingGraphemeNum() int {
	return len(r.Delta.Data.Missing.Graphemes) - 1
}

func (r Row) ExtraGraphemeNum() int {
	return len(r.Delta.Data.Extra.Graphemes) - 1
}

func (r Row) MissingSoftBreakNum() int {
	return len(r.Delta.Data.Missing.SoftBreaks) - 1
}

func (r Row) ExtraSoftBreakNum() int {
	return len(r.Delta.Data.Extra.SoftBreaks) - 1
}

func (r Row) MissingHardBreakNum() int {
	return len(r.Delta.Data.Missing.HardBreaks) - 1
}

func (r Row) ExtraHardBreakNum() int {
	return len(r.Delta.Data.Extra.HardBreaks) - 1
}

func (r Row) MissingWhitespaceNum() int {
	return len(r.Delta.Data.Missing.Whitespaces) - 1
}

func (r Row) ExtraWhitespaceNum() int {
	return len(r.Delta.Data.Extra.Whitespaces) - 1
}

func (r Row) MissingWordNum() int {
	return len(r.Delta.Data.Missing.Words) - 1
}

func (r Row) ExtraWordNum() int {
	return len(r.Delta.Data.Extra.Words) - 1
}

func (r Row) MissingControlNum() int {
	return len(r.Delta.Data.Missing.Controls) - 1
}

func (r Row) ExtraControlNum() int {
	return len(r.Delta.Data.Extra.Controls) - 1
}

type WebPage struct {
	Title   string
	Heading string
	Rows    []Row
}

func assignIDs(children []Row, parentId, parentNum string) {
	for i := range children {
		children[i].Num = fmt.Sprintf("%s_%d", parentNum, i+1)
		children[i].Id = fmt.Sprintf("%s_%d", parentId, i+1)
		children[i].ParentId = parentId
		assignIDs(children[i].Children, children[i].Id, children[i].Num)
	}
}

func addImpl(web *WebPage, impl Row) {
	impl.Num = fmt.Sprintf("%d", len(web.Rows)+1)
	impl.Id = fmt.Sprintf("id_%d", len(web.Rows)+1)
	impl.ParentId = ""
	assignIDs(impl.Children, impl.Id, impl.Num)
	web.Rows = append(web.Rows, impl)
}

func parseFile(path string, textLen int) (ParsedData, error) {
	var result ParsedData
	// Time: float64
	// Memory: float64
	// Graphemes: n1 n2 ...
	// SoftBreaks: n1 n2 ...
	// HardBreaks: n1 n2 ...
	// Whitespaces: n1 n2 ...
	// Words: n1 n2 ...
	// Controls: n1 n2 ...
	content, err := os.ReadFile(path)
	if err != nil {
		return result, err
	}

	lines := strings.Split(string(content), "\n")
	if len(lines) < 8 {
		return result, errors.New("Wrong data format (number of lines)")
	}

	result.Time, err = strconv.ParseFloat(lines[0], 64)
	if err != nil {
		return result, errors.New("Wrong data format (time)")
	}
	result.Memory, err = strconv.ParseFloat(lines[1], 64)
	if err != nil {
		return result, errors.New("Wrong data format (memory)")
	}

	result.Graphemes = helpers.SplitAsInts(lines[2]+" "+strconv.Itoa(textLen), " ")
	result.SoftBreaks = helpers.SplitAsInts(lines[3]+" "+strconv.Itoa(textLen), " ")
	result.HardBreaks = helpers.SplitAsInts(lines[4]+" "+strconv.Itoa(textLen), " ")
	result.Whitespaces = helpers.SplitAsInts(lines[5]+" "+strconv.Itoa(textLen), " ")
	result.Words = helpers.SplitAsInts(lines[6]+" "+strconv.Itoa(textLen), " ")
	result.Controls = helpers.SplitAsInts(lines[7]+" "+strconv.Itoa(textLen), " ")

	return result, nil
}

func compareLines(expected []int, actual []int, includeRange bool, missing bool) (Ratio, []Range) {

	var diff []Range
	diff = append(diff, Range{len(actual), len(expected), ""})
	aLen := len(actual) - 1
	eLen := len(expected) - 1

	e := 1
	a := 1
	for e < eLen || a < aLen {
		a1 := actual[a]
		if includeRange && a < aLen {
			a1 = actual[a+1]
		}
		e1 := expected[e]
		if includeRange && e < eLen {
			e1 = expected[e+1]
		}

		if e >= eLen {
			if !missing {
				diff = append(diff, Range{helpers.Abs(actual[a]), helpers.Abs(a1), "extra"})
			}
			a += 1
		} else if a >= aLen {
			if missing {
				diff = append(diff, Range{helpers.Abs(expected[e]), helpers.Abs(e1), "missing"})
			}
			e += 1
		} else if actual[a] < expected[e] {
			if !missing {
				diff = append(diff, Range{helpers.Abs(actual[a]), helpers.Abs(a1), "extra"})
			}
			a += 1
		} else if actual[a] > expected[e] {
			if missing {
				diff = append(diff, Range{helpers.Abs(expected[e]), helpers.Abs(e1), "missing"})
			}
			e += 1
		} else {
			a += 1
			e += 1
		}
	}

	// TODO: keep the difference, too
	if len(diff) > 1 {
		return Ratio{len(diff) - 1, len(expected)}, diff
	} else {
		return Ratio{0, 1}, nil
	}
}

func compareData(expected ParsedData, actual ParsedData) CalculatedDelta {

	var delta CalculatedDelta

	delta.Performance.Top = actual.Time
	delta.Performance.Bottom = expected.Time

	var deltaGraphemes, deltaSoftBreaks, deltaHardBreaks, deltaWhitespaces, deltaWords, deltaControls Ratio
	deltaGraphemes, delta.Data.Missing.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true, true)

	deltaGraphemes, delta.Data.Missing.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true, true)
	deltaSoftBreaks, delta.Data.Missing.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks, false, true)
	deltaHardBreaks, delta.Data.Missing.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks, false, true)
	deltaWhitespaces, delta.Data.Missing.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces, true, true)
	deltaWords, delta.Data.Missing.Words = compareLines(expected.Words, actual.Words, true, true)
	deltaControls, delta.Data.Missing.Controls = compareLines(expected.Controls, actual.Controls, false, true)

	delta.Graphemes.Add(deltaGraphemes)
	delta.SoftBreaks.Add(deltaSoftBreaks)
	delta.HardBreaks.Add(deltaHardBreaks)
	delta.Whitespaces.Add(deltaWhitespaces)
	delta.Words.Add(deltaWords)
	delta.Controls.Add(deltaControls)

	deltaGraphemes, delta.Data.Extra.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true, false)
	deltaSoftBreaks, delta.Data.Extra.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks, false, false)
	deltaHardBreaks, delta.Data.Extra.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks, false, false)
	deltaWhitespaces, delta.Data.Extra.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces, true, false)
	deltaWords, delta.Data.Extra.Words = compareLines(expected.Words, actual.Words, true, false)
	deltaControls, delta.Data.Extra.Controls = compareLines(expected.Controls, actual.Controls, false, false)

	delta.Graphemes.Add(deltaGraphemes)
	delta.SoftBreaks.Add(deltaSoftBreaks)
	delta.HardBreaks.Add(deltaHardBreaks)
	delta.Whitespaces.Add(deltaWhitespaces)
	delta.Words.Add(deltaWords)
	delta.Controls.Add(deltaControls)

	return delta
}

func printDifference(text string, diff []int) {
	count := diff[0]

	if len(diff) <= 1 {
		// No diff
	} else if (len(diff)-1)*10 < count {
		// Too small diff
		fmt.Printf("%d < %d:\n%s\n", (len(diff)-1)*10, count, text)
		return
	} else if count == 0 {
		// Too small string
		fmt.Printf("%d == 0:\n%s\n", count, text)
		return
	}
	first := helpers.Abs(diff[1])
	last := first + 10
	if last >= len(text) {
		last = len(text) - 1
	}
	fmt.Printf("Difference @%d:\n%s\n", first, text[:last])
}

func finishRows(rows []Row, start int) []Row {
	if len(rows) == 0 {
		return []Row{}
	}
	i := len(rows) - 1
	for i > start {
		(rows)[i-1].Add((rows)[i])
		i -= 1
	}

	if start > 0 {
		return rows[:start]
	} else {
		return rows[:start+1]
	}
}

func findParentRow(rows []Row, name string) int {
	for i := range rows {
		row := rows[len(rows)-1-i]
		if row.Names[len(row.Names)-1] == name {
			return len(rows) - 1 - i
		}
	}
	return -1
}

func compareFiles(inputPath string, sampleLimit int) (WebPage, error) {

	var rows []Row

	// Define the data to be used in the template
	web := WebPage{
		Title: "Comparison Table (accuracy, performance and disk memory)",
	}

	err := filepath.Walk(inputPath,
		func(inputFile string, info os.FileInfo, err error) error {
			if err != nil {
				fmt.Println(err)
				return err
			}
			tokens := strings.Split(inputFile, string(os.PathSeparator))
			outputIndex := -1
			for i, t := range tokens {
				if t == "output" {
					outputIndex = i
					break
				}
			}
			if outputIndex < 0 {
				return fmt.Errorf("Currently only supported directory structure: [...]/output/{implementation}/{locale}:\n%s\n", inputFile)
			}

			if info.IsDir() {
				if len(tokens) == outputIndex+1 {
					// ~/datasets/output
				} else if len(tokens) == outputIndex+2 {
					// ~/datasets/output/icu
					rows = finishRows(rows, 0)
					if len(rows) > 0 {
						addImpl(&web, rows[0])
					}
					rows = []Row{*NewImpl(tokens[outputIndex+1])}
				} else if len(tokens) == outputIndex+3 {
					// ~/datasets/output/icu/en
					rows = finishRows(rows, 0)
					rows = append(rows, *NewLocale(rows[0].Names[0], tokens[outputIndex+2]))
				} else {
					fmt.Printf("skipping %s\n", inputFile)
					return nil
				}
			} else if len(rows) <= 1 {
				return errors.New(fmt.Sprintf("Wrong directory structure: %s\n", inputFile))
			} else {
				// Find the parent row
				parent := &rows[len(rows)-1]
				impl := parent.Names[0]
				// Read and parse the data
				textFile := strings.Replace(inputFile, filepath.Join("output", impl), "input", 1)
				textContent, err := os.ReadFile(textFile)
				helpers.Check(err)
				if len(textContent) == 0 {
					fmt.Printf("Empty text file %s\n", inputFile)
					return nil
				}

				var actualData ParsedData
				actualData, err = parseFile(inputFile, len(textContent))
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse output file %s: %s\n", inputFile, err.Error()))
				}

				var validationData ParsedData
				validationFile := strings.Replace(inputFile, filepath.Join("output", impl), "validation", 1)
				validationData, err = parseFile(validationFile, len(textContent))
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse validation file%s: %s\n", validationFile, err.Error()))
				}

				// Compare the data
				var delta CalculatedDelta
				_, shortFileName := filepath.Split(inputFile)
				delta = compareData(validationData, actualData)
				row := NewRow(string(textContent), delta, append(parent.Names, shortFileName)...)
				if !row.HasNoDifferences() {
					parent.Add(*row)
					sort.Slice(parent.Children, func(i, j int) bool {
						return parent.Children[i].Differences() > parent.Children[j].Differences()
					})
					if len(parent.Children) > sampleLimit {
						parent.Children = parent.Children[0 : sampleLimit-1]
					}
				}
			}
			return nil
		})
	rows = finishRows(rows, 0)
	if len(rows) > 0 {
		addImpl(&web, rows[0])
	}
	return web, err
}

func main() {
	var (
		root        = flag.String("root", "~/datasets", "Folder (inputs for the table expected to be under <Folder>/output/>")
		sampleLimit = flag.Int("sampleLimit", 10, "Number of files to show with differences")
	)
	flag.Parse()
	if *root == "" {
		fmt.Println("Must set --root")
		flag.PrintDefaults()
	}
	*root = helpers.ExpandPath(*root)

	// Parse the template
	t, err := template.ParseFiles("../html/index.html", "../html/scripts.html", "../html/styles.html", "../html/tbody.html")
	helpers.Check(err)

	// Create index.html
	indexPath := filepath.Join(*root, "index.html")
	indexFile, err := os.Create(indexPath)
	helpers.Check(err)

	outputPath := filepath.Join(*root, "output")
	web, err := compareFiles(outputPath, *sampleLimit)
	helpers.Check(err)

	// Execute the template and write the result to index.html
	err = t.Execute(indexFile, web)
	helpers.Check(err)
	indexFile.Close()
}
