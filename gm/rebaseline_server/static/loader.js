/*
 * Loader:
 * Reads GM result reports written out by results_loader.py, and imports
 * their data into $scope.results .
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
        $scope.results = response.data;
        $scope.sortColumn = 'test';
      }
    );
  }
);
