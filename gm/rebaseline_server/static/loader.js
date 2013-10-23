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
    return function(unfilteredItems, hiddenResultTypes, hiddenConfigs,
                    viewingTab) {
      var filteredItems = [];
      for (var i = 0; i < unfilteredItems.length; i++) {
        var item = unfilteredItems[i];
        if (!(true == hiddenResultTypes[item.resultType]) &&
            !(true == hiddenConfigs[item.config]) &&
            (viewingTab == item.tab)) {
          filteredItems.push(item);
        }
      }
      return filteredItems;
    };
  }
);

Loader.controller(
  'Loader.Controller',
    function($scope, $http, $filter, $location) {
    $scope.windowTitle = "Loading GM Results...";
    var resultsToLoad = $location.search().resultsToLoad;
    $scope.loadingMessage = "Loading results of type '" + resultsToLoad +
        "', please wait...";

    $http.get("/results/" + resultsToLoad).success(
      function(data, status, header, config) {
        $scope.loadingMessage = "Processing data, please wait...";

        $scope.header = data.header;
        $scope.categories = data.categories;
        $scope.testData = data.testData;
        $scope.sortColumn = 'test';
        $scope.showTodos = false;

        // Create the list of tabs (lists into which the user can file each
        // test).  This may vary, depending on isEditable.
        $scope.tabs = [
          'Unfiled', 'Hidden'
        ];
        if (data.header.isEditable) {
          $scope.tabs = $scope.tabs.concat(
              ['Pending Approval']);
        }
        $scope.defaultTab = $scope.tabs[0];
        $scope.viewingTab = $scope.defaultTab;

        // Track the number of results on each tab.
        $scope.numResultsPerTab = {};
        for (var i = 0; i < $scope.tabs.length; i++) {
          $scope.numResultsPerTab[$scope.tabs[i]] = 0;
        }
        $scope.numResultsPerTab[$scope.defaultTab] = $scope.testData.length;

        // Add index and tab fields to all records.
        for (var i = 0; i < $scope.testData.length; i++) {
          $scope.testData[i].index = i;
          $scope.testData[i].tab = $scope.defaultTab;
        }

        $scope.hiddenResultTypes = {
          'failure-ignored': true,
          'no-comparison': true,
          'succeeded': true,
        };
        $scope.hiddenConfigs = {};
        $scope.selectedItems = [];

        $scope.updateResults();
        $scope.loadingMessage = "";
        $scope.windowTitle = "Current GM Results";
      }
    ).error(
      function(data, status, header, config) {
        $scope.loadingMessage = "Failed to load results of type '"
            + resultsToLoad + "'";
        $scope.windowTitle = "Failed to Load GM Results";
      }
    );

    $scope.isItemSelected = function(index) {
      return (-1 != $scope.selectedItems.indexOf(index));
    }
    $scope.toggleItemSelected = function(index) {
      var i = $scope.selectedItems.indexOf(index);
      if (-1 == i) {
        $scope.selectedItems.push(index);
      } else {
        $scope.selectedItems.splice(i, 1);
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

    $scope.setViewingTab = function(tab) {
      $scope.viewingTab = tab;
      $scope.updateResults();
    }

    $scope.localTimeString = function(secondsPastEpoch) {
      var d = new Date(secondsPastEpoch * 1000);
      return d.toString();
    }

    /**
     * Move the items in $scope.selectedItems to a different tab,
     * and then clear $scope.selectedItems.
     *
     * @param newTab (string): name of the tab to move the tests to
     */
    $scope.moveSelectedItemsToTab = function(newTab) {
      $scope.moveItemsToTab($scope.selectedItems, newTab);
      $scope.selectedItems = [];
      $scope.updateResults();
    }

    /**
     * Move a subset of $scope.testData to a different tab.
     *
     * @param itemIndices (array of ints): indices into $scope.testData
     *        indicating which test results to move
     * @param newTab (string): name of the tab to move the tests to
     */
    $scope.moveItemsToTab = function(itemIndices, newTab) {
      var itemIndex;
      var numItems = itemIndices.length;
      for (var i = 0; i < numItems; i++) {
        itemIndex = itemIndices[i];
        $scope.numResultsPerTab[$scope.testData[itemIndex].tab]--;
        $scope.testData[itemIndex].tab = newTab;
      }
      $scope.numResultsPerTab[newTab] += numItems;
    }

    $scope.updateResults = function() {
      $scope.displayLimit = $scope.displayLimitPending;
      // TODO(epoger): Every time we apply a filter, AngularJS creates
      // another copy of the array.  Is there a way we can filter out
      // the items as they are displayed, rather than storing multiple
      // array copies?  (For better performance.)

      if ($scope.viewingTab == $scope.defaultTab) {
        $scope.filteredTestData =
            $filter("orderBy")(
                $filter("removeHiddenItems")(
                    $scope.testData,
                    $scope.hiddenResultTypes,
                    $scope.hiddenConfigs,
                    $scope.viewingTab
                ),
                $scope.sortColumn);
        $scope.limitedTestData = $filter("limitTo")(
            $scope.filteredTestData, $scope.displayLimit);
      } else {
        $scope.filteredTestData =
            $filter("orderBy")(
                $filter("filter")(
                    $scope.testData,
                    {tab: $scope.viewingTab},
                    true
                ),
                $scope.sortColumn);
        $scope.limitedTestData = $filter("limitTo")(
            $scope.filteredTestData, $scope.displayLimit);
      }
      $scope.imageSize = $scope.imageSizePending;
      $scope.areUpdatesPending = false;
    }

    $scope.sortResultsBy = function(sortColumn) {
      $scope.sortColumn = sortColumn;
      $scope.updateResults();
    }

    /**
     * Tell the server that the actual results of these particular tests
     * are acceptable.
     *
     * @param testDataSubset an array of test results, most likely a subset of
     *        $scope.testData (perhaps with some modifications)
     */
    $scope.submitApprovals = function(testDataSubset) {
      $scope.submitPending = true;
      var newResults = [];
      for (var i = 0; i < testDataSubset.length; i++) {
        var actualResult = testDataSubset[i];
        var expectedResult = {
          builder: actualResult['builder'],
          test: actualResult['test'],
          config: actualResult['config'],
          expectedHashType: actualResult['actualHashType'],
          expectedHashDigest: actualResult['actualHashDigest'],
        };
        newResults.push(expectedResult);
      }
      $http({
        method: "POST",
        url: "/edits",
        data: {
          oldResultsType: $scope.header.type,
          oldResultsHash: $scope.header.dataHash,
          modifications: newResults
        }
      }).success(function(data, status, headers, config) {
        var itemIndicesToMove = [];
        for (var i = 0; i < testDataSubset.length; i++) {
          itemIndicesToMove.push(testDataSubset[i].index);
        }
        $scope.moveItemsToTab(itemIndicesToMove,
                              "HackToMakeSureThisItemDisappears");
        $scope.updateResults();
        alert("New baselines submitted successfully!\n\n" +
            "You still need to commit the updated expectations files on " +
            "the server side to the Skia repo.\n\n" +
            "Also: in order to see the complete updated data, or to submit " +
            "more baselines, you will need to reload your client.");
        $scope.submitPending = false;
      }).error(function(data, status, headers, config) {
        alert("There was an error submitting your baselines.\n\n" +
            "Please see server-side log for details.");
        $scope.submitPending = false;
      });
    }
  }
);
