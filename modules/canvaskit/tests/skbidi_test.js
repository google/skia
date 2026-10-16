/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Dual-mode test runner for standalone skbidi WASM target (works in Karma and Node.js)
(function () {
    const isNode = typeof process !== 'undefined' &&
                   process.versions &&
                   process.versions.node &&
                   typeof describe === 'undefined';

    function runTests(SkBidi, expectFn) {
        // Test 1: Empty text handling
        {
            const regions = SkBidi.Bidi.getBidiRegions('', SkBidi.TextDirection.LTR);
            expectFn(regions.length, 0, 'empty text produces no regions');

            const reordered = SkBidi.Bidi.reorderVisual([]);
            expectFn(reordered.length, 0, 'empty runs produces empty reorder');

            const flags = SkBidi.CodeUnits.compute('');
            expectFn(flags.length, 0, 'empty text produces no code unit flags');
        }

        // Test 2: Pure LTR text
        {
            const ltrText = 'Hello World';
            const regions = SkBidi.Bidi.getBidiRegions(ltrText, SkBidi.TextDirection.LTR);
            expectFn(regions.length, 1, 'pure LTR text has 1 region');
            expectFn(regions[0].start, 0, 'LTR region start is 0');
            expectFn(regions[0].end, ltrText.length, 'LTR region end matches length');
            expectFn(regions[0].level, 0, 'LTR region level is 0');
        }

        // Test 3: Mixed LTR and RTL text
        {
            const sampleText =
                'left1 يَهْدِيْكُمُ left2 اللَّه left3 ُ ' +
                'وَيُصْلِحُ left4 بَالَكُم left5 يَهْدِيْكُم';
            const regions = SkBidi.Bidi.getBidiRegions(sampleText, SkBidi.TextDirection.LTR);
            expectFn(regions.length, 10, 'mixed text has 10 regions');
            expectFn(regions[regions.length - 1].end, sampleText.length,
                     'last region ends at text length');

            for (let i = 0; i < 5; i++) {
                expectFn(regions[i * 2].level, 0, 'even region level is 0 (LTR)');
                expectFn(regions[i * 2 + 1].level, 1, 'odd region level is 1 (RTL)');
            }
        }

        // Test 4: Visual reordering
        {
            const emptyReorder = SkBidi.Bidi.reorderVisual([]);
            expectFn(emptyReorder.length, 0, 'empty reorder length is 0');

            const singleRun = SkBidi.Bidi.reorderVisual([0]);
            expectFn(singleRun.length, 1, 'single run reorder length is 1');
            expectFn(singleRun[0].index, 0, 'single run index is 0');

            const alternating = SkBidi.Bidi.reorderVisual([0, 1, 0, 1]);
            const altIndices = alternating.map((r) => r.index);
            expectFn(altIndices.join(','), '0,1,2,3', 'alternating levels order');

            // Two adjacent RTL runs inside LTR context: [0, 1, 1, 0] visually flips runs 1 and 2
            const nestedRtl = SkBidi.Bidi.reorderVisual([0, 1, 1, 0]);
            const nestedIndices = nestedRtl.map((r) => r.index);
            expectFn(nestedIndices.join(','), '0,2,1,3', 'nested RTL runs visually flip');

            // Levels above UBIDI_MAX_EXPLICIT_LEVEL + 1 (126) fail gracefully without reading
            // uninitialized memory
            const invalidLevel = SkBidi.Bidi.reorderVisual([127]);
            expectFn(invalidLevel.length, 0, 'invalid bidi level > 126 returns empty array');
        }

        // Test 4b: Positions exceeding 16-bit range (> 65535 UTF-16 code units)
        {
            const longText = 'a'.repeat(70000);
            const regions = SkBidi.Bidi.getBidiRegions(longText, SkBidi.TextDirection.LTR);
            expectFn(regions.length, 1, 'long LTR text has 1 region');
            expectFn(regions[0].start, 0, 'long LTR region start is 0');
            expectFn(regions[0].end, 70000, 'long LTR region end is not truncated to 16 bits');
        }

        // Test 5: Code unit flags computation
        {
            const flagsText = '   |\u{a0}\u{a0}\u{a0}|\u{0a}\u{0a}\u{0a}|満毎行';
            const flags = SkBidi.CodeUnits.compute(flagsText);
            expectFn(flags.length, flagsText.length, 'flags count matches UTF-16 length');

            const kPartOfWhiteSpaceBreak = 0x01;
            const kPartOfIntraWordBreak = 0x10;
            const kControl = 0x20;
            const kIdeographic = 0x100;

            // Space characters: both whitespace and intra-word break
            for (let i = 0; i < 3; i++) {
                const f = flags[i].flags;
                expectFn((f & kPartOfWhiteSpaceBreak) !== 0, true, 'space has whitespace flag');
                expectFn((f & kPartOfIntraWordBreak) !== 0, true, 'space has intra-word flag');
            }

            // Non-breaking space: intra-word break but not whitespace break
            for (let i = 4; i < 7; i++) {
                const f = flags[i].flags;
                expectFn((f & kPartOfWhiteSpaceBreak) === 0, true, 'nbsp lacks whitespace break');
                expectFn((f & kPartOfIntraWordBreak) !== 0, true, 'nbsp has intra-word break');
            }

            // Newlines: whitespace break and control flag
            for (let i = 8; i < 11; i++) {
                const f = flags[i].flags;
                expectFn((f & kControl) !== 0, true, 'newline has control flag');
                expectFn((f & kPartOfWhiteSpaceBreak) !== 0, true, 'newline has whitespace break');
            }

            // Ideographic characters
            for (let i = 12; i < 15; i++) {
                const f = flags[i].flags;
                expectFn((f & kIdeographic) !== 0, true, 'cjk character has ideographic flag');
            }
        }
    }

    if (isNode) {
        const path = require('path');
        const fs = require('fs');

        const wasmDir = process.argv[2] || 'out/skbidi_wasm';
        const jsPath = path.resolve(wasmDir, 'skbidi.js');
        const wasmPath = path.resolve(wasmDir, 'skbidi.wasm');

        if (!fs.existsSync(jsPath) || !fs.existsSync(wasmPath)) {
            console.error('skbidi.js or skbidi.wasm not found in ' + wasmDir);
            process.exit(1);
        }

        const SkBidiInit = require(jsPath);
        SkBidiInit({
            locateFile: (file) => path.join(wasmDir, file),
        }).then((SkBidi) => {
            let passCount = 0;
            function nodeExpect(actual, expected, desc) {
                if (actual !== expected) {
                    throw new Error('FAIL: ' + desc + ' (expected ' + expected + ', got ' +
                                    actual + ')');
                }
                passCount++;
            }

            runTests(SkBidi, nodeExpect);
            console.log('All ' + passCount + ' skbidi assertions passed successfully');
        }).catch((err) => {
            console.error(err);
            process.exit(1);
        });
    } else {
        describe('SkBidi Standalone WASM', function () {
            let skBidiInstance = null;

            beforeAll(async () => {
                if (typeof SkBidiInit === 'function') {
                    skBidiInstance = await SkBidiInit({
                        locateFile: (file) => '/build/' + file,
                    });
                }
            });

            it('executes standalone skbidi test suite', () => {
                if (!skBidiInstance || !skBidiInstance.Bidi) {
                    console.warn('Skipping SkBidi test because skbidi wasm is not available');
                    return;
                }

                function karmaExpect(actual, expected, desc) {
                    expect(actual).withContext(desc).toEqual(expected);
                }

                runTests(skBidiInstance, karmaExpect);
            });
        });
    }
})();
