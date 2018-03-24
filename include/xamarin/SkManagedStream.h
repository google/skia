//
//  SkManagedStream.hpp
//
//  Created by Matthew on 2016/01/08.
//  Copyright © 2016 Xamarin. All rights reserved.
//

#ifndef SkManagedStream_h
#define SkManagedStream_h

#include "SkTypes.h"
#include "SkStream.h"


class SkManagedWStream;
class SkManagedStream;

// delegate declarations
typedef size_t            (*read_delegate)        (SkManagedStream* managedStream, void* buffer, size_t size);
typedef size_t            (*peek_delegate)        (SkManagedStream* managedStream, void* buffer, size_t size);
typedef bool              (*isAtEnd_delegate)     (const SkManagedStream* managedStream);
typedef bool              (*hasPosition_delegate) (const SkManagedStream* managedStream);
typedef bool              (*hasLength_delegate)   (const SkManagedStream* managedStream);
typedef bool              (*rewind_delegate)      (SkManagedStream* managedStream);
typedef size_t            (*getPosition_delegate) (const SkManagedStream* managedStream);
typedef bool              (*seek_delegate)        (SkManagedStream* managedStream, size_t position);
typedef bool              (*move_delegate)        (SkManagedStream* managedStream, long offset);
typedef size_t            (*getLength_delegate)   (const SkManagedStream* managedStream);
typedef SkManagedStream*  (*createNew_delegate)   (const SkManagedStream* managedStream);
typedef void              (*destroy_delegate)     (size_t managedStream);

// delegate declarations
typedef bool     (*write_delegate)          (SkManagedWStream* managedStream, const void* buffer, size_t size);
typedef void     (*flush_delegate)          (SkManagedWStream* managedStream);
typedef size_t   (*bytesWritten_delegate)   (const SkManagedWStream* managedStream);
typedef void     (*wdestroy_delegate)       (size_t managedStream);


// managed stream wrapper
class SkManagedStream : public SkStreamAsset {
public:
    SkManagedStream();
    
    virtual ~SkManagedStream();

    static void setDelegates(const read_delegate pRead,
                             const peek_delegate pPeek,
                             const isAtEnd_delegate pIsAtEnd,
                             const hasPosition_delegate pHasPosition,
                             const hasLength_delegate pHasLength,
                             const rewind_delegate pRewind,
                             const getPosition_delegate pGetPosition,
                             const seek_delegate pSeek,
                             const move_delegate pMove,
                             const getLength_delegate pGetLength,
                             const createNew_delegate pCreateNew,
                             const destroy_delegate pDestroy);
    
    size_t read(void* buffer, size_t size) override;
    bool isAtEnd() const override;
    bool hasPosition() const override;
    bool hasLength() const override;
    
    size_t peek(void* buffer, size_t size) const override;
    
    bool rewind() override;
    SkManagedStream* duplicate() const override;
    
    size_t getPosition() const override;
    bool seek(size_t position) override;
    bool move(long offset) override;
    SkManagedStream* fork() const override;
    
    size_t getLength() const override;
    
private:
    size_t address;

    typedef SkStreamAsset INHERITED;
};


// managed wstream wrapper
class SkManagedWStream : public SkWStream {
public:
    SkManagedWStream();

    virtual ~SkManagedWStream();

    static void setDelegates(const write_delegate pWrite,
                             const flush_delegate pFlush,
                             const bytesWritten_delegate pBytesWritten,
                             const wdestroy_delegate pDestroy);
    
    bool write(const void* buffer, size_t size) override;
    void flush() override;
    size_t bytesWritten() const override;
    
private:
    size_t address;
    
    typedef SkWStream INHERITED;
};


#endif
