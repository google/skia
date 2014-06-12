/*
 * Constants used by our imagediff-viewing Javascript code.
 */
var module = angular.module(
    'ConstantsModule',
    []
);

module.constant('constants', (function() {
  return {
    // NOTE: Keep these in sync with ../column.py
    KEY__EXTRACOLUMNHEADERS__HEADER_TEXT: 'headerText',
    KEY__EXTRACOLUMNHEADERS__HEADER_URL: 'headerUrl',
    KEY__EXTRACOLUMNHEADERS__IS_FILTERABLE: 'isFilterable',
    KEY__EXTRACOLUMNHEADERS__IS_SORTABLE: 'isSortable',
    KEY__EXTRACOLUMNHEADERS__VALUES_AND_COUNTS: 'valuesAndCounts',

    // NOTE: Keep these in sync with ../imagediffdb.py
    KEY__DIFFERENCES__MAX_DIFF_PER_CHANNEL: 'maxDiffPerChannel',
    KEY__DIFFERENCES__NUM_DIFF_PIXELS: 'numDifferingPixels',
    KEY__DIFFERENCES__PERCENT_DIFF_PIXELS: 'percentDifferingPixels',
    KEY__DIFFERENCES__PERCEPTUAL_DIFF: 'perceptualDifference',

    // NOTE: Keep these in sync with ../imagepair.py
    KEY__IMAGEPAIRS__DIFFERENCES: 'differenceData',
    KEY__IMAGEPAIRS__EXPECTATIONS: 'expectations',
    KEY__IMAGEPAIRS__EXTRACOLUMNS: 'extraColumns',
    KEY__IMAGEPAIRS__IMAGE_A_URL: 'imageAUrl',
    KEY__IMAGEPAIRS__IMAGE_B_URL: 'imageBUrl',
    KEY__IMAGEPAIRS__IS_DIFFERENT: 'isDifferent',

    // NOTE: Keep these in sync with ../imagepairset.py
    KEY__ROOT__EXTRACOLUMNHEADERS: 'extraColumnHeaders',
    KEY__ROOT__HEADER: 'header',
    KEY__ROOT__IMAGEPAIRS: 'imagePairs',
    KEY__ROOT__IMAGESETS: 'imageSets',
    //
    KEY__IMAGESETS__FIELD__BASE_URL: 'baseUrl',
    KEY__IMAGESETS__FIELD__DESCRIPTION: 'description',
    KEY__IMAGESETS__SET__DIFFS: 'diffs',
    KEY__IMAGESETS__SET__IMAGE_A: 'imageA',
    KEY__IMAGESETS__SET__IMAGE_B: 'imageB',
    KEY__IMAGESETS__SET__WHITEDIFFS: 'whiteDiffs',

    // NOTE: Keep these in sync with ../results.py
    KEY__EXPECTATIONS__BUGS: 'bugs',
    KEY__EXPECTATIONS__IGNOREFAILURE: 'ignore-failure',
    KEY__EXPECTATIONS__REVIEWED: 'reviewed-by-human',
    //
    KEY__EXTRACOLUMNS__BUILDER: 'builder',
    KEY__EXTRACOLUMNS__CONFIG: 'config',
    KEY__EXTRACOLUMNS__RESULT_TYPE: 'resultType',
    KEY__EXTRACOLUMNS__TEST: 'test',
    //
    KEY__HEADER__DATAHASH: 'dataHash',
    KEY__HEADER__IS_EDITABLE: 'isEditable',
    KEY__HEADER__IS_EXPORTED: 'isExported',
    KEY__HEADER__IS_STILL_LOADING: 'resultsStillLoading',
    KEY__HEADER__RESULTS_ALL: 'all',
    KEY__HEADER__RESULTS_FAILURES: 'failures',
    KEY__HEADER__SCHEMA_VERSION: 'schemaVersion',
    KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE: 'timeNextUpdateAvailable',
    KEY__HEADER__TIME_UPDATED: 'timeUpdated',
    KEY__HEADER__TYPE: 'type',
    VALUE__HEADER__SCHEMA_VERSION: 3,
    //
    KEY__RESULT_TYPE__FAILED: 'failed',
    KEY__RESULT_TYPE__FAILUREIGNORED: 'failure-ignored',
    KEY__RESULT_TYPE__NOCOMPARISON: 'no-comparison',
    KEY__RESULT_TYPE__SUCCEEDED: 'succeeded',

    // NOTE: Keep these in sync with ../server.py
    KEY__EDITS__MODIFICATIONS: 'modifications',
    KEY__EDITS__OLD_RESULTS_HASH: 'oldResultsHash',
    KEY__EDITS__OLD_RESULTS_TYPE: 'oldResultsType',

    // These are just used on the client side, no need to sync with server code.
    KEY__IMAGEPAIRS__ROWSPAN: 'rowspan',
  }
})())
