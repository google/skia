#ifndef SkArenaAlloc_DEFINED
#define SkArenaAlloc_DEFINED

class SkArenaAlloc {
public:
    enum Tracking {kDontTrack, kTrack};
    SkArenaAlloc(char* block, size_t size, size_t, Tracking tracking = kDontTrack);

    SkArenaAlloc(size_t extraSize, Tracking tracking = kDontTrack)
        : SkArenaAlloc(nullptr, 0, extraSize, tracking)
    {}

    ~SkArenaAlloc();

    template <typename T, typename... Args>
    T* make(Args&&... args);
    template <typename T>
    T* makeArrayDefault(size_t count);
};

template <size_t InlineStorageSize>
class SkSTArenaAlloc : public SkArenaAlloc {
public:
    explicit SkSTArenaAlloc(size_t extraSize = InlineStorageSize, Tracking tracking = kDontTrack)
        : INHERITED(fInlineStorage, InlineStorageSize, extraSize, tracking) {}

private:
    char fInlineStorage[InlineStorageSize];

    using INHERITED = SkArenaAlloc;
};


#endif

