/**
 * Command line application to process the output of
 *   make skps_release_and_SIMD
 * and present some statistical results in a human-readable table format.
 */

const fs = require('fs');

// These files are the output of `make skps_release_and_SIMD`
const SIMD_DATA = JSON.parse(fs.readFileSync('simd_out.json', 'utf8'));
const RELEASE_DATA = JSON.parse(fs.readFileSync('release_out.json', 'utf8'));

let skp_names = new Set();
for (const key of Object.keys(SIMD_DATA)) {
    skp_names.add(key)
}
for (const key of Object.keys(RELEASE_DATA)) {
    skp_names.add(key)
}

let simd_frame_average_accumulator = 0;
let simd_frame_median_accumulator = 0;
let release_frame_average_accumulator = 0;
let release_frame_median_accumulator = 0;
const comparisonData = [];

for (const skp_name of skp_names) {
    if (SIMD_DATA[skp_name] && RELEASE_DATA[skp_name]) {
        // note: frames are frametimes, measured in ms
        const simd_frames = SIMD_DATA[skp_name].total_frame_ms;
        const simd_frames_average = averageFromArray(simd_frames);
        const simd_frames_median = medianFromArray(simd_frames);
        simd_frame_average_accumulator += simd_frames_average;
        simd_frame_median_accumulator += simd_frames_median;

        const release_frames = RELEASE_DATA[skp_name].total_frame_ms;
        const release_frames_average = averageFromArray(release_frames);
        const release_frames_median = medianFromArray(release_frames);
        release_frame_average_accumulator += release_frames_average;
        release_frame_median_accumulator += release_frames_median;

        comparisonData.push({
            skp_name: skp_name,
            frames_average_difference: release_frames_average - simd_frames_average,
            frames_median_difference: release_frames_median - simd_frames_median,
            simd_frames_median: simd_frames_median,
            simd_frames_average: simd_frames_average,
            release_frames_average: release_frames_average,
            release_frames_median: release_frames_median
        });
    }
}
const simd_average_frame = simd_frame_average_accumulator / comparisonData.length;
const simd_skps_median_frame = simd_frame_median_accumulator / comparisonData.length;
const release_average_frame = release_frame_average_accumulator / comparisonData.length;
const release_median_frame = release_frame_median_accumulator / comparisonData.length;

console.log('\nAverages across all SKP files');
console.table({
    'Average frame time average': {
        'release CanvasKit build (ms)': release_average_frame.toFixed(2),
        'experimental_simd CanvasKit build (ms)': simd_average_frame.toFixed(2),
        'difference (ms)': (release_average_frame - simd_average_frame).toFixed(2)
    },
    'Median frame time average': {
        'release CanvasKit build (ms)': release_median_frame.toFixed(2),
        'experimental_simd CanvasKit build (ms)': simd_skps_median_frame.toFixed(2),
        'difference (ms)': (release_median_frame - simd_skps_median_frame).toFixed(2)
    }
});

const frameTimeMedianDifferenceSorted =
    comparisonData.sort(
        ({frames_median_difference: m1}, {frames_median_difference: m2}) => m2 - m1
    );

console.log('\nBest 3 Individual SKP frame time median differences in favor of the SIMD build');
console.table(
    frameTimeMedianDifferenceSorted
    .map(tableDataFromComparisonDataObject)
    .slice(0,3)
);

console.log('\nWorst 3 Individual SKP frame time median differences NOT in favor of the SIMD build');
console.table(
    frameTimeMedianDifferenceSorted
    .map(tableDataFromComparisonDataObject)
    .reverse().slice(0,3)
);

function averageFromArray(array) {
    return array.reduce((a, b) => a+b, 0) / array.length;
}
function medianFromArray(array) {
    return array.sort((a,b) => a-b)[Math.floor(array.length/2)];
}

function tableDataFromComparisonDataObject({
    skp_name,
    frames_median_difference,
    simd_frames_median,
    release_frames_median
}) {
    return {
        '.SKP name': skp_name,
        'release CanvasKit build (ms)': release_frames_median.toFixed(2),
        'experimental_simd CanvasKit build (ms)': simd_frames_median.toFixed(2),
        'difference (ms)': frames_median_difference.toFixed(2)
    };
}