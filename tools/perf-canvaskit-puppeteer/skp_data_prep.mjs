import { readFileSync } from 'fs';

const SIMD_DATA = JSON.parse(readFileSync('simd_out.json', 'utf8'));
const RELEASE_DATA = JSON.parse(readFileSync('release_out.json', 'utf8'));

let SKP_Set = new Set();
for (const key of Object.keys(SIMD_DATA)) {
    SKP_Set.add(key)
}
for (const key of Object.keys(RELEASE_DATA)) {
    SKP_Set.add(key)
}

const SUMMARY_DATA =  {};
for (const key of SKP_Set) {
    if (SIMD_DATA[key] && RELEASE_DATA[key]) {
        SUMMARY_DATA[key] = {
            total_frame_ms_average_vs: RELEASE_DATA[key].total_frame_ms_average - SIMD_DATA[key].total_frame_ms_average,
            total_frame_ms_median_vs: RELEASE_DATA[key].total_frame_ms_median - SIMD_DATA[key].total_frame_ms_median
        }
    }
}

let averagevs_average = 0;
let medianvs_average = 0;
for (const key of Object.keys(SUMMARY_DATA)) {
    averagevs_average += SUMMARY_DATA[key].total_frame_ms_average_vs;
    medianvs_average += SUMMARY_DATA[key].total_frame_ms_median_vs;
}
averagevs_average = averagevs_average / Object.keys(SUMMARY_DATA).length;
medianvs_average = medianvs_average / Object.keys(SUMMARY_DATA).length;

console.log('averagevs_average', averagevs_average);
console.log('medianvs_average', medianvs_average);