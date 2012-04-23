//
//  SkTLS.h
//  
//
//  Created by Mike Reed on 4/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SkTLS_DEFINED
#define SkTLS_DEFINED

#include "SkTypes.h"

class SkTLS {
public:
    typedef void* (*CreateProc)();
    typedef void  (*DeleteProc)(void*);

    /**
     *  Create proc is called once per thread, and its return value is cached
     *  and returned by this call, treating the proc as a key. When this thread
     *  exists, if DeleteProc is not NULL, it is called and passed the value
     *  that was returned by CreateProc.
     */
    static void* Get(CreateProc, DeleteProc);

    /**
     *  If Get() has previously been called with this CreateProc, then this
     *  returns its cached data, otherwise it returns NULL. The CreateProc is
     *  never invoked in Find, it is only used as a key for searching the
     *  cache.
     */
    static void* Find(CreateProc);
};

#endif
