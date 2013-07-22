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

                // Make it so the maximum size of an image is 500, and the images are scaled
                // down in halves.
                var divisor = 1;
                while ((image.width / divisor) > 500) {
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

function DiffListController($scope) {
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

    // Indicates which diff metric is used for sorting
    $scope.sortIndex = 1;

    // Called by the sort buttons to adjust the metric used for sorting
    $scope.setSortIndex = function(idx) {
        $scope.sortIndex = idx;
    };

    // A predicate for pulling out the number used for sorting
    $scope.sortingDiffer = function(record) {
        return record.diffs[$scope.sortIndex].result;
    };
}
