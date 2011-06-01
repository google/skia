#include "SkView.h"
#include "SkColor.h"
#include "SkBitmap.h"

/*
 * Pipe Reader with File IO. This view reads from the data file produced by the 
 * Pipe Writer. 
 */

//Make sure to change this to a valid path that matches FILE_PATH in sampleapp
#define FILE_PATH "/path/to/drawing.data"

class ReaderView : public SkView {
public:
    ReaderView();
    virtual void draw(SkCanvas* canvas);
    
private:
    int     fFilePos;
    int     fFront;
    int     fBack;
    SkColor fBGColor;
    SkBitmap fBufferBitmaps[2];
    typedef SkView INHERITED;
};

