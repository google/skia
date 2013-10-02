/*
 * Loader:
 * Reads GM result reports written out by results.py, and imports
 * them into $scope.categories and $scope.testData .
 */
var Loader = angular.module(
    'Loader',
    []
);

// TODO(epoger): Combine ALL of our filtering operations (including
// truncation) into this one filter, so that runs most efficiently?
// (We would have to make sure truncation still took place after
// sorting, though.)
Loader.filter(
  'removeHiddenItems',
  function() {
    return function(unfilteredItems, hiddenResultTypes, hiddenConfigs) {
      var filteredItems = [];
      for (var i = 0; i < unfilteredItems.length; i++) {
	var item = unfilteredItems[i];
	if ((hiddenResultTypes.indexOf(item.resultType) < 0) &&
	    (hiddenConfigs.indexOf(item.config) < 0)) {
	  filteredItems.push(item);
	}
      }
      return filteredItems;
    };
  }
);

Loader.controller(
  'Loader.Controller',
  function($scope, $http, $filter) {
    $http.get("/results/all").then(
      function(response) {
        $scope.categories = response.data.categories;
        $scope.testData = response.data.testData;
        $scope.sortColumn = 'test';

        $scope.hiddenResultTypes = [
          'failure-ignored', 'no-comparison', 'succeeded'];
        $scope.hiddenConfigs = [];

        $scope.updateResults();
      }
    );

    $scope.isHiddenResultType = function(thisResultType) {
      return ($scope.hiddenResultTypes.indexOf(thisResultType) >= 0);
    }
    $scope.toggleHiddenResultType = function(thisResultType) {
      var i = $scope.hiddenResultTypes.indexOf(thisResultType);
      if (i >= 0) {
	$scope.hiddenResultTypes.splice(i, 1);
      } else {
	$scope.hiddenResultTypes.push(thisResultType);
      }
      $scope.areUpdatesPending = true;
    }

    // TODO(epoger): Rather than maintaining these as hard-coded
    // variants of isHiddenResultType and toggleHiddenResultType, we
    // should create general-purpose functions that can work with ANY
    // category.
    // But for now, I wanted to see this working. :-)
    $scope.isHiddenConfig = function(thisConfig) {
      return ($scope.hiddenConfigs.indexOf(thisConfig) >= 0);
    }
    $scope.toggleHiddenConfig = function(thisConfig) {
      var i = $scope.hiddenConfigs.indexOf(thisConfig);
      if (i >= 0) {
	$scope.hiddenConfigs.splice(i, 1);
      } else {
	$scope.hiddenConfigs.push(thisConfig);
      }
      $scope.areUpdatesPending = true;
    }

    $scope.updateResults = function() {
      $scope.displayLimit = $scope.displayLimitPending;
      // TODO(epoger): Every time we apply a filter, AngularJS creates
      // another copy of the array.  Is there a way we can filter out
      // the items as they are displayed, rather than storing multiple
      // array copies?  (For better performance.)
      $scope.filteredTestData =
	  $filter("orderBy")(
	      $filter("removeHiddenItems")(
		  $scope.testData,
		  $scope.hiddenResultTypes,
		  $scope.hiddenConfigs
	      ),
	      $scope.sortColumn);
      $scope.limitedTestData = $filter("limitTo")(
	  $scope.filteredTestData, $scope.displayLimit);
      $scope.imageSize = $scope.imageSizePending;
      $scope.areUpdatesPending = false;
    }

    $scope.sortResultsBy = function(sortColumn) {
      $scope.sortColumn = sortColumn;
      $scope.updateResults();
    }
  }
);
