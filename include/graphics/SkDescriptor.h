/* include/graphics/SkDescriptor.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkDescriptor_DEFINED
#define SkDescriptor_DEFINED

#include "SkTypes.h"

class SkDescriptor {
public:
    static size_t ComputeOverhead(int entryCount)
    {
        SkASSERT(entryCount >= 0);
        return sizeof(SkDescriptor) + entryCount * sizeof(Entry);
    }

    static SkDescriptor* Alloc(size_t length)
    {
        SkASSERT(SkAlign4(length) == length);
        SkDescriptor* desc = (SkDescriptor*)sk_malloc_throw(length);
        return desc;
    }

    static void Free(SkDescriptor* desc)
    {
        sk_free(desc);
    }

    void init()
    {
        fLength = sizeof(SkDescriptor);
        fCount  = 0;
    }

    U32 getLength() const { return fLength; }

    void* addEntry(U32 tag, U32 length, const void* data = nil)
    {
        SkASSERT(tag);
        SkASSERT(SkAlign4(length) == length);
        SkASSERT(this->findEntry(tag, nil) == nil);

        Entry*  entry = (Entry*)((char*)this + fLength);
        entry->fTag = tag;
        entry->fLen = length;
        if (data)
            memcpy(entry + 1, data, length);

        fCount += 1;
        fLength += sizeof(Entry) + length;
        return (entry + 1); // return its data
    }

    void computeChecksum()
    {
        fChecksum = SkDescriptor::ComputeChecksum(this);
    }

#ifdef SK_DEBUG
    void assertChecksum() const
    {
        SkASSERT(fChecksum == SkDescriptor::ComputeChecksum(this));
    }
#endif

    const void* findEntry(U32 tag, U32* length) const
    {
        const Entry* entry = (const Entry*)(this + 1);
        int          count = fCount;

        while (--count >= 0)
        {
            if (entry->fTag == tag)
            {
                if (length)
                    *length = entry->fLen;
                return entry + 1;
            }
            entry = (const Entry*)((const char*)(entry + 1) + entry->fLen);
        }
        return nil;
    }

    SkDescriptor* copy() const
    {
        SkDescriptor* desc = SkDescriptor::Alloc(fLength);
        memcpy(desc, this, fLength);
        return desc;
    }

    friend bool operator==(const SkDescriptor& a, const SkDescriptor& b)
    {
        return  a.fChecksum == b.fChecksum &&
                a.fLength == b.fLength &&
                // this assumes that fCount is the beginning of the rest of the descriptor
                // (after fCheckSum and fLength)
                memcmp(&a.fCount, &b.fCount, a.fLength - 2*sizeof(U32)) == 0;
    }

    struct Entry {
        U32 fTag;
        U32 fLen;
    };

#ifdef SK_DEBUG
    U32 getChecksum() const { return fChecksum; }
    U32 getCount() const { return fCount; }
#endif

private:
    U32 fChecksum;  // must be first
    U32 fLength;    // must be second
    U32 fCount;

    static U32 ComputeChecksum(const SkDescriptor* desc)
    {
        const U32*  ptr = (const U32*)desc + 1; // skip the checksum field
        const U32*  stop = (const U32*)((const char*)desc + desc->fLength);
        U32         sum = 0;

        SkASSERT(ptr < stop);
        do {
            sum = (sum << 1) | (sum >> 31);
            sum += *ptr++;
        } while (ptr < stop);

        return sum;
    }
};

#include "SkScalerContext.h"

class SkAutoDescriptor {
public:
    SkAutoDescriptor(size_t size)
    {
        if (size <= kStorageSize)
            fDesc = fStorage;
        else
            fDesc = SkDescriptor::Alloc(size);
    }
    ~SkAutoDescriptor()
    {
        if (fDesc != fStorage)
            SkDescriptor::Free(fDesc);
    }
    SkDescriptor* getDesc() const { return fDesc; }
private:
    enum {
        kStorageSize =  sizeof(SkDescriptor)
                        + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContext::Rec)    // for rec
                        + sizeof(SkDescriptor::Entry) + sizeof(void*)                   // for typeface
                        + 32   // slop for occational small extras
    };
    SkDescriptor*   fDesc;
    union {
        uint32_t     fStorage32[(kStorageSize + 3) >> 2];
        SkDescriptor fStorage[1];
    };
};


#endif

