/*
 * GMExpectedResultsLoader:
 * Reads an expected-results.json file, and imports its data into $scope.
 */
var GMExpectedResultsLoader = angular.module(
    'GMExpectedResultsLoader',
    [],
    function($httpProvider) {
      /* Override transformResponse so that the numeric checksums are interpreted as
       * strings instead, since Javascript cannot handle 64-bit integers. */
      $httpProvider.defaults.transformResponse = function(data, headersGetter) {
        return JSON.parse(data.replace(/\s(\d+)\s/g, " \"$1\" "));
      }
    }
);
GMExpectedResultsLoader.controller(
    'GMExpectedResultsLoader.Controller',
    function($scope, $http) {
      /* When the changePlatformPath function is called, download expected-results.json
       * from the desired platform directory.
       *
       * When the JSON is received, predigest it and return it to the frontend as
       * $scope.gmExpectedResults .
       */
      $scope.changePlatformPath = function() {
        $http.get($scope.platformPath + "/expected-results.json").success(
            function(response) {
              var jsonResults = [];
              var imageNameRegex = /^(.+)_([^_]+).png/;
              angular.forEach(response['expected-results'], function(imageExpectations, imageName) {
                var matched = imageNameRegex.exec(imageName);
		var allowedImages = [];
		angular.forEach(imageExpectations['allowed-digests'], function(allowedDigest, key) {
		  var thisImage = {
		    hashType: allowedDigest[0], hashValue: allowedDigest[1]
		  };
		  allowedImages.push(thisImage);
		});
                var thisResult = {
                  test: matched[1], config: matched[2],
		  allowedImages: allowedImages,
		  bugs: imageExpectations['bugs'],
		  reviewedByHuman: imageExpectations['reviewed-by-human']
		};
                jsonResults.push(thisResult);
              });
              $scope.gmExpectedResults = jsonResults;
            }
         );
      };
    }
);
