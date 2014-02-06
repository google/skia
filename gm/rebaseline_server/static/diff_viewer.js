// What portion of the image width will be taken up by the magnifier.
var MAGNIFIER_WIDTH_FACTOR = 0.66;

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

          // When the type attribute changes, set magnifyContent appropriately.
          attrs.$observe('type', function(value) {
              switch(attrs.type) {
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
          });

          // Reset ImageController's scale and render the image;
          // we need to do this whenever attrs.src or attrs.width changes.
          scope.setScaleAndRender = function() {
              // compute the scaled image width/height for image and canvas
              scope.setImgScale(image.width, attrs.width);

              // Set canvas to correct size
              canvas.width = image.width / scope.imgScaleDivisor;
              canvas.height = image.height / scope.imgScaleDivisor;

              scope.renderImage();
          };
          attrs.$observe('src', function(value) {
              image.src = attrs.src;
              image.onload = scope.setScaleAndRender;
          });
          attrs.$observe('width', scope.setScaleAndRender);

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

              var magX = magCenter.x - scope.magnifierHalfWidth;
              var magY = magCenter.y - scope.magnifierHalfHeight;

              var magMaxX = canvas.width - scope.magnifierWidth;
              var magMaxY = canvas.height - scope.magnifierHeight;

              var magRect = { x: Math.max(0, Math.min(magX, magMaxX)),
                              y: Math.max(0, Math.min(magY, magMaxY)),
                              width: scope.magnifierWidth,
                              height: scope.magnifierHeight
                            };

              var imgRect = { x: (magCenter.x * scope.imgScaleDivisor) - scope.magnifierHalfWidth,
                              y: (magCenter.y * scope.imgScaleDivisor) - scope.magnifierHalfHeight,
                              width: scope.magnifierWidth,
                              height: scope.magnifierHeight
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
            var scaledWidth = scope.magnifierWidth / scope.imgScaleDivisor;
            var scaledHeight = scope.magnifierHeight / scope.imgScaleDivisor;
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
  $scope.imgScaleDivisor = 1.0;
  $scope.magnifierOn = false;
  $scope.magnifyCenter = undefined;

  $scope.setImgScale = function(srcImageWidth, displayWidth) {
    $scope.imgScaleDivisor = srcImageWidth / displayWidth;
    $scope.magnifierWidth = displayWidth * MAGNIFIER_WIDTH_FACTOR;
    $scope.magnifierHeight = $scope.magnifierWidth;
    $scope.magnifierHalfWidth = $scope.magnifierWidth / 2;
    $scope.magnifierHalfHeight = $scope.magnifierHeight / 2;
  }

  $scope.setMagnifierState = function(magnifierOn) {
    $scope.magnifierOn = magnifierOn;
  }

  $scope.setMagnifyCenter = function(magnifyCenter) {
    $scope.magnifyCenter = magnifyCenter;
  }
}
