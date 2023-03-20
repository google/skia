// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      CanvasKit.MakeGPUDeviceContext = function(device) {
        if (!device) {
          return null;
        }

        // This allows native code to access this device by calling
        // `emscripten_webgpu_get_device().`
        CanvasKit.preinitializedWebGPUDevice = device;
        var context = this._MakeGrContext();
        context._device = device;

        return context;
      };

      CanvasKit.MakeGPUCanvasContext = function(devCtx, canvas, opts) {
        var canvasCtx = canvas.getContext('webgpu');
        if (!canvasCtx) {
          return null;
        }

        let format = (opts && opts.format) ? opts.format : navigator.gpu.getPreferredCanvasFormat();
        // GPUCanvasConfiguration
        canvasCtx.configure({
            device: devCtx._device,
            format: format,
            alphaMode: (opts && opts.alphaMode) ? opts.alphaMode : undefined,
        });

        var context = {
          '_inner': canvasCtx,
          '_deviceContext': devCtx,
          '_textureFormat': format,
        };
        context['requestAnimationFrame'] =  function(callback) {
          requestAnimationFrame(function() {
            const surface = CanvasKit.MakeGPUCanvasSurface(context);
            if (!surface) {
              console.error('Failed to initialize Surface for current canvas swapchain texture');
              return;
            }
            callback(surface.getCanvas());
            surface.flush();
            surface.dispose();
          });
        };
        return context;
      };

      CanvasKit.MakeGPUCanvasSurface = function(canvasCtx, colorSpace, width, height) {
        let context = canvasCtx._inner;
        if (!width) {
          width = context.canvas.width;
        }
        if (!height) {
          height = context.canvas.height;
        }
        let surface = this.MakeGPUTextureSurface(canvasCtx._deviceContext,
                                                 context.getCurrentTexture(),
                                                 canvasCtx._textureFormat,
                                                 width, height, colorSpace);
        surface._canvasContext = canvasCtx;
        return surface;
      };

      CanvasKit.MakeGPUTextureSurface = function (devCtx, texture, textureFormat, width, height, colorSpace) {
          colorSpace = colorSpace || null;

          // JsValStore and WebGPU are objects in Emscripten's library_html5_webgpu.js utility
          // library. JsValStore allows a WebGPU object to be imported by native code by calling the
          // various `emscripten_webgpu_import_*` functions.
          //
          // The CanvasKit WASM module is responsible for removing entries from the value store by
          // calling `emscripten_webgpu_release_js_handle` after importing the object.
          //
          // (see
          // https://github.com/emscripten-core/emscripten/blob/0e63f74f36b06849ef1c777b130783a43316ade0/src/library_html5_webgpu.js
          // for reference)
          return this._MakeGPUTextureSurface(
              devCtx,
              this.JsValStore.add(texture),
              this.WebGPU.TextureFormat.indexOf(textureFormat),
              width, height,
              colorSpace);
      };

      CanvasKit.Surface.prototype.assignCurrentSwapChainTexture = function() {
        // This feature is only supported for a Surface that was created via MakeGPUCanvasSurface.
        if (!this._canvasContext) {
          console.log('Surface is not bound to a canvas context');
          return false;
        }
        let ctx = this._canvasContext._inner;
        return this._replaceBackendTexture(
            CanvasKit.JsValStore.add(ctx.getCurrentTexture()),
            CanvasKit.WebGPU.TextureFormat.indexOf(this._canvasContext._textureFormat),
            ctx.canvas.width, ctx.canvas.height);
      };

      CanvasKit.Surface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
        if (!this.reportBackendTypeIsGPU()) {
          return this._requestAnimationFrameInternal(callback, dirtyRect);
        }

        return requestAnimationFrame(function() {
          // Replace the render target of the Surface with the current swapchain surface if this is
          // bound to a canvas context.
          if (this._canvasContext && !this.assignCurrentSwapChainTexture()) {
            console.log('failed to replace GPU backend texture');
            return;
          }
          callback(this.getCanvas());
          this.flush(dirtyRect);
        }.bind(this));
      };

      CanvasKit.Surface.prototype.drawOnce = function(callback, dirtyRect) {
        if (!this.reportBackendTypeIsGPU()) {
          this._drawOnceInternal(callback, dirtyRect);
          return;
        }

        requestAnimationFrame(function() {
          // Replace the render target of the Surface with the current swapchain surface if this is
          // bound to a canvas context.
          if (this._canvasContext && !this.assignCurrentSwapChainTexture()) {
            console.log('failed to replace GPU backend texture');
            return;
          }
          callback(this.getCanvas());
          this.flush(dirtyRect);
          this.dispose();
        }.bind(this));
      };
    });
}(Module));  // When this file is loaded in, the high level object is "Module".
