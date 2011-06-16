#include "SkDebuggerViews.h"
#include <stdio.h>

SkContentView::SkContentView(SkEventSinkID clID, SkEventSinkID ipID) : 
                         fDumper(this->getSinkID(), clID, ipID) {
    fBGColor = 0xFFDDDDDD;
    fAtomsToRead = 0;
    fDisplayClip = false;
}

SkContentView::~SkContentView() {
    fAtomBounds.clear();
    fFrameBounds.clear();
}

void SkContentView::reinit(const char* filename) {
    fFilePath.set(filename);
    fAtomsToRead = 0;
    this->init();
}

bool SkContentView::onEvent(const SkEvent& evt) {
    return this->INHERITED::onEvent(evt);
}

//Read file atom by atom and record attom bounds
void SkContentView::init() {
    fDumper.unload();
    fAtomBounds.clear();
    fFrameBounds.clear();
    
    SkDumpCanvasM* dumpCanvas = new SkDumpCanvasM(&fDumper);
    SkGPipeReader* dumpReader = new SkGPipeReader(dumpCanvas);
    
    FILE* f = fopen(fFilePath.c_str(), "rb");
    SkASSERT(f != NULL);
    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f) * sizeof(char);
    fseek(f, 0, SEEK_SET);
    if (fileSize > 0) {
        char* block = (char*)sk_malloc_throw(fileSize);
        fread(block, 1, fileSize, f);
        int offset = 0;
        int frameBound = 0;
        size_t bytesRead;
        while (offset < fileSize) {
            SkGPipeReader::Status s = dumpReader->playback(block + offset, 
                                                           fileSize - offset, 
                                                           &bytesRead, true);
            SkASSERT(SkGPipeReader::kError_Status != s);
            offset += bytesRead;
            if (SkGPipeReader::kDone_Status == s) {
                fDumper.dump(dumpCanvas,SkDumpCanvasM::kNULL_Verb, 
                             "End of Frame", NULL);
                delete dumpReader;
                delete dumpCanvas;
                dumpCanvas = new SkDumpCanvasM(&fDumper);
                dumpReader = new SkGPipeReader(dumpCanvas);
                frameBound = offset;
            }
            fAtomBounds.push_back(offset);
            fFrameBounds.push_back(frameBound);
        }
        sk_free(block);
    }

    fclose(f);
    
    delete dumpReader;
    delete dumpCanvas;
    
    fDumper.load();
}

void SkContentView::goToAtom(int atom) {
    if (atom != fAtomsToRead) {
        fAtomsToRead = atom;
        this->inval(NULL);
    }
}

void SkContentView::toggleClip() {
    fDisplayClip = !fDisplayClip;
    this->inval(NULL);
}

void SkContentView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
    
    SkAutoCanvasRestore acr(canvas, true);

    int lastFrameBound = fFrameBounds[fAtomsToRead];
    int toBeRead = fAtomBounds[fAtomsToRead] - lastFrameBound;
    int firstChunk = (fAtomsToRead > 0) ? fAtomBounds[fAtomsToRead - 1] -
                                          lastFrameBound: 0;
    if (toBeRead > 0) {
        SkDumpCanvasM* dumpCanvas = new SkDumpCanvasM(&fDumper);
        SkGPipeReader* dumpReader = new SkGPipeReader(dumpCanvas);
        SkGPipeReader* reader = new SkGPipeReader(canvas);
        fDumper.disable();
        
        FILE* f = fopen(fFilePath.c_str(), "rb");
        SkASSERT(f != NULL);
        fseek(f, lastFrameBound, SEEK_SET);
        char* block = (char*)sk_malloc_throw(toBeRead);
        fread(block, 1, toBeRead, f);
        int offset = 0;
        size_t bytesRead;
        SkGPipeReader::Status s;
        //Read the first chunk
        if (offset < firstChunk && firstChunk < toBeRead) {
            s = dumpReader->playback(block + offset, firstChunk - offset, NULL, false);
            SkASSERT(SkGPipeReader::kError_Status != s);
            s = reader->playback(block + offset, firstChunk - offset, &bytesRead, false);
            SkASSERT(SkGPipeReader::kError_Status != s);
            if (SkGPipeReader::kDone_Status == s){
                delete dumpReader;
                delete dumpCanvas;
                dumpCanvas = new SkDumpCanvasM(&fDumper);
                dumpReader = new SkGPipeReader(dumpCanvas);
                delete reader;
                reader = new SkGPipeReader(canvas);
            }
            offset += bytesRead;
        }
        SkASSERT(offset == firstChunk);
        //Then read the current atom
        fDumper.enable();
        s = dumpReader->playback(block + offset, toBeRead - offset, NULL, true);
        SkASSERT(SkGPipeReader::kError_Status != s);
        s = reader->playback(block + offset, toBeRead - offset, &bytesRead, true);
        SkASSERT(SkGPipeReader::kError_Status != s);

        sk_free(block);
        fclose(f);
        
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
    this->INHERITED::onDraw(canvas);
}