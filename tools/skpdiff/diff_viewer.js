var MAX_SWAP_IMG_SIZE = 400;
var MAGNIFIER_WIDTH = 200;
var MAGNIFIER_HEIGHT = 200;
var MAGNIFIER_HALF_WIDTH = MAGNIFIER_WIDTH * 0.5;
var MAGNIFIER_HALF_HEIGHT = MAGNIFIER_HEIGHT * 0.5;
// TODO add support for a magnified scale factor
var MAGNIFIER_SCALE_FACTOR = 2.0;

angular.module('diff_viewer', []).
directive('imgCompare', function() {
  // Custom directive for comparing (3-way) images
  return {
      restrict: 'E', // The directive can be used as an element name
      replace: true, // The directive replaces itself with the template
      template: '<canvas/>',
      scope: true,
      link: function(scope, elm, attrs, ctrl) {
          var image = new Image();
          var canvas = elm[0];
          var ctx = canvas.getContext('2d');

          var magnifyContent = false;
          var maskCanvas = false;

          // When the type attribute changes, load the image and then render
          attrs.$observe('type', function(value) {
              switch(value) {
                case "alphaMask":
                    image.src = scope.record.differencePath;
                    maskCanvas = true;
                    break;
                case "baseline":
                    image.src = scope.record.baselinePath;
                    magnifyContent = true;
                    break;
                case "test":
                    image.src = scope.record.testPath;
                    magnifyContent = true;
                    break;
                default:
                    console.log("Unknown type attribute on <img-compare>: " + value);
                    return;
              }

              image.onload = function() {
                  // compute the scaled image width/height for image and canvas
                  var divisor = 1;
                  // Make it so the maximum size of an image is MAX_SWAP_IMG_SIZE,
                  // and the images are scaled down in halves.
                  while ((image.width / divisor) > MAX_SWAP_IMG_SIZE) {
                      divisor *= 2;
                  }

                  scope.setImgScaleFactor(1 / divisor);

                  // Set canvas to correct size
                  canvas.width = image.width * scope.imgScaleFactor;
                  canvas.height = image.height * scope.imgScaleFactor;

                  // update the size for non-alphaMask canvas when loading baseline image
                  if (!scope.maskSizeUpdated) {
                      if (!maskCanvas) {
                          scope.updateMaskCanvasSize({width: canvas.width, height: canvas.height});
                      }
                      scope.maskCanvasSizeUpdated(true);
                  }

                  // render the image onto the canvas
                  scope.renderImage();
              }
          });

          // when updatedMaskSize changes, update mask canvas size.
          scope.$watch('updatedMaskSize', function(updatedSize) {
              if (!maskCanvas) {
                  return;
              }

              canvas.width = updatedSize.width;
              canvas.height = updatedSize.height;
          });

          // When the magnify attribute changes, render the magnified rect at
          // the default zoom level.
          scope.$watch('magnifyCenter', function(magCenter) {
              if (!magnifyContent) {
                  return;
              }

              scope.renderImage();

              if (!magCenter) {
                  return;
              }

              var magX = magCenter.x - MAGNIFIER_HALF_WIDTH;
              var magY = magCenter.y - MAGNIFIER_HALF_HEIGHT;

              var magMaxX = canvas.width - MAGNIFIER_WIDTH;
              var magMaxY = canvas.height - MAGNIFIER_HEIGHT;

              var magRect = { x: Math.max(0, Math.min(magX, magMaxX)),
                              y: Math.max(0, Math.min(magY, magMaxY)),
                              width: MAGNIFIER_WIDTH,
                              height: MAGNIFIER_HEIGHT
                            };

              var imgRect = { x: (magCenter.x / scope.imgScaleFactor) - MAGNIFIER_HALF_WIDTH,
                              y: (magCenter.y  / scope.imgScaleFactor) - MAGNIFIER_HALF_HEIGHT,
                              width: MAGNIFIER_WIDTH,
                              height: MAGNIFIER_HEIGHT
                            };

              // draw the magnified image
              ctx.clearRect(magRect.x, magRect.y, magRect.width, magRect.height);
              ctx.drawImage(image, imgRect.x, imgRect.y, imgRect.width, imgRect.height,
                            magRect.x, magRect.y, magRect.width, magRect.height);

              // draw the outline rect
              ctx.beginPath();
              ctx.rect(magRect.x, magRect.y, magRect.width, magRect.height);
              ctx.lineWidth = 2;
              ctx.strokeStyle = 'red';
              ctx.stroke();

          });

          // render the image to the canvas. This is often done every frame prior
          // to any special effects (i.e. magnification).
          scope.renderImage = function() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
          };

          // compute a rect (x,y,width,height) that represents the bounding box for
          // the magnification effect
          scope.computeMagnifierOutline = function(event) {
            var scaledWidth = MAGNIFIER_WIDTH * scope.imgScaleFactor;
            var scaledHeight = MAGNIFIER_HEIGHT * scope.imgScaleFactor;
            return {
              x: event.offsetX - (scaledWidth * 0.5),
              y: event.offsetY  - (scaledHeight * 0.5),
              width: scaledWidth,
              height: scaledHeight
            };
          };

          // event handler for mouse events that triggers the magnification
          // effect across the 3 images being compared.
          scope.MagnifyDraw = function(event, startMagnify) {
              if (startMagnify) {
                  scope.setMagnifierState(true);
              }  else if (!scope.magnifierOn) {
                  return;
              }

              scope.renderImage();

              // render the magnifier outline rect
              var rect = scope.computeMagnifierOutline(event);
              ctx.save();
              ctx.beginPath();
              ctx.rect(rect.x, rect.y, rect.width, rect.height);
              ctx.lineWidth = 2;
              ctx.strokeStyle = 'red';
              ctx.stroke();
              ctx.restore();

              // update scope on baseline / test that will cause them to render
              scope.setMagnifyCenter({x: event.offsetX, y: event.offsetY});
          };

          // event handler that triggers the end of the magnification effect and
          // resets all the canvases to their original state.
          scope.MagnifyEnd = function(event) {
              scope.renderImage();
              // update scope on baseline / test that will cause them to render
              scope.setMagnifierState(false);
              scope.setMagnifyCenter(undefined);
        };
      }
  };
});

function ImageController($scope, $http, $location, $timeout, $parse) {
  $scope.imgScaleFactor = 1.0;
  $scope.magnifierOn = false;
  $scope.magnifyCenter = undefined;
  $scope.updatedMaskSize = undefined;
  $scope.maskSizeUpdated = false;

  $scope.setImgScaleFactor = function(scaleFactor) {
    $scope.imgScaleFactor = scaleFactor;
  }

  $scope.setMagnifierState = function(magnifierOn) {
    $scope.magnifierOn = magnifierOn;
  }

  $scope.setMagnifyCenter = function(magnifyCenter) {
      $scope.magnifyCenter = magnifyCenter;
  }

  $scope.updateMaskCanvasSize = function(updatedSize) {
      $scope.updatedMaskSize = updatedSize;
  }

  $scope.maskCanvasSizeUpdated = function(flag) {
      $scope.maskSizeUpdated = flag;
  }
}

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
