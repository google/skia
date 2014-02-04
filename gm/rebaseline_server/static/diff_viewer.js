var MAX_SWAP_IMG_SIZE = 200;
var MAGNIFIER_WIDTH = 100;
var MAGNIFIER_HEIGHT = 100;
var MAGNIFIER_HALF_WIDTH = MAGNIFIER_WIDTH * 0.5;
var MAGNIFIER_HALF_HEIGHT = MAGNIFIER_HEIGHT * 0.5;
var MAGNIFIER_SCALE_FACTOR = 2.0;

angular.module('diff_viewer', []).directive('imgCompare', function() {
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

          // When the type attribute changes, load the image and then render
          attrs.$observe('type', function(value) {
              switch(value) {
                case "differingPixelsInWhite":
                    magnifyContent = true;
                    break;
                case "differencePerPixel":
                    break;
                case "baseline":
                    magnifyContent = true;
                    break;
                case "test":
                    magnifyContent = true;
                    break;
                default:
                    console.log("Unknown type attribute on <img-compare>: " + value);
                    return;
              }

              image.src = attrs.src;
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

                  scope.renderImage();
              }
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
          // effect across the 4 images being compared.
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

  $scope.setImgScaleFactor = function(scaleFactor) {
    $scope.imgScaleFactor = scaleFactor;
  }

  $scope.setMagnifierState = function(magnifierOn) {
    $scope.magnifierOn = magnifierOn;
  }

  $scope.setMagnifyCenter = function(magnifyCenter) {
      $scope.magnifyCenter = magnifyCenter;
  }
}

