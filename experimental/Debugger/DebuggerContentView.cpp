#include "SampleCode.h"
#include "SkOSMenu.h"

#include "DebuggerViews.h"
static const char gIsDebuggerQuery[] = "is-debugger";
class DebuggerView : public SampleView {
public:
        DebuggerView(const char* data, size_t size) {
        fData.append(size, data);
        fCommandsVisible = true;
        fCommandsResizing = false;
        fStateVisible = true;
        fStateResizing = false;
        
        fCommands = new DebuggerCommandsView;
        fCommands->setVisibleP(fCommandsVisible);
        this->attachChildToFront(fCommands)->unref();
        

        fState = new DebuggerStateView;
        fState->setVisibleP(fStateVisible);
        this->attachChildToFront(fState)->unref();
        
        fAtomsToRead = 0;
        fDisplayClip = false;
        
        fDumper = new SkDebugDumper(this->getSinkID(), fCommands->getSinkID(), 
                                    fState->getSinkID());
                                    
        fDumper->unload();
        fAtomBounds.reset();
        fFrameBounds.reset();
        
        SkDumpCanvas* dumpCanvas = new SkDumpCanvas(fDumper);
        SkGPipeReader* dumpReader = new SkGPipeReader(dumpCanvas);
        

        if (size > 0) {
            int offset = 0;
            int frameBound = 0;
            size_t bytesRead;
            while (static_cast<unsigned>(offset) < size) {
                SkGPipeReader::Status s = dumpReader->playback(data + offset, 
                                                               size - offset, 
                                                               &bytesRead, 
                                                               true);
                SkASSERT(SkGPipeReader::kError_Status != s);
                offset += bytesRead;
                
                if (SkGPipeReader::kDone_Status == s) {
                    fDumper->dump(dumpCanvas, SkDumpCanvas::kNULL_Verb, 
                                 "End of Frame", NULL);
                    delete dumpReader;
                    delete dumpCanvas;
                    dumpCanvas = new SkDumpCanvas(fDumper);
                    dumpReader = new SkGPipeReader(dumpCanvas);
                    frameBound = offset;
                }
                fAtomBounds.append(1, &offset);
                fFrameBounds.append(1, &frameBound);
            }
        }
        
        delete dumpReader;
        delete dumpCanvas;
        
        fDumper->load();
    }
    
    ~DebuggerView() {
        fAtomBounds.reset();
        fFrameBounds.reset();
        delete fDumper;
    }
    
