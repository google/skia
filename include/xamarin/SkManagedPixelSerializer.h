//
//  SkManagedPixelSerializer.h
//
//  Created by Matthew on 2017/08/18.
//  Copyright © 2017 Xamarin. All rights reserved.
//

#ifndef SkManagedPixelSerializer_h
#define SkManagedPixelSerializer_h

#include "SkTypes.h"
#include "SkPixelSerializer.h"


class SkData;
class SkPixmap;
class SkManagedPixelSerializer;

// delegate declarations
typedef bool    (*use_delegate)    (SkManagedPixelSerializer* managed, const void* data, size_t len);
typedef SkData* (*encode_delegate) (SkManagedPixelSerializer* managed, const SkPixmap& pixmap);

// managed wrapper
class SkManagedPixelSerializer : public SkPixelSerializer {
public:
    SkManagedPixelSerializer();

    static void setDelegates(const use_delegate pUse, const encode_delegate pEncode);

protected:
    bool onUseEncodedData(const void* data, size_t len) override;
    SkData* onEncode(const SkPixmap&) override;

private:
    typedef SkPixelSerializer INHERITED;
};


#endif
