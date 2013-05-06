#include <cstdio>
#include <string>

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "SampleApp.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkEvent.h"
#include "SkWindow.h"

class SkiaInstance;

namespace {
void FlushCallback(void* data, int32_t result);
}

SkiaInstance* gPluginInstance;
extern int main(int, char**);

class SkiaInstance : public pp::Instance {
 public:
    explicit SkiaInstance(PP_Instance instance) : pp::Instance(instance),
                                                  fFlushPending(false),
                                                  fGraphics2dContext(NULL),
                                                  fPixelBuffer(NULL)
    {
        gPluginInstance = this;
        application_init();
        char* commandName = "SampleApp";
        fWindow = new SampleWindow(NULL, 0, &commandName, NULL);
    }

    virtual ~SkiaInstance() {
        gPluginInstance = NULL;
        delete fWindow;
        application_term();
    }

    virtual void HandleMessage(const pp::Var& var_message) {
        // Receive a message from javascript.  Right now this just signals us to
        // get started.
        uint32_t width = 500;
        uint32_t height = 500;
        char buffer[2048];
        sprintf(buffer, "SetSize:%d,%d", width, height);
        PostMessage(buffer);
    }

    virtual void DidChangeView(const pp::Rect& position,
                               const pp::Rect& clip) {
        if (position.size().width() == width() &&
            position.size().height() == height()) {
            return;  // Size didn't change, no need to update anything.
        }
        // Create a new device context with the new size.
        DestroyContext();
        CreateContext(position.size());
        // Delete the old pixel buffer and create a new one.
        delete fPixelBuffer;
        fPixelBuffer = NULL;
        if (fGraphics2dContext != NULL) {
            fPixelBuffer = new pp::ImageData(this,
                                              PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                                              fGraphics2dContext->size(),
                                              false);
            fWindow->resize(position.size().width(), position.size().height());
            fWindow->update(NULL);
            paint();
        }
    }

    // Indicate whether a flush is pending.  This can only be called from the
    // main thread; it is not thread safe.
    bool flush_pending() const {
        return fFlushPending;
    }
    void set_flush_pending(bool flag) {
        fFlushPending = flag;
    }

  void paint() {
    if (fPixelBuffer) {
      // Draw some stuff.  TODO(borenet): Actually have SampleApp draw into
      // the plugin area.
      uint32_t w = fPixelBuffer->size().width();
      uint32_t h = fPixelBuffer->size().height();
      uint32_t* data = (uint32_t*) fPixelBuffer->data();
      // Create a bitmap using the fPixelBuffer pixels
      SkBitmap bitmap;
      bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
      bitmap.setPixels(data);
      // Create a canvas with the bitmap as the backend
      SkCanvas canvas(bitmap);

      canvas.drawColor(SK_ColorBLUE);
      SkRect rect = SkRect::MakeXYWH(10, 10, 80, 80);
      SkPaint rect_paint;
      rect_paint.setStyle(SkPaint::kFill_Style);
      rect_paint.setColor(SK_ColorRED);
      canvas.drawRect(rect, rect_paint);

      FlushPixelBuffer();
    }
  }

private:
    int width() const {
        return fPixelBuffer ? fPixelBuffer->size().width() : 0;
    }

    int height() const {
        return fPixelBuffer ? fPixelBuffer->size().height() : 0;
    }

    bool IsContextValid() const {
        return fGraphics2dContext != NULL;
    }

    void CreateContext(const pp::Size& size) {
        if (IsContextValid())
            return;
        fGraphics2dContext = new pp::Graphics2D(this, size, false);
        if (!BindGraphics(*fGraphics2dContext)) {
            SkDebugf("Couldn't bind the device context");
        }
    }

    void DestroyContext() {
        if (!IsContextValid())
            return;
        delete fGraphics2dContext;
        fGraphics2dContext = NULL;
    }

    void FlushPixelBuffer() {
        if (!IsContextValid())
            return;
        // Note that the pixel lock is held while the buffer is copied into the
        // device context and then flushed.
        fGraphics2dContext->PaintImageData(*fPixelBuffer, pp::Point());
        if (flush_pending())
            return;
        set_flush_pending(true);
        fGraphics2dContext->Flush(pp::CompletionCallback(&FlushCallback, this));
    }

    bool fFlushPending;
    pp::Graphics2D* fGraphics2dContext;
    pp::ImageData* fPixelBuffer;
    SampleWindow* fWindow;
};

class SkiaModule : public pp::Module {
public:
    SkiaModule() : pp::Module() {}
    virtual ~SkiaModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        gPluginInstance = new SkiaInstance(instance);
        return gPluginInstance;
    }
};

namespace {
void FlushCallback(void* data, int32_t result) {
    static_cast<SkiaInstance*>(data)->set_flush_pending(false);
}
}

namespace pp {
Module* CreateModule() {
    return new SkiaModule();
}
}  // namespace pp


///////////////////////////////////////////
///////////// SkOSWindow impl /////////////
///////////////////////////////////////////

void SkOSWindow::onSetTitle(const char title[])
{
    char buffer[2048];
    sprintf(buffer, "SetTitle:%s", title);
    gPluginInstance->PostMessage(buffer);
}

void SkOSWindow::onHandleInval(const SkIRect& rect)
{
    gPluginInstance->paint();
}

void SkOSWindow::onPDFSaved(const char title[], const char desc[],
                            const char path[]) {
}

///////////////////////////////////////////
/////////////// SkEvent impl //////////////
///////////////////////////////////////////

void SkEvent::SignalQueueTimer(SkMSec ms) {
}

void SkEvent::SignalNonEmptyQueue() {
}
