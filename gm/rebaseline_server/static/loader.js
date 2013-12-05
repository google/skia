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
                    builderSubstring, testSubstring, viewingTab) {
      var filteredItems = [];
      for (var i = 0; i < unfilteredItems.length; i++) {
        var item = unfilteredItems[i];
        // For performance, we examine the "set" objects directly rather
        // than calling $scope.isValueInSet().
        // Besides, I don't think we have access to $scope in here...
        if (!(true == hiddenResultTypes[item.resultType]) &&
            !(true == hiddenConfigs[item.config]) &&
            !(-1 == item.builder.indexOf(builderSubstring)) &&
            !(-1 == item.test.indexOf(testSubstring)) &&
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
    function($scope, $http, $filter, $location, $timeout) {
    $scope.windowTitle = "Loading GM Results...";
    $scope.resultsToLoad = $location.search().resultsToLoad;
    $scope.loadingMessage = "Loading results of type '" + $scope.resultsToLoad +
        "', please wait...";

    /**
     * On initial page load, load a full dictionary of results.
     * Once the dictionary is loaded, unhide the page elements so they can
     * render the data.
     */
    $http.get("/results/" + $scope.resultsToLoad).success(
      function(data, status, header, config) {
        if (data.header.resultsStillLoading) {
          $scope.loadingMessage =
              "Server is still loading initial results; will retry at " +
              $scope.localTimeString(data.header.timeNextUpdateAvailable);
          $timeout(
              function(){location.reload();},
              (data.header.timeNextUpdateAvailable * 1000) - new Date().getTime());
        } else {
          $scope.loadingMessage = "Processing data, please wait...";

          $scope.header = data.header;
          $scope.categories = data.categories;
          $scope.testData = data.testData;
          $scope.sortColumn = 'weightedDiffMeasure';
          $scope.showTodos = false;

          $scope.showSubmitAdvancedSettings = false;
          $scope.submitAdvancedSettings = {};
          $scope.submitAdvancedSettings['reviewed-by-human'] = true;
          $scope.submitAdvancedSettings['ignore-failure'] = false;
          $scope.submitAdvancedSettings['bug'] = '';

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

          // Arrays within which the user can toggle individual elements.
          $scope.selectedItems = [];

          // Sets within which the user can toggle individual elements.
          $scope.hiddenResultTypes = {
            'failure-ignored': true,
            'no-comparison': true,
            'succeeded': true,
          };
          $scope.allResultTypes = Object.keys(data.categories['resultType']);
          $scope.hiddenConfigs = {};
          $scope.allConfigs = Object.keys(data.categories['config']);

          // Associative array of partial string matches per category.
          $scope.categoryValueMatch = {};
          $scope.categoryValueMatch.builder = "";
          $scope.categoryValueMatch.test = "";

          // If any defaults were overridden in the URL, get them now.
          $scope.queryParameters.load();

          $scope.updateResults();
          $scope.loadingMessage = "";
          $scope.windowTitle = "Current GM Results";
        }
      }
    ).error(
      function(data, status, header, config) {
        $scope.loadingMessage = "Failed to load results of type '"
            + $scope.resultsToLoad + "'";
        $scope.windowTitle = "Failed to Load GM Results";
      }
    );


    //
    // Select/Clear/Toggle all tests.
    //

    /**
     * Select all currently showing tests.
     */
    $scope.selectAllItems = function() {
      var numItemsShowing = $scope.limitedTestData.length;
      for (var i = 0; i < numItemsShowing; i++) {
        var index = $scope.limitedTestData[i].index;
        if (!$scope.isValueInArray(index, $scope.selectedItems)) {
          $scope.toggleValueInArray(index, $scope.selectedItems);
        }
      }
    }

    /**
     * Deselect all currently showing tests.
     */
    $scope.clearAllItems = function() {
      var numItemsShowing = $scope.limitedTestData.length;
      for (var i = 0; i < numItemsShowing; i++) {
        var index = $scope.limitedTestData[i].index;
        if ($scope.isValueInArray(index, $scope.selectedItems)) {
          $scope.toggleValueInArray(index, $scope.selectedItems);
        }
      }
    }

    /**
     * Toggle selection of all currently showing tests.
     */
    $scope.toggleAllItems = function() {
      var numItemsShowing = $scope.limitedTestData.length;
      for (var i = 0; i < numItemsShowing; i++) {
        var index = $scope.limitedTestData[i].index;
        $scope.toggleValueInArray(index, $scope.selectedItems);
      }
    }


    //
    // Tab operations.
    //

    /**
     * Change the selected tab.
     *
     * @param tab (string): name of the tab to select
     */
    $scope.setViewingTab = function(tab) {
      $scope.viewingTab = tab;
      $scope.updateResults();
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


    //
    // $scope.queryParameters:
    // Transfer parameter values between $scope and the URL query string.
    //
    $scope.queryParameters = {};

    // load and save functions for parameters of each type
    // (load a parameter value into $scope from nameValuePairs,
    //  save a parameter value from $scope into nameValuePairs)
    $scope.queryParameters.copiers = {
      'simple': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            $scope[name] = value;
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = $scope[name];
        }
      },

      'categoryValueMatch': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            $scope.categoryValueMatch[name] = value;
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = $scope.categoryValueMatch[name];
        }
      },

      'set': {
        'load': function(nameValuePairs, name) {
          var value = nameValuePairs[name];
          if (value) {
            var valueArray = value.split(',');
            $scope[name] = {};
            $scope.toggleValuesInSet(valueArray, $scope[name]);
          }
        },
        'save': function(nameValuePairs, name) {
          nameValuePairs[name] = Object.keys($scope[name]).join(',');
        }
      },

    };

    // parameter name -> copier objects to load/save parameter value
    $scope.queryParameters.map = {
      'resultsToLoad':       $scope.queryParameters.copiers.simple,
      'displayLimitPending': $scope.queryParameters.copiers.simple,
      'imageSizePending':    $scope.queryParameters.copiers.simple,
      'sortColumn':          $scope.queryParameters.copiers.simple,

      'builder': $scope.queryParameters.copiers.categoryValueMatch,
      'test':    $scope.queryParameters.copiers.categoryValueMatch,

      'hiddenResultTypes': $scope.queryParameters.copiers.set,
      'hiddenConfigs':     $scope.queryParameters.copiers.set,
    };

    // Loads all parameters into $scope from the URL query string;
    // any which are not found within the URL will keep their current value.
    $scope.queryParameters.load = function() {
      var nameValuePairs = $location.search();
      angular.forEach($scope.queryParameters.map,
                      function(copier, paramName) {
                        copier.load(nameValuePairs, paramName);
                      }
                     );
    };

    // Saves all parameters from $scope into the URL query string.
    $scope.queryParameters.save = function() {
      var nameValuePairs = {};
      angular.forEach($scope.queryParameters.map,
                      function(copier, paramName) {
                        copier.save(nameValuePairs, paramName);
                      }
                     );
      $location.search(nameValuePairs);
    };


    //
    // updateResults() and friends.
    //

    /**
     * Set $scope.areUpdatesPending (to enable/disable the Update Results
     * button).
     *
     * TODO(epoger): We could reduce the amount of code by just setting the
     * variable directly (from, e.g., a button's ng-click handler).  But when
     * I tried that, the HTML elements depending on the variable did not get
     * updated.
     * It turns out that this is due to variable scoping within an ng-repeat
     * element; see http://stackoverflow.com/questions/15388344/behavior-of-assignment-expression-invoked-by-ng-click-within-ng-repeat
     *
     * @param val boolean value to set $scope.areUpdatesPending to
     */
    $scope.setUpdatesPending = function(val) {
      $scope.areUpdatesPending = val;
    }

    /**
     * Update the displayed results, based on filters/settings,
     * and call $scope.queryParameters.save() so that the new filter results
     * can be bookmarked.
     */
    $scope.updateResults = function() {
      $scope.displayLimit = $scope.displayLimitPending;
      // TODO(epoger): Every time we apply a filter, AngularJS creates
      // another copy of the array.  Is there a way we can filter out
      // the items as they are displayed, rather than storing multiple
      // array copies?  (For better performance.)

      if ($scope.viewingTab == $scope.defaultTab) {

        // TODO(epoger): Until we allow the user to reverse sort order,
        // there are certain columns we want to sort in a different order.
        var doReverse = (
            ($scope.sortColumn == 'percentDifferingPixels') ||
            ($scope.sortColumn == 'weightedDiffMeasure'));

        $scope.filteredTestData =
            $filter("orderBy")(
                $filter("removeHiddenItems")(
                    $scope.testData,
                    $scope.hiddenResultTypes,
                    $scope.hiddenConfigs,
                    $scope.categoryValueMatch.builder,
                    $scope.categoryValueMatch.test,
                    $scope.viewingTab
                ),
                $scope.sortColumn, doReverse);
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
        $scope.limitedTestData = $scope.filteredTestData;
      }
      $scope.imageSize = $scope.imageSizePending;
      $scope.setUpdatesPending(false);
      $scope.queryParameters.save();
    }

    /**
     * Re-sort the displayed results.
     *
     * @param sortColumn (string): name of the column to sort on
     */
    $scope.sortResultsBy = function(sortColumn) {
      $scope.sortColumn = sortColumn;
      $scope.updateResults();
    }

    /**
     * Set $scope.categoryValueMatch[name] = value, and update results.
     *
     * @param name
     * @param value
     */
    $scope.setCategoryValueMatch = function(name, value) {
      $scope.categoryValueMatch[name] = value;
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenResultTypes so that ONLY this resultType is showing,
     * and update the visible results.
     *
     * @param resultType
     */
    $scope.showOnlyResultType = function(resultType) {
      $scope.hiddenResultTypes = {};
      // TODO(epoger): Maybe change $scope.allResultTypes to be a Set like
      // $scope.hiddenResultTypes (rather than an array), so this operation is
      // simpler (just assign or add allResultTypes to hiddenResultTypes).
      $scope.toggleValuesInSet($scope.allResultTypes, $scope.hiddenResultTypes);
      $scope.toggleValueInSet(resultType, $scope.hiddenResultTypes);
      $scope.updateResults();
    }

    /**
     * Update $scope.hiddenConfigs so that ONLY this config is showing,
     * and update the visible results.
     *
     * @param config
     */
    $scope.showOnlyConfig = function(config) {
      $scope.hiddenConfigs = {};
      $scope.toggleValuesInSet($scope.allConfigs, $scope.hiddenConfigs);
      $scope.toggleValueInSet(config, $scope.hiddenConfigs);
      $scope.updateResults();
    }


    //
    // Operations for sending info back to the server.
    //

    /**
     * Tell the server that the actual results of these particular tests
     * are acceptable.
     *
     * @param testDataSubset an array of test results, most likely a subset of
     *        $scope.testData (perhaps with some modifications)
     */
    $scope.submitApprovals = function(testDataSubset) {
      $scope.submitPending = true;

      // Convert bug text field to null or 1-item array.
      var bugs = null;
      var bugNumber = parseInt($scope.submitAdvancedSettings['bug']);
      if (!isNaN(bugNumber)) {
        bugs = [bugNumber];
      }

      // TODO(epoger): This is a suboptimal way to prevent users from
      // rebaselining failures in alternative renderModes, but it does work.
      // For a better solution, see
      // https://code.google.com/p/skia/issues/detail?id=1748 ('gm: add new
      // result type, RenderModeMismatch')
      var encounteredComparisonConfig = false;

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
        if (0 == expectedResult.config.indexOf('comparison-')) {
          encounteredComparisonConfig = true;
        }

        // Advanced settings...
        expectedResult['reviewed-by-human'] =
            $scope.submitAdvancedSettings['reviewed-by-human'];
        if (true == $scope.submitAdvancedSettings['ignore-failure']) {
          // if it's false, don't send it at all (just keep the default)
          expectedResult['ignore-failure'] = true;
        }
        expectedResult['bugs'] = bugs;

        newResults.push(expectedResult);
      }
      if (encounteredComparisonConfig) {
        alert("Approval failed -- you cannot approve results with config " +
            "type comparison-*");
        $scope.submitPending = false;
        return;
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


    //
    // Operations we use to mimic Set semantics, in such a way that
    // checking for presence within the Set is as fast as possible.
    // But getting a list of all values within the Set is not necessarily
    // possible.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns true if value "value" is present within set "set".
     *
     * @param value a value of any type
     * @param set an Object which we use to mimic set semantics
     *        (this should make isValueInSet faster than if we used an Array)
     */
    $scope.isValueInSet = function(value, set) {
      return (true == set[value]);
    }

    /**
     * If value "value" is already in set "set", remove it; otherwise, add it.
     *
     * @param value a value of any type
     * @param set an Object which we use to mimic set semantics
     */
    $scope.toggleValueInSet = function(value, set) {
      if (true == set[value]) {
        delete set[value];
      } else {
        set[value] = true;
      }
    }

    /**
     * For each value in valueArray, call toggleValueInSet(value, set).
     *
     * @param valueArray
     * @param set
     */
    $scope.toggleValuesInSet = function(valueArray, set) {
      var arrayLength = valueArray.length;
      for (var i = 0; i < arrayLength; i++) {
        $scope.toggleValueInSet(valueArray[i], set);
      }
    }


    //
    // Array operations; similar to our Set operations, but operate on a
    // Javascript Array so we *can* easily get a list of all values in the Set.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns true if value "value" is present within array "array".
     *
     * @param value a value of any type
     * @param array a Javascript Array
     */
    $scope.isValueInArray = function(value, array) {
      return (-1 != array.indexOf(value));
    }

    /**
     * If value "value" is already in array "array", remove it; otherwise,
     * add it.
     *
     * @param value a value of any type
     * @param array a Javascript Array
     */
    $scope.toggleValueInArray = function(value, array) {
      var i = array.indexOf(value);
      if (-1 == i) {
        array.push(value);
      } else {
        array.splice(i, 1);
      }
    }


    //
    // Miscellaneous utility functions.
    // TODO(epoger): move into a separate .js file?
    //

    /**
     * Returns a human-readable (in local time zone) time string for a
     * particular moment in time.
     *
     * @param secondsPastEpoch (numeric): seconds past epoch in UTC
     */
    $scope.localTimeString = function(secondsPastEpoch) {
      var d = new Date(secondsPastEpoch * 1000);
      return d.toString();
    }

  }
);
