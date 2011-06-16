#ifndef SkDebugDumper_DEFINED
#define SkDebugDumper_DEFINED
#include "SkDumpCanvasM.h"
#include "SkEvent.h"

class CommandListView;
class InfoPanelView;
class ContentView;
/** Formats the draw commands, and send them to a function-pointer provided
 by the caller.
 */
class SkDebugDumper : public SkDumpCanvasM::Dumper {
public:
    SkDebugDumper(SkEventSinkID cID, SkEventSinkID clID, SkEventSinkID ipID);
    // override from baseclass that does the formatting, and in turn calls
    // the function pointer that was passed to the constructor
    virtual void dump(SkDumpCanvasM*, SkDumpCanvasM::Verb, const char str[],
                      const SkPaint*);
    
    void load() { fInit = true; };
    void unload() { fInit = false; fCount = 0;};
    void disable() { fDisabled = true; };
    void enable() { fDisabled = false; };
private:
    int             fCount;
    bool            fInit;
    bool            fDisabled;
    SkEventSinkID   fContentID;
    SkEventSinkID   fCommandListID;
    SkEventSinkID   fInfoPanelID;
    
    typedef SkDumpCanvasM::Dumper INHERITED;
};
#endif