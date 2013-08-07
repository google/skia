var MAX_SWAP_IMG_SIZE = 400;

angular.module('diff_viewer', []).
config(['$routeProvider', function($routeProvider) {
    // Show the list of differences by default
    $routeProvider.
    otherwise({ templateUrl: '/diff_list.html', controller: DiffListController});
}]).
directive('swapImg', function() {
    // Custom directive for showing an image that gets swapped my mouseover.
    return {
        restrict: 'E', // The directive can be used as an element name
        replace: true, // The directive replaces itself with the template
        template: '<canvas ng-mouseenter="swap()" ng-mouseleave="swap()"></canvas>',
        scope: { // The attributes below are bound to the scope
            leftSrc: '@',
            rightSrc: '@',
            side: '@'
        },
        link: function(scope, elm, attrs, ctrl) {
            var leftImage = new Image();
            var rightImage = new Image();
            var ctx = elm[0].getContext('2d');

            scope.render = function() {
                var image;
                if (scope.side == "left") {
                    image = leftImage;
                } else {
                    image = rightImage;
                }

                // Make it so the maximum size of an image is MAX_SWAP_IMG_SIZE, and the images are
                // scaled down in halves.
                var divisor = 1;
                while ((image.width / divisor) > MAX_SWAP_IMG_SIZE) {
                    divisor *= 2;
                }

                // Set canvas to correct size and draw the image into it
                elm[0].width = image.width / divisor;
                elm[0].height = image.height / divisor;
                ctx.drawImage(image, 0, 0, elm[0].width, elm[0].height);
            };

            // When the leftSrc attribute changes, load the image and then rerender
            attrs.$observe('leftSrc', function(value) {
                leftImage.src = value;
                leftImage.onload = function() {
                    if (scope.side == "left") {
                        scope.render();
                    }
                };
            });

            // When the rightSrc attribute changes, load the image and then rerender
            attrs.$observe('rightSrc', function(value) {
                rightImage.src = value;
                rightImage.onload = function() {
                    if (scope.side == "right") {
                        scope.render();
                    }
                };
            });

            // Swap which side to draw onto the canvas and then rerender
            scope.swap = function() {
                if (scope.side == "left") {
                    scope.side = "right";
                } else {
                    scope.side = "left";
                }
                scope.render();
            };
        }
    };
});

function DiffListController($scope, $http, $location, $timeout, $parse) {
    // Detect if we are running the web server version of the viewer. If so, we set a flag and
    // enable some extra functionality of the website for rebaselining.
    $scope.isDynamic = ($location.protocol() == "http" || $location.protocol() == "https");

    // Label each kind of differ for the sort buttons.
    $scope.differs = [
        {
            "title": "Different Pixels"
        },
        {
            "title": "Perceptual Difference"
        }
    ];

    // Puts the records within AngularJS scope
    $scope.records = SkPDiffRecords.records;

    // Keep track of the index of the last record to change so that shift clicking knows what range
    // of records to apply the action to.
    $scope.lastSelectedIndex = undefined;

    // Indicates which diff metric is used for sorting
    $scope.sortIndex = 1;

    // Called by the sort buttons to adjust the metric used for sorting
    $scope.setSortIndex = function(idx) {
        $scope.sortIndex = idx;

        // Because the index of things has most likely changed, the ranges of shift clicking no
        // longer make sense from the user's point of view. We reset it to avoid confusion.
        $scope.lastSelectedIndex = undefined;
    };

    // A predicate for pulling out the number used for sorting
    $scope.sortingDiffer = function(record) {
        return record.diffs[$scope.sortIndex].result;
    };

    // Flash status indicator on the page, and then remove it so the style can potentially be
    // reapplied later.
    $scope.flashStatus = function(success) {
        var flashStyle = success ? "success-flash" : "failure-flash";
        var flashDurationMillis = success ? 500 : 800;

        // Store the style in the record. The row will pick up the style this way instead of through
        // index because index can change with sort order.
        $scope.statusClass = flashStyle;

        // The animation cannot be repeated unless the class is removed the element.
        $timeout(function() {
            $scope.statusClass = "";
        }, flashDurationMillis);
    };

    $scope.selectedRebaseline = function(index, event) {
        // Retrieve the records in the same order they are displayed.
        var recordsInOrder = $parse("records | orderBy:sortingDiffer")($scope);

        // If the user is shift clicking, apply the last tick/untick to all elements in between this
        // record, and the last one they ticked/unticked.
        if (event.shiftKey && $scope.lastSelectedIndex !== undefined) {
            var currentAction = recordsInOrder[index].isRebaselined;
            var smallerIndex = Math.min($scope.lastSelectedIndex, index);
            var largerIndex = Math.max($scope.lastSelectedIndex, index);
            for (var recordIndex = smallerIndex; recordIndex <= largerIndex; recordIndex++) {
                recordsInOrder[recordIndex].isRebaselined = currentAction;
            }
            $scope.lastSelectedIndex = index;
        }
        else
        {
            $scope.lastSelectedIndex = index;
        }

    };

    $scope.commitRebaselines = function() {
        // Gather up all records that have the rebaseline set.
        var rebaselines = [];
        for (var recordIndex = 0; recordIndex < $scope.records.length; recordIndex++) {
            if ($scope.records[recordIndex].isRebaselined) {
                rebaselines.push($scope.records[recordIndex].testPath);
            }
        }
        $http.post("/commit_rebaselines", {
            "rebaselines": rebaselines
        }).success(function(data) {
            $scope.flashStatus(data.success);
        }).error(function() {
            $scope.flashStatus(false);
        });
    };
}
