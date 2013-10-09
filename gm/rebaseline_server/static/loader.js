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
        if (!(true == hiddenResultTypes[item.resultType]) &&
            !(true == hiddenConfigs[item.config])) {
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
        $scope.header = response.data.header;
        $scope.categories = response.data.categories;
        $scope.testData = response.data.testData;
        $scope.sortColumn = 'test';

        $scope.hiddenResultTypes = {
          'failure-ignored': true,
          'no-comparison': true,
          'succeeded': true,
        };
        $scope.hiddenConfigs = {};
        $scope.selectedItems = {};

        $scope.updateResults();
      }
    );

    $scope.isItemSelected = function(index) {
      return (true == $scope.selectedItems[index]);
    }
    $scope.toggleItemSelected = function(index) {
      if (true == $scope.selectedItems[index]) {
        delete $scope.selectedItems[index];
      } else {
        $scope.selectedItems[index] = true;
      }
      // unlike other toggle methods below, does not set
      // $scope.areUpdatesPending = true;
    }

    $scope.isHiddenResultType = function(thisResultType) {
      return (true == $scope.hiddenResultTypes[thisResultType]);
    }
    $scope.toggleHiddenResultType = function(thisResultType) {
      if (true == $scope.hiddenResultTypes[thisResultType]) {
        delete $scope.hiddenResultTypes[thisResultType];
      } else {
        $scope.hiddenResultTypes[thisResultType] = true;
      }
      $scope.areUpdatesPending = true;
    }

    // TODO(epoger): Rather than maintaining these as hard-coded
    // variants of isHiddenResultType and toggleHiddenResultType, we
    // should create general-purpose functions that can work with ANY
    // category.
    // But for now, I wanted to see this working. :-)
    $scope.isHiddenConfig = function(thisConfig) {
      return (true == $scope.hiddenConfigs[thisConfig]);
    }
    $scope.toggleHiddenConfig = function(thisConfig) {
      if (true == $scope.hiddenConfigs[thisConfig]) {
        delete $scope.hiddenConfigs[thisConfig];
      } else {
        $scope.hiddenConfigs[thisConfig] = true;
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
