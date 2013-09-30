/*
 * Loader:
 * Reads GM result reports written out by results.py, and imports
 * them into $scope.categories and $scope.testData .
 */
var Loader = angular.module(
    'Loader',
    []
);
Loader.controller(
  'Loader.Controller',
  function($scope, $http) {
    $http.get("/results/all").then(
      function(response) {
        $scope.categories = response.data.categories;
        $scope.testData = response.data.testData;
        $scope.sortColumn = 'test';
	$scope.showResultsOfType = 'failed';
      }
    );
  }
);
