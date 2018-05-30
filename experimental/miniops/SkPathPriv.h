#ifndef SkPathPriv_DEFINED
#define SkPathPriv_DEFINED

class SkPathPriv {
public:
    enum FirstDirection {
        kCW_FirstDirection,         // == SkPath::kCW_Direction
        kCCW_FirstDirection,        // == SkPath::kCCW_Direction
        kUnknown_FirstDirection,
    };

    static bool CheapComputeFirstDirection(const SkPath&, FirstDirection* dir);
};

#endif