    virtual void requestMenu(SkOSMenu* menu) {
        menu->setTitle("Debugger");
        menu->appendSwitch("Show Commands", "Commands", this->getSinkID(), fCommandsVisible);
        menu->appendSwitch("Show State", "State", this->getSinkID(), fStateVisible);
        menu->appendSwitch("Display Clip", "Clip", this->getSinkID(), fDisplayClip);
    }
    
    
    void goToAtom(int atom) {
        if (atom != fAtomsToRead) {
            fAtomsToRead = atom;
            this->inval(NULL);
        }
    }
    
protected:
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Debugger");
            return true;
        }
        if (evt->isType(gIsDebuggerQuery)) {
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual bool onEvent(const SkEvent& evt) {
        if (SkOSMenu::FindSwitchState(evt, "Commands", &fCommandsVisible) ||
            SkOSMenu::FindSwitchState(evt, "State", &fStateVisible)) {
            fCommands->setVisibleP(fCommandsVisible);
            fState->setVisibleP(fStateVisible);
            fStateOffset = (fCommandsVisible) ? fCommands->width() : 0;
            fState->setSize(this->width() - fStateOffset, fState->height());
            fState->setLoc(fStateOffset, this->height() - fState->height());
            this->inval(NULL);
            return true;
        }
        if (SkOSMenu::FindSwitchState(evt, "Clip", &fDisplayClip)) {
            this->inval(NULL);
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        if (fData.count() <= 0)
            return;
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(fStateOffset, 0);
        
        int lastFrameBound = fFrameBounds[fAtomsToRead];
        int toBeRead = fAtomBounds[fAtomsToRead] - lastFrameBound;
        int firstChunk = (fAtomsToRead > 0) ? fAtomBounds[fAtomsToRead - 1] - lastFrameBound: 0;
        if (toBeRead > 0) {
            SkDumpCanvas* dumpCanvas = new SkDumpCanvas(fDumper);
            SkGPipeReader* dumpReader = new SkGPipeReader(dumpCanvas);
            SkGPipeReader* reader = new SkGPipeReader(canvas);
            fDumper->disable();
            
            int offset = 0;
            size_t bytesRead;
            SkGPipeReader::Status s;
            //Read the first chunk
            if (offset < firstChunk && firstChunk < toBeRead) {
                s = dumpReader->playback(fData.begin() + offset, firstChunk - offset, NULL, false);
                SkASSERT(SkGPipeReader::kError_Status != s);
                s = reader->playback(fData.begin() + offset, firstChunk - offset, &bytesRead, false);
                SkASSERT(SkGPipeReader::kError_Status != s);
                if (SkGPipeReader::kDone_Status == s){
                    delete dumpReader;
                    delete dumpCanvas;
                    dumpCanvas = new SkDumpCanvas(fDumper);
                    dumpReader = new SkGPipeReader(dumpCanvas);
                    delete reader;
                    reader = new SkGPipeReader(canvas);
                }
                offset += bytesRead;
            }
            SkASSERT(offset == firstChunk);
            //Then read the current atom
            fDumper->enable();
            s = dumpReader->playback(fData.begin() + offset, toBeRead - offset, NULL, true);
            SkASSERT(SkGPipeReader::kError_Status != s);
            s = reader->playback(fData.begin() + offset, toBeRead - offset, &bytesRead, true);
            SkASSERT(SkGPipeReader::kError_Status != s);

            delete reader;
            delete dumpReader;
            delete dumpCanvas;
            
            if (fDisplayClip) {
                SkPaint p;
                p.setColor(0x440000AA);
                SkPath path;
                canvas->getTotalClip().getBoundaryPath(&path);
                canvas->drawPath(path, p);
            }
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(SkView::Click* click) {
        SkPoint prev = click->fPrev;
        SkPoint curr = click->fCurr;
        bool handled = true;
        switch (click->fState) {
            case SkView::Click::kDown_State:
                if (SkScalarAbs(curr.fX - fCommands->width()) <= SKDEBUGGER_RESIZEBARSIZE) {
                    fCommandsResizing = true;
                }
                else if (SkScalarAbs(curr.fY - (this->height() - fState->height())) <= SKDEBUGGER_RESIZEBARSIZE &&
                         curr.fX > fCommands->width()) {
                    fStateResizing = true;
                }
                else if (curr.fX < fCommands->width()) {
                    fAtomsToRead = fCommands->selectHighlight(
                                                  SkScalarFloorToInt(curr.fY));
                }
                else 
                    handled = false;
                break;
            case SkView::Click::kMoved_State:
                if (fCommandsResizing)
                    fCommands->setSize(curr.fX, this->height());
                else if (fStateResizing)
                    fState->setSize(this->width(), this->height() - curr.fY);
                else if (curr.fX < fCommands->width()) {
                    if (curr.fY - prev.fY < 0) {
                        fCommands->scrollDown();
                    }
                    if (curr.fY - prev.fY > 0) {
                        fCommands->scrollUp();
                    }
                }
                else
                    handled = false;
                break;
            case SkView::Click::kUp_State:
                fStateResizing = fCommandsResizing = false;
                break;
            default:
                break;
        }
        
        fStateOffset = fCommands->width();
        fState->setSize(this->width() - fStateOffset, fState->height());
        fState->setLoc(fStateOffset, this->height() - fState->height());
        if (handled)
            this->inval(NULL);
        return handled;
    }
    
    virtual void onSizeChange() {
        this->INHERITED::onSizeChange();
        fCommands->setSize(CMD_WIDTH, this->height());
        fCommands->setLoc(0, 0);
        fState->setSize(this->width() - CMD_WIDTH, SkFloatToScalar(INFO_HEIGHT));
        fState->setLoc(CMD_WIDTH, this->height() - SkFloatToScalar(INFO_HEIGHT));
    }
    
private:
    DebuggerCommandsView*   fCommands;
    DebuggerStateView*      fState;
    bool                    fCommandsResizing;
    bool                    fCommandsVisible;
    bool                    fStateResizing;
    bool                    fStateVisible;
    float                   fStateOffset;
    bool                    fDisplayClip;
    int                     fAtomsToRead;
    SkTDArray<int>          fAtomBounds;
    SkTDArray<int>          fFrameBounds;
    SkTDArray<char>         fData;
    SkDebugDumper*          fDumper;
    
    typedef SampleView INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

SkView* create_debugger(const char* data, size_t size) {
    return SkNEW_ARGS(DebuggerView, (data, size));
};

bool is_debugger(SkView* view) {
    SkEvent isDebugger(gIsDebuggerQuery);
    return view->doQuery(&isDebugger); 
}
