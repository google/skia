/**
 * Command line application to process the output of
 *   make skps_release_and_SIMD
 * and present some statistical results in a human-readable table format.
 */

const fs = require('fs');

// These files are the output of `make skps_release_and_SIMD`
const SIMD_DATA = JSON.parse(fs.readFileSync('simd_out.json', 'utf8'));
const RELEASE_DATA = JSON.parse(fs.readFileSync('release_out.json', 'utf8'));

let SKP_Set = new Set();
for (const key of Object.keys(SIMD_DATA)) {
    SKP_Set.add(key)
}
for (const key of Object.keys(RELEASE_DATA)) {
    SKP_Set.add(key)
}

let simd_total_frame_ms_average = 0;
let simd_total_frame_ms_median = 0;
let release_total_frame_ms_average = 0;
let release_total_frame_ms_median = 0;
const difference = {};
let commonSKPDataCount = 0;
for (const key of SKP_Set) {
    if (SIMD_DATA[key] && RELEASE_DATA[key]) {
        simd_total_frame_ms_average += SIMD_DATA[key].total_frame_ms_average;
        simd_total_frame_ms_median += SIMD_DATA[key].total_frame_ms_median;
        release_total_frame_ms_average += RELEASE_DATA[key].total_frame_ms_average;
        release_total_frame_ms_median += RELEASE_DATA[key].total_frame_ms_median;

        difference[key] = {
            total_frame_ms_average: RELEASE_DATA[key].total_frame_ms_average - SIMD_DATA[key].total_frame_ms_average,
            total_frame_ms_median: RELEASE_DATA[key].total_frame_ms_median - SIMD_DATA[key].total_frame_ms_median
        };

        commonSKPDataCount++;
    }
}

console.log('\nAverages across all SKP files');
console.table({
    'Average frame time average': {
        'release CanvasKit build': release_total_frame_ms_average / commonSKPDataCount,
        'experimental_SIMD CanvasKit build': simd_total_frame_ms_average / commonSKPDataCount,
        'difference': (release_total_frame_ms_average - simd_total_frame_ms_average) / commonSKPDataCount
    },
    'Median frame time average': {
        'release CanvasKit build': release_total_frame_ms_median / commonSKPDataCount,
        'experimental_SIMD CanvasKit build': simd_total_frame_ms_median / commonSKPDataCount,
        'difference': (release_total_frame_ms_median - simd_total_frame_ms_median) / commonSKPDataCount
    }
});

const frameTimeMedianDifferenceSorted =
    Object.entries(difference).sort(
        ([key1, value1], [key2, value2]) => value2.total_frame_ms_median - value1.total_frame_ms_median
    );

console.log('\nTop 3 Individual SKP frame time median differences in favor of the SIMD build');
console.table(frameTimeMedianDifferenceSorted.map(([key, { total_frame_ms_median }]) => ({
    'SKP name': key,
    'release CanvasKit build': RELEASE_DATA[key].total_frame_ms_median,
    'experimental_SIMD CanvasKit build': SIMD_DATA[key].total_frame_ms_median,
    'difference': total_frame_ms_median
})).slice(0,3));

console.log('\nTop 3 Individual SKP frame time median differences NOT in favor of the SIMD build');
console.table(frameTimeMedianDifferenceSorted.map(([key, { total_frame_ms_median }]) => ({
    'SKP name': key,
    'release CanvasKit build': RELEASE_DATA[key].total_frame_ms_median,
    'experimental_SIMD CanvasKit build': SIMD_DATA[key].total_frame_ms_median,
    'difference': total_frame_ms_median
})).reverse().slice(0,3));