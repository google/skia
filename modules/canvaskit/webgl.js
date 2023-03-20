// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      function get(obj, attr, defaultValue) {
        if (obj && obj.hasOwnProperty(attr)) {
          return obj[attr];
        }
        return defaultValue;
      }

      CanvasKit.GetWebGLContext = function(canvas, attrs) {
        if (!canvas) {
          throw 'null canvas passed into makeWebGLContext';
        }
        var contextAttributes = {
          'alpha': get(attrs, 'alpha', 1),
          'depth': get(attrs, 'depth', 1),
          'stencil': get(attrs, 'stencil', 8),
          'antialias': get(attrs, 'antialias', 0),
          'premultipliedAlpha': get(attrs, 'premultipliedAlpha', 1),
          'preserveDrawingBuffer': get(attrs, 'preserveDrawingBuffer', 0),
          'preferLowPowerToHighPerformance': get(attrs, 'preferLowPowerToHighPerformance', 0),
          'failIfMajorPerformanceCaveat': get(attrs, 'failIfMajorPerformanceCaveat', 0),
          'enableExtensionsByDefault': get(attrs, 'enableExtensionsByDefault', 1),
          'explicitSwapControl': get(attrs, 'explicitSwapControl', 0),
          'renderViaOffscreenBackBuffer': get(attrs, 'renderViaOffscreenBackBuffer', 0),
        };

        if (attrs && attrs['majorVersion']) {
          contextAttributes['majorVersion'] = attrs['majorVersion']
        } else {
          // Default to WebGL 2 if available and not specified.
          contextAttributes['majorVersion'] = (typeof WebGL2RenderingContext !== 'undefined') ? 2 : 1;
        }

        // This check is from the emscripten version
        if (contextAttributes['explicitSwapControl']) {
          throw 'explicitSwapControl is not supported';
        }
        // Creates a WebGL context and sets it to be the current context.
        // These functions are defined in emscripten's library_webgl.js
        var handle = GL.createContext(canvas, contextAttributes);
        if (!handle) {
          return 0;
        }
        GL.makeContextCurrent(handle);
        // Emscripten does not enable this by default and Skia needs this to handle certain GPU
        // corner cases.
        GL.currentContext.GLctx.getExtension('WEBGL_debug_renderer_info');
        return handle;
      };

      CanvasKit.deleteContext = function(handle) {
        GL.deleteContext(handle);
      };

      CanvasKit._setTextureCleanup({
        'deleteTexture': function(webglHandle, texHandle) {
          var tex = GL.textures[texHandle];
          if (tex) {
            GL.getContext(webglHandle).GLctx.deleteTexture(tex);
          }
          GL.textures[texHandle] = null;
        },
      });

      CanvasKit.MakeWebGLContext = function(ctx) {
        // Make sure we are pointing at the right WebGL context.
        if (!this.setCurrentContext(ctx)) {
          return null;
        }
        var grCtx = this._MakeGrContext();
        if (!grCtx) {
          return null;
        }
        // This context is an index into the emscripten-provided GL wrapper.
        grCtx._context = ctx;
        var oldDelete = grCtx.delete.bind(grCtx);
        // We need to make sure we are focusing on the correct webgl context
        // when Skia cleans up the context.
        grCtx['delete'] = function() {
          CanvasKit.setCurrentContext(this._context);
          oldDelete();
        }.bind(grCtx);
        // Save this so it is easy to access (e.g. Image.readPixels)
        GL.currentContext.grDirectContext = grCtx;
        return grCtx;
      };

      CanvasKit.MakeGrContext = CanvasKit.MakeWebGLContext;

      CanvasKit.GrDirectContext.prototype.getResourceCacheLimitBytes = function() {
          CanvasKit.setCurrentContext(this._context);
          this._getResourceCacheLimitBytes();
      };

      CanvasKit.GrDirectContext.prototype.getResourceCacheUsageBytes = function() {
          CanvasKit.setCurrentContext(this._context);
          this._getResourceCacheUsageBytes();
      };

      CanvasKit.GrDirectContext.prototype.releaseResourcesAndAbandonContext = function() {
          CanvasKit.setCurrentContext(this._context);
          this._releaseResourcesAndAbandonContext();
      };

      CanvasKit.GrDirectContext.prototype.setResourceCacheLimitBytes = function(maxResourceBytes) {
          CanvasKit.setCurrentContext(this._context);
          this._setResourceCacheLimitBytes(maxResourceBytes);
      };

      CanvasKit.MakeOnScreenGLSurface = function(grCtx, w, h, colorspace, sc, st) {
        if (!this.setCurrentContext(grCtx._context)) {
          return null;
        }
        var surface;
        // zero is a valid value for sample count or stencil bits.
        if (sc === undefined || st === undefined) {
          surface = this._MakeOnScreenGLSurface(grCtx, w, h, colorspace);
        } else {
          surface = this._MakeOnScreenGLSurface(grCtx, w, h, colorspace, sc, st);
        }
        if (!surface) {
          return null;
        }
        surface._context = grCtx._context;
        return surface;
      }

      CanvasKit.MakeRenderTarget = function() {
        var grCtx = arguments[0];
        if (!this.setCurrentContext(grCtx._context)) {
          return null;
        }
        var surface;
        if (arguments.length === 3) {
          surface = this._MakeRenderTargetWH(grCtx, arguments[1], arguments[2]);
          if (!surface) {
            return null;
          }
        } else if (arguments.length === 2) {
          surface = this._MakeRenderTargetII(grCtx, arguments[1]);
          if (!surface) {
            return null;
          }
        } else {
          Debug('Expected 2 or 3 params');
          return null;
        }
        surface._context = grCtx._context;
        return surface;
      }

      // idOrElement can be of types:
      //  - String - in which case it is interpreted as an id of a
      //          canvas element.
      //  - HTMLCanvasElement - in which the provided canvas element will
      //          be used directly.
      // colorSpace - sk_sp<ColorSpace> - one of the supported color spaces:
      //          CanvasKit.ColorSpace.SRGB
      //          CanvasKit.ColorSpace.DISPLAY_P3
      //          CanvasKit.ColorSpace.ADOBE_RGB
      CanvasKit.MakeWebGLCanvasSurface = function(idOrElement, colorSpace, attrs) {
        colorSpace = colorSpace || null;
        var canvas = idOrElement;
        var isHTMLCanvas = typeof HTMLCanvasElement !== 'undefined' && canvas instanceof HTMLCanvasElement;
        var isOffscreenCanvas = typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas;
        if (!isHTMLCanvas && !isOffscreenCanvas) {
          canvas = document.getElementById(idOrElement);
          if (!canvas) {
            throw 'Canvas with id ' + idOrElement + ' was not found';
          }
        }

        var ctx = this.GetWebGLContext(canvas, attrs);
        if (!ctx || ctx < 0) {
          throw 'failed to create webgl context: err ' + ctx;
        }

        var grcontext = this.MakeWebGLContext(ctx);

        // Note that canvas.width/height here is used because it gives the size of the buffer we're
        // rendering into. This may not be the same size the element is displayed on the page, which
        // controlled by css, and available in canvas.clientWidth/height.
        var surface = this.MakeOnScreenGLSurface(grcontext, canvas.width, canvas.height, colorSpace);
        if (!surface) {
          Debug('falling back from GPU implementation to a SW based one');
          // we need to throw away the old canvas (which was locked to
          // a webGL context) and create a new one so we can
          var newCanvas = canvas.cloneNode(true);
          var parent = canvas.parentNode;
          parent.replaceChild(newCanvas, canvas);
          // add a class so the user can detect that it was replaced.
          newCanvas.classList.add('ck-replaced');

          return CanvasKit.MakeSWCanvasSurface(newCanvas);
        }
        return surface;
      };
      // Default to trying WebGL first.
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeWebGLCanvasSurface;

      function pushTexture(tex) {
        // GL is an emscripten object that holds onto WebGL state. One item in that state is
        // an array of textures, of which the index is the handle/id. We must call getNewId so
        // the GL's tracking of textures is up to date and we do not accidentally use the same
        // texture in two different places if Skia creates a texture. (e.g. skbug.com/12797)
        var texHandle = GL.getNewId(GL.textures);
        GL.textures[texHandle] = tex;
        return texHandle
      }

      CanvasKit.Surface.prototype.makeImageFromTexture = function(tex, info) {
        CanvasKit.setCurrentContext(this._context);
        var texHandle = pushTexture(tex);
        var img = this._makeImageFromTexture(this._context, texHandle, info);
        if (img) {
          img._tex = texHandle;
        }
        return img;
      };

      // We try to find the natural media type (for <img> and <video>), display* for
      // https://developer.mozilla.org/en-US/docs/Web/API/VideoFrame and then fall back to
      // the height and width (to cover <canvas>, ImageBitmap or ImageData).
      function getHeight(src) {
        return src['naturalHeight'] || src['videoHeight'] || src['displayHeight'] || src['height'];
      }

      function getWidth(src) {
        return src['naturalWidth'] || src['videoWidth'] || src['displayWidth'] || src['width'];
      }

      function setupTexture(glCtx, newTex, imageInfo, srcIsPremul) {
        glCtx.bindTexture(glCtx.TEXTURE_2D, newTex);
        // See https://github.com/flutter/flutter/issues/106433#issuecomment-1169102945
        // for an example of what can happen if we do not set this.
        if (!srcIsPremul && imageInfo['alphaType'] === CanvasKit.AlphaType.Premul) {
          glCtx.pixelStorei(glCtx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
        }
        return newTex;
      }

      function resetTexture(glCtx, imageInfo, srcIsPremul) {
        // If we set this earlier, we want to unset it now.
        if (!srcIsPremul && imageInfo['alphaType'] === CanvasKit.AlphaType.Premul) {
          glCtx.pixelStorei(glCtx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
        }
        glCtx.bindTexture(glCtx.TEXTURE_2D, null);
      }

      CanvasKit.Surface.prototype.makeImageFromTextureSource = function(src, info, srcIsPremul) {
        if (!info) {
          // If the user didn't specify the image info, use some sensible defaults.
          info = {
            'height': getHeight(src),
            'width': getWidth(src),
            'colorType': CanvasKit.ColorType.RGBA_8888,
            'alphaType': srcIsPremul ? CanvasKit.AlphaType.Premul: CanvasKit.AlphaType.Unpremul,
          };
        }
        if (!info['colorSpace']) {
          info['colorSpace'] = CanvasKit.ColorSpace.SRGB;
        }
        if (info['colorType'] !== CanvasKit.ColorType.RGBA_8888) {
          Debug('colorType currently has no impact on makeImageFromTextureSource');
        }

        // We want to be pointing at the context associated with this surface.
        CanvasKit.setCurrentContext(this._context);
        var glCtx = GL.currentContext.GLctx;
        var newTex = setupTexture(glCtx, glCtx.createTexture(), info, srcIsPremul);
        if (GL.currentContext.version === 2) {
          glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, info['width'], info['height'], 0, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
        } else {
          glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
        }
        resetTexture(glCtx, info);
        this._resetContext();
        return this.makeImageFromTexture(newTex, info);
      };

      CanvasKit.Surface.prototype.updateTextureFromSource = function(img, src, srcIsPremul) {
        if (!img._tex) {
          Debug('Image is not backed by a user-provided texture');
          return;
        }
        CanvasKit.setCurrentContext(this._context);
        var ii = img.getImageInfo();
        var glCtx = GL.currentContext.GLctx;
        // Copy the contents of src over the texture associated with this image.
        var tex = setupTexture(glCtx, GL.textures[img._tex], ii, srcIsPremul);
        if (GL.currentContext.version === 2) {
          glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, getWidth(src), getHeight(src), 0, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
        } else {
          glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
        }
        resetTexture(glCtx, ii, srcIsPremul);
        // Tell Skia we messed with the currently bound texture.
        this._resetContext();
        // Create a new texture entry and put null into the old slot. This keeps our texture alive,
        // otherwise it will be deleted when we delete the old Image.
        GL.textures[img._tex] = null;
        img._tex = pushTexture(tex);
        ii['colorSpace'] = img.getColorSpace();
        // Skia may cache parts of the image, and some places assume images are immutable. In order
        // to make things work, we create a new SkImage based on the same texture as the old image.
        var newImg = this._makeImageFromTexture(this._context, img._tex, ii);
        // To make things more ergonomic for the user, we change passed in img object to refer
        // to the new image and clean up the old SkImage object. This has the effect of updating
        // the Image (from the user's side of things), because they shouldn't be caring about what
        // part of WASM memory we are pointing to.
        // The $$ part is provided by emscripten's embind, so this could break if they change
        // things on us.
        // https://github.com/emscripten-core/emscripten/blob/a65d70c809f077542649c60097787e1c7460ced6/src/embind/embind.js
        // They do not do anything special to keep closure from minifying things and neither do we.
        var oldPtr = img.$$.ptr;
        var oldSmartPtr = img.$$.smartPtr;
        img.$$.ptr = newImg.$$.ptr;
        img.$$.smartPtr = newImg.$$.smartPtr;
        // We want to clean up the previous image, so we swap out the pointers and call delete on it
        // which should have that effect.
        newImg.$$.ptr = oldPtr;
        newImg.$$.smartPtr = oldSmartPtr;
        newImg.delete();
        // Clean up the colorspace that we used.
        ii['colorSpace'].delete();
      }

      CanvasKit.MakeLazyImageFromTextureSource = function(src, info, srcIsPremul) {
        if (!info) {
          info = {
            'height': getHeight(src),
            'width': getWidth(src),
            'colorType': CanvasKit.ColorType.RGBA_8888,
            'alphaType': srcIsPremul ? CanvasKit.AlphaType.Premul : CanvasKit.AlphaType.Unpremul,
          };
        }
        if (!info['colorSpace']) {
          info['colorSpace'] = CanvasKit.ColorSpace.SRGB;
        }
        if (info['colorType'] !== CanvasKit.ColorType.RGBA_8888) {
          Debug('colorType currently has no impact on MakeLazyImageFromTextureSource');
        }

        var callbackObj = {
          'makeTexture': function() {
            // This callback function will make a texture on the current drawing surface (i.e.
            // the current WebGL context). It assumes that Skia is just about to draw the texture
            // to the desired surface, and thus the currentContext is the correct one.
            // This is a lot easier than needing to pass the surface handle from the C++ side here.
            var ctx = GL.currentContext;
            var glCtx = ctx.GLctx;
            var newTex = setupTexture(glCtx, glCtx.createTexture(), info, srcIsPremul);
            if (ctx.version === 2) {
              glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, info['width'], info['height'], 0, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
            } else {
              glCtx.texImage2D(glCtx.TEXTURE_2D, 0, glCtx.RGBA, glCtx.RGBA, glCtx.UNSIGNED_BYTE, src);
            }
            resetTexture(glCtx, info, srcIsPremul);
            return pushTexture(newTex);
          },
          'freeSrc': function() {
            // This callback will be executed whenever the returned image is deleted. This gives
            // us a chance to free up the src (which we now own). Generally, there's nothing
            // we need to do (we can let JS garbage collection do its thing). The one exception
            // is for https://developer.mozilla.org/en-US/docs/Web/API/VideoFrame, which we should
            // close when we are done.
          },
        }
        if (src.constructor.name === 'VideoFrame') {
          callbackObj['freeSrc'] = function() {
            src.close();
          }
        }
        return CanvasKit.Image._makeFromGenerator(info, callbackObj);
      }

      CanvasKit.setCurrentContext = function(ctx) {
        if (!ctx) {
          return false;
        }
        return GL.makeContextCurrent(ctx);
      };

      CanvasKit.getCurrentGrDirectContext = function() {
        if (GL.currentContext && GL.currentContext.grDirectContext &&
            !GL.currentContext.grDirectContext['isDeleted']()) {
          return GL.currentContext.grDirectContext;
        }
        return null;
      };

    });
}(Module)); // When this file is loaded in, the high level object is "Module";
