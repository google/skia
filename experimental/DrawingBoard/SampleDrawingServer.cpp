#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGPipe.h"
#include "SkSockets.h"
#include "SkNetPipeController.h"
#include "SkCornerPathEffect.h"
#include "SkOSMenu.h"
#include <map>
class DrawingServerView : public SampleView {
public:
	DrawingServerView(){
        fServer = new SkTCPServer(40000);
        fServer->suspendWrite();
        fTotalBytesRead = fTotalBytesWritten = 0;
        fVector = true;
    }
    ~DrawingServerView() {
        delete fServer;
        fData.reset();
        fBuffer.reset();
        fClientMap.clear();
    }
    
    virtual void requestMenu(SkOSMenu* menu) {
        menu->setTitle("Drawing Server");
        menu->appendAction("Clear", this->getSinkID());
        menu->appendSwitch("Vector", "Vector", this->getSinkID(), fVector);
    }
    
protected:
    static void readData(int cid, const void* data, size_t size, 
                         SkSocket::DataType type, void* context) {
        DrawingServerView* view = (DrawingServerView*)context;
        view->onRead(cid, data, size, type);
    }
    
    void onRead(int cid, const void* data, size_t size, SkSocket::DataType type) {
        if (NULL == data && size <= 0)
            return;
        
        ClientState* cs;
        std::map<int, ClientState*>::iterator it = fClientMap.find(cid);
        if (it == fClientMap.end()) { //New client
            cs = new ClientState;
            cs->bufferBase = 0;
            cs->bufferSize = 0;
            fClientMap[cid] = cs;
        }
        else {
            cs = it->second;
        }
        
        if (type == SkSocket::kPipeReplace_type) {
            fBuffer.remove(cs->bufferBase, cs->bufferSize);
            
            for (it = fClientMap.begin(); it != fClientMap.end(); ++it) {
                if (cid == it->first)
                    continue;
                else {
                    if (it->second->bufferBase > cs->bufferBase) {
                        it->second->bufferBase -= cs->bufferSize;
                        SkASSERT(it->second->bufferBase >= 0);
                    }
                }
            }
            
            cs->bufferBase = fBuffer.count();
            cs->bufferSize = size;
            fBuffer.append(size, (const char*)data);
        }
        else if (type == SkSocket::kPipeAppend_type) {
            fData.append(size, (const char*)data);
            fServer->resumeWrite();
            fServer->writePacket(fData.begin() + fTotalBytesWritten,
                                 fData.count() - fTotalBytesWritten,
                                 SkSocket::kPipeAppend_type);
            fTotalBytesWritten = fData.count();
            fServer->suspendWrite();
            //this->clearBitmap();
        }
        else {
            //other types of data
        }
    }
    
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Drawing Server");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    bool onEvent(const SkEvent& evt) {
        if (SkOSMenu::FindAction(&evt, "Clear")) {
            this->clear();
            return true;
        }
        if (SkOSMenu::FindSwitchState(&evt, "Vector", &fVector)) {
            this->clearBitmap();
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }
    
    
    virtual void onDrawContent(SkCanvas* canvas) {
        if (fCurrMatrix != canvas->getTotalMatrix()) {
            fTotalBytesRead = 0;
            fCurrMatrix = canvas->getTotalMatrix();
        }

        fServer->acceptConnections();
        if (fServer->readPacket(readData, this) > 0) {
            fServer->resumeWrite();
        }
        else {
            fServer->suspendWrite();
        }
        
        size_t bytesRead;
        SkCanvas bufferCanvas(fBase);
        SkCanvas* tempCanvas;
        while (fTotalBytesRead < fData.count()) {
            if (fVector)
                tempCanvas = canvas;
            else
                tempCanvas = &bufferCanvas;
            SkGPipeReader reader(tempCanvas);
            SkGPipeReader::Status stat = reader.playback(fData.begin() + fTotalBytesRead,
                                                         fData.count() - fTotalBytesRead,
                                                         &bytesRead);
            SkASSERT(SkGPipeReader::kError_Status != stat);
            fTotalBytesRead += bytesRead;
            
            if (SkGPipeReader::kDone_Status == stat) {}
        }
        if (fVector)
            fTotalBytesRead = 0;
        else
            canvas->drawBitmap(fBase, 0, 0, NULL);
        
        size_t totalBytesRead = 0;
        while (totalBytesRead < fBuffer.count()) {
            SkGPipeReader reader(canvas);
            reader.playback(fBuffer.begin() + totalBytesRead,
                            fBuffer.count() - totalBytesRead,
                            &bytesRead);
            totalBytesRead += bytesRead;
        }
        
        fServer->writePacket(fBuffer.begin(), fBuffer.count(), 
                             SkSocket::kPipeReplace_type);

        this->inval(NULL);
    }
    
    virtual void onSizeChange() {
        this->INHERITED::onSizeChange();
        fBase.setConfig(SkBitmap::kARGB_8888_Config, this->width(), this->height());
        fBase.allocPixels(NULL);
        this->clearBitmap();
    }
    
private:
    void clear() {
        fData.reset();
        fBuffer.reset();
        fTotalBytesRead = fTotalBytesWritten = 0;
        fClientMap.clear();
        this->clearBitmap();
    }
    void clearBitmap() {
        fTotalBytesRead = 0;
        fBase.eraseColor(fBGColor);
    }
    
    struct ClientState {
        int bufferBase;
        int bufferSize;
    };
    std::map<int, ClientState*> fClientMap;
    SkTDArray<char>             fData;
    SkTDArray<char>             fBuffer;
    size_t                      fTotalBytesRead;
    size_t                      fTotalBytesWritten;
    SkMatrix                    fCurrMatrix;
    SkBitmap                    fBase;
    bool                        fVector;
    SkTCPServer*                fServer;
    typedef SampleView INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawingServerView; }
static SkViewRegister reg(MyFactory);
