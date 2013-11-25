#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGPipe.h"
#include "SkSockets.h"
#include "SkNetPipeController.h"
#include "SkCornerPathEffect.h"
#include "SkColorPalette.h"
#include "SkOSMenu.h"


/**
 * Drawing Client
 *
 * A drawing client that allows a user to perform simple brush stokes with
 * a selected color and brush size. The drawing client communicates with a
 * drawing server to send/receive data to/from other clients connected to the
 * same server. The drawing client stores data in fData and fBuffer depending on
 * the data type. Append type means that the drawing data is a completed stroke
 * and Replace type means that the drawing data is in progress and will be
 * replaced by subsequent data. fData and fBuffer are read by a pipe reader and
 * reproduce the drawing. When the client is in a normal state, the data stored
 * on the client and the server should be identical.
 * The drawing client is also able to switch between vector and bitmap drawing.
 * The drawing client also renders the latest drawing stroke locally in order to
 * produce better reponses. This can be disabled by calling
 * controller.disablePlayBack(), which will introduce a lag between the input
 * and the drawing.
 * Note: in order to keep up with the drawing data, the client will try to read
 * a few times each frame in case more than one frame worth of data has been
 * received and render them together. This behavior can be adjusted by tweaking
 * MAX_READ_PER_FRAME or disabled by turning fSync to false
 */

#define MAX_READ_PER_FRAME 5

class DrawingClientView : public SampleView {
public:
    DrawingClientView() {
        fSocket = NULL;
        fTotalBytesRead = 0;
        fPalette = new SkColorPalette;
        fPalette->setSize(100, 300);
        fPalette->setVisibleP(true);
        this->attachChildToFront(fPalette);
        fPalette->unref();
        fBrushSize = 2.5;
        fAA = false;
        fPaletteVisible = true;
        fSync = true;
        fVector = true;
    }
    ~DrawingClientView() {
        if (fSocket) {
            delete fSocket;
        }
        fData.reset();
        fBuffer.reset();
    }

    virtual void requestMenu(SkOSMenu* menu) {
        menu->setTitle("Drawing Client");
        menu->appendTextField("Server IP", "Server IP", this->getSinkID(),
                              "IP address or hostname");
        menu->appendSwitch("Vector", "Vector", this->getSinkID(), fVector);
        menu->appendSlider("Brush Size", "Brush Size", this->getSinkID(), 1.0,
                           100.0, fBrushSize);
        menu->appendSwitch("Anti-Aliasing", "AA", this->getSinkID(), fAA);
        menu->appendSwitch("Show Color Palette", "Palette", this->getSinkID(),
                           fPaletteVisible);
        menu->appendSwitch("Sync", "Sync", this->getSinkID(), fSync);
        menu->appendAction("Clear", this->getSinkID());
    }

protected:

    static void readData(int cid, const void* data, size_t size,
                         SkSocket::DataType type, void* context) {
        DrawingClientView* view = (DrawingClientView*)context;
        view->onRead(cid, data, size, type);
    }

    void onRead(int cid, const void* data, size_t size, SkSocket::DataType type) {
        if (size > 0) {
            fBuffer.reset();
            if (type == SkSocket::kPipeReplace_type)
                fBuffer.append(size, (const char*)data);
            else if (type == SkSocket::kPipeAppend_type)
                fData.append(size, (const char*)data);
            else {
                //other types of data
            }
        }
    }

    bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Drawing Client");
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }

    bool onEvent(const SkEvent& evt) {;
        if (SkOSMenu::FindSliderValue(evt, "Brush Size", &fBrushSize))
            return true;

        SkString s;
        if (SkOSMenu::FindText(evt, "Server IP", &s)) {
            if (NULL != fSocket) {
                delete fSocket;
            }
            fSocket = new SkTCPClient(s.c_str(), 40000);
            fSocket->connectToServer();
            fSocket->suspendWrite();
            SkDebugf("Connecting to %s\n", s.c_str());
            fData.reset();
            fBuffer.reset();
            this->inval(NULL);
            return true;
        }
        if (SkOSMenu::FindSwitchState(evt, "AA", &fAA) ||
            SkOSMenu::FindSwitchState(evt, "Sync", &fSync))
            return true;
        if (SkOSMenu::FindSwitchState(evt, "Vector", &fVector)) {
            this->clearBitmap();
            return true;
        }
        if (SkOSMenu::FindAction(evt, "Clear")) {
            this->clear();
            return true;
        }
        if (SkOSMenu::FindSwitchState(evt, "Palette", &fPaletteVisible)) {
            fPalette->setVisibleP(fPaletteVisible);
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }

    virtual bool onClick(SkView::Click* click) {
        switch (click->fState) {
            case SkView::Click::kDown_State:
                fCurrLine.moveTo(click->fCurr);
                fType = SkSocket::kPipeReplace_type;
                if (fSocket)
                    fSocket->resumeWrite();
                break;
            case SkView::Click::kMoved_State:
                fCurrLine.lineTo(click->fCurr);
                break;
            case SkView::Click::kUp_State:
                fType = SkSocket::kPipeAppend_type;
                break;
            default:
                break;
        }
        return true;
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        if (fSocket) {
            if (fSocket->isConnected()) {
                if (fSync) {
                    int count = 0;
                    while (fSocket->readPacket(readData, this) > 0 &&
                           count < MAX_READ_PER_FRAME)
                        ++count;
                }
                else
                    fSocket->readPacket(readData, this);
            }
            else
                fSocket->connectToServer();
        }
        size_t bytesRead = 0;
        SkGPipeReader::Status status;
        SkCanvas bufferCanvas(fBase);
        SkCanvas* tempCanvas;
        while (fTotalBytesRead < fData.count()) {
            if (fVector)
                tempCanvas = canvas;
            else
                tempCanvas = &bufferCanvas;
            SkGPipeReader reader(tempCanvas);
            status = reader.playback(fData.begin() + fTotalBytesRead,
                                     fData.count() - fTotalBytesRead,
                                     &bytesRead);
            SkASSERT(SkGPipeReader::kError_Status != status);
            fTotalBytesRead += bytesRead;
        }
        if (fVector)
            fTotalBytesRead = 0;
        else
            canvas->drawBitmap(fBase, 0, 0, NULL);

        size_t totalBytesRead = 0;
        while (totalBytesRead < fBuffer.count()) {
            SkGPipeReader reader(canvas);
            status = reader.playback(fBuffer.begin() + totalBytesRead,
                                     fBuffer.count() - totalBytesRead,
                                     &bytesRead);
            SkASSERT(SkGPipeReader::kError_Status != status);
            totalBytesRead += bytesRead;
        }

        SkNetPipeController controller(canvas);
        SkGPipeWriter writer;
        SkCanvas* writerCanvas = writer.startRecording(&controller,
                                                       SkGPipeWriter::kCrossProcess_Flag);

        //controller.disablePlayback();
        SkPaint p;
        p.setColor(fPalette->getColor());
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(fBrushSize);
        p.setStrokeCap(SkPaint::kRound_Cap);
        p.setStrokeJoin(SkPaint::kRound_Join);
        p.setAntiAlias(fAA);
        p.setPathEffect(new SkCornerPathEffect(55))->unref();
        writerCanvas->drawPath(fCurrLine, p);
        writer.endRecording();

        controller.writeToSocket(fSocket, fType);
        if (fType == SkSocket::kPipeAppend_type && fSocket) {
            fSocket->suspendWrite();
            fCurrLine.reset();
        }

        this->inval(NULL);
    }

    virtual void onSizeChange() {
        this->INHERITED::onSizeChange();
        fPalette->setLoc(this->width()-100, 0);
        fBase.setConfig(SkBitmap::kARGB_8888_Config, this->width(), this->height());
        fBase.allocPixels(NULL);
        this->clearBitmap();
    }

private:
    void clear() {
        fData.reset();
        fBuffer.reset();
        fCurrLine.reset();
        fTotalBytesRead = 0;
        this->clearBitmap();
    }
    void clearBitmap() {
        fTotalBytesRead = 0;
        fBase.eraseColor(fBGColor);
    }
    SkTDArray<char>     fData;
    SkTDArray<char>     fBuffer;
    SkBitmap            fBase;
    SkPath              fCurrLine;
    SkTCPClient*        fSocket;
    SkSocket::DataType  fType;
    SkColorPalette*     fPalette;
    bool                fPaletteVisible;
    size_t              fTotalBytesRead;
    SkScalar            fBrushSize;
    bool                fAA;
    bool                fSync;
    bool                fVector;

    typedef SampleView INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawingClientView; }
static SkViewRegister reg(MyFactory);
