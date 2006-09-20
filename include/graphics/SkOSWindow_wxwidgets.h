/*
 *  SkOSWindow_wxwidgets.h
 *  wxwidgets
 *
 *  Created by phanna on 12/14/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SkOSWindow_wxwidgets_DEFINED
#define SkOSWindow_wxwidgets_DEFINED

#include "SkWindow.h"
#include "wx/frame.h"

class SkOSWindow: public SkWindow
{
public:
	SkOSWindow();
    SkOSWindow(const wxString& title, int x, int y, int width, int height);
    ~SkOSWindow();
	
	wxFrame* getWXFrame() const { return fFrame; }
	
	void updateSize();
	
protected:
    virtual void onHandleInval(const SkRect16&);
    virtual void onAddMenu(const SkOSMenu*);
    
private:
    wxFrame* fFrame;
    typedef SkWindow INHERITED;
    
};

#endif