/*
 * GMActualResultsLoader:
 * Reads an actual-results.json file, and imports its data into $scope.
 */
var GMActualResultsLoader = angular.module(
    'GMActualResultsLoader',
    [],
    function($httpProvider) {
      /* Override transformResponse so that the numeric checksums are interpreted as
       * strings instead, since Javascript cannot handle 64-bit integers. */
      $httpProvider.defaults.transformResponse = function(data, headersGetter) {
        return JSON.parse(data.replace(/\s(\d+)\s/g, " \"$1\" "));
      }
    }
);
GMActualResultsLoader.controller(
    'GMActualResultsLoader.Controller',
    function($scope, $http) {
      /* When the changePlatformPath function is called, download actual-results.json
       * from the desired platform directory.
       *
       * When the JSON is received, predigest it (combining actual and expected results for each
       * test) and return it to the frontend as $scope.gmActualResults like so:
       *
       * [
       *   {"resultType": "failed",
       *    "resultsOfThisType": [
       *      {"test":"bigmatrix", "config":"gpu",
       *       "actualHashType": "bitmap-64bitMD5", "actualHashValue": "1234",
       *       "expectedHashType": "bitmap-64bitMD5", "actualHashValue": "6789"},
       *      {"test":"bigmatrix", "config":"8888",
       *       "actualHashType": "bitmap-64bitMD5", "actualHashValue": "5678",
       *       "expectedHashType": "bitmap-64bitMD5", "actualHashValue": "6789"},
       *      more tests...
       *    ]},
       *   {"resultType": "no-comparison",
       *    "resultsOfThisType": [
       *      {"test":"aaclip", "config":"gpu",
       *       "actualHashType": "bitmap-64bitMD5", "actualHashValue": "8765"},
       *      {"test":"aaclip", "config":"8888",
       *       "actualHashType": "bitmap-64bitMD5", "actualHashValue": "4321"},
       *      more tests...
       *    ]},
       *   more result types...
       * ]
       */
      $scope.changePlatformPath = function() {
        $http.get($scope.platformPath + "/actual-results.json").success(
            function(response) {
              var jsonResults = [];
              var imageNameRegex = /^(.+)_([^_]+).png/;
              angular.forEach(response['actual-results'], function(resultsOfThisType, resultType) {
                var resultCollection = [];
                angular.forEach(resultsOfThisType, function(hashTypeAndValue, imageName) {
                  var matched = imageNameRegex.exec(imageName);
                  var thisResult = {
                    test: matched[1], config: matched[2],
                    actualHashType: hashTypeAndValue[0], actualHashValue: hashTypeAndValue[1] };
                  var expectations = response['expected-results'][imageName]['allowed-digests'];
                  if (expectations != null) {
                    thisResult.expectedHashType = expectations[0][0];
                    thisResult.expectedHashValue = expectations[0][1];
                  }
                  resultCollection.push(thisResult);
                });
                var resultTypeAndCollection = { resultType: resultType,
                                                resultsOfThisType: resultCollection };
                jsonResults.push(resultTypeAndCollection);
              });
              $scope.gmActualResults = jsonResults;
            }
         );
      };
    }
);
