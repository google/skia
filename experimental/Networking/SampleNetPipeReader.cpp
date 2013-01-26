#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGPipe.h"
#include "SkSockets.h"
#include "SkOSMenu.h"

/**
 * A simple networked pipe reader
 *
 * This view will connect to a user specified server, in this case meaning any
 * Skia app that's has a SkTCPServer set up to broadcast its piped drawing data,
 * received all the data transmitted and attempt to reproduce the drawing calls.
 * This reader will only keep the latest batch of data. In order to keep up with
 * the server, which may be producing data at a much higher rate than the reader
 * is consuming, the reader will attempt multiple reads and only render the
 * latest frame. this behavior can be adjusted by changing MAX_READS_PER_FRAME
 * or disabled by setting fSync to false
 */

#define MAX_READS_PER_FRAME 12

class NetPipeReaderView : public SampleView {
public:
    NetPipeReaderView() {
        fSocket = NULL;
        fSync = true;
    }

    ~NetPipeReaderView() {
        if (fSocket) {
            delete fSocket;
        }
        fDataArray.reset();
    }
    virtual void requestMenu(SkOSMenu* menu) {
        menu->setTitle("Net Pipe Reader");
        menu->appendTextField("Server IP", "Server IP", this->getSinkID(),
                              "IP address");
        menu->appendSwitch("Sync", "Sync", this->getSinkID(), fSync);
    }

protected:
    static void readData(int cid, const void* data, size_t size,
                         SkSocket::DataType type, void* context) {
        NetPipeReaderView* view = (NetPipeReaderView*)context;
        view->onRead(data, size);
    }

    void onRead(const void* data, size_t size) {
        if (size > 0)
            fDataArray.append(size, (const char*)data);
    }

    bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Net Pipe Reader");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    bool onEvent(const SkEvent& evt) {
        SkString s;
        if (SkOSMenu::FindText(evt, "Server IP", &s)) {
            if (NULL != fSocket) {
                delete fSocket;
            }
            fSocket = new SkTCPClient(s.c_str());
            fSocket->connectToServer();
            SkDebugf("Connecting to %s\n", s.c_str());
            return true;
        }
        if (SkOSMenu::FindSwitchState(evt, "Sync", &fSync))
            return true;
        return this->INHERITED::onEvent(evt);
    }

    void onDrawContent(SkCanvas* canvas) {
        if (NULL == fSocket)
            return;

        if (fSocket->isConnected()) {
            int dataToRemove = fDataArray.count();
            if (fSync) {
                int numreads = 0;
                while (fSocket->readPacket(readData, this) > 0 &&
                       numreads < MAX_READS_PER_FRAME) {
                    // at this point, new data has been read and stored, discard
                    // old data since it's not needed anymore
                    SkASSERT(fDataArray.count() > dataToRemove);
                    fDataArray.remove(0, dataToRemove);
                    dataToRemove = fDataArray.count();
                    ++numreads;
                }
                // clean up if max reads reached
                if (numreads == MAX_READS_PER_FRAME &&
                    fDataArray.count() > dataToRemove)
                    fDataArray.remove(0, dataToRemove);
            }
            else {
                if (fSocket->readPacket(readData, this) > 0)
                    fDataArray.remove(0, dataToRemove);
            }
        }
        else
            fSocket->connectToServer();

        SkGPipeReader reader(canvas);
        size_t bytesRead;
        SkGPipeReader::Status fStatus = reader.playback(fDataArray.begin(),
                                                        fDataArray.count(),
                                                        &bytesRead);
        SkASSERT(SkGPipeReader::kError_Status != fStatus);
        this->inval(NULL);
    }

private:
    bool fSync;
    SkTDArray<char> fDataArray;
    SkTCPClient* fSocket;
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new NetPipeReaderView; }
static SkViewRegister reg(MyFactory);
