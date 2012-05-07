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

/**
 *  Maintains a per-thread cache, using a CreateProc as the key into that cache.
 */
class SkTLS {
public:
    typedef void* (*CreateProc)();
    typedef void  (*DeleteProc)(void*);
    
    /**
     *  If Get() has previously been called with this CreateProc, then this
     *  returns its cached data, otherwise it returns NULL. The CreateProc is
     *  never invoked in Find, it is only used as a key for searching the
     *  cache.
     */
    static void* Find(CreateProc);
    
    /**
     *  Return the cached data that was returned by the CreateProc. This proc
     *  is only called the first time Get is called, and there after it is
     *  cached (per-thread), using the CreateProc as a key to look it up.
     *
     *  When this thread, or Delete is called, the cached data is removed, and
     *  if a DeleteProc was specified, it is passed the pointer to the cached
     *  data.
     */
    static void* Get(CreateProc, DeleteProc);

    /**
     *  Remove (optionally calling the DeleteProc if it was specificed in Get)
     *  the cached data associated with this CreateProc. If no associated cached
     *  data is found, do nothing.
     */
    static void Delete(CreateProc);

private:
    // Our implementation requires only 1 TLS slot, as we manage multiple values
    // ourselves in a list, with the platform specific value as our head.

    /**
     *  implemented by the platform, to return the value of our (one) slot per-thread
     */
    static void* PlatformGetSpecific();

    /**
     *  implemented by the platform, to set the value for our (one) slot per-thread
     */
    static void  PlatformSetSpecific(void*);

public:
    /**
     *  Will delete our internal list. To be called by the platform if/when its
     *  TLS slot is deleted (often at thread shutdown).
     *
     *  Public *only* for the platform's use, not to be called by a client.
     */
    static void Destructor(void* ptr);
};

#endif
