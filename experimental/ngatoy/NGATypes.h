// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef NGATypes_DEFINED
#define NGATypes_DEFINED

class ID {
public:
    explicit ID(int id) : fID(id) {
//        SkASSERT(id != -1;)
    }

    static ID Invalid() {
        return ID(-1);
    }

    bool operator==(ID other) const { return fID == other.fID; }

private:
    int fID;
};

class PaintersOrder {
public:
    PaintersOrder() : fPaintersOrder(0) {}

    static PaintersOrder Next() {
        return PaintersOrder(fCounter++);
    }

    static PaintersOrder Peek() {
        return PaintersOrder(fCounter);
    }

    static PaintersOrder Invalid() {
        return PaintersOrder(0);
    }

    bool isValid() const { return fPaintersOrder != 0; }

private:
    static uint32_t fCounter;

    explicit PaintersOrder(int paintersOrder) : fPaintersOrder(paintersOrder) {}
    uint32_t fPaintersOrder;
};


#endif


