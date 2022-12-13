/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/CombinationBuilder.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/shaders/SkShaderBase.h"

using namespace skgpu::graphite;

#define SHADER_TYPES(M)             \
    M(SolidColor)                   \
                                    \
    M(LinearGradient)               \
    M(RadialGradient)               \
    M(SweepGradient)                \
    M(ConicalGradient)              \
                                    \
    M(LocalMatrix)                  \
    M(Image)                        \
    M(PorterDuffBlendShader)        \
    M(BlendShader)

// To keep the SHADER_TYPES list and SkShaderType aligned, we create a hidden enum class
// and check it against SkShaderType
#define MAKE_ENUM(T) k##T,
enum class UnusedShaderType { SHADER_TYPES(MAKE_ENUM) };
#undef MAKE_ENUM
static constexpr int kUnusedShaderTypeCount = static_cast<int>(UnusedShaderType::kBlendShader) + 1;

#define STATIC_ASSERT(T) static_assert((int) ShaderType::k##T == (int) UnusedShaderType::k##T);
   SHADER_TYPES(STATIC_ASSERT)
#undef STATIC_ASSERT

static_assert(kShaderTypeCount == kUnusedShaderTypeCount);

//--------------------------------------------------------------------------------------------------
namespace {

int tm_pair_to_index(TileModePair tileModes) {
    static_assert(kSkTileModeCount <= 4);

    return (int) tileModes.fX << 2 | (int) tileModes.fY;
}

#ifdef SK_DEBUG
TileModePair index_to_tm_pair(int index) {
    static_assert(kSkTileModeCount <= 4);
    SkASSERT(index < 16);

    int tmX = index >> 2;
    int tmY = index & 3;

    return { (SkTileMode) tmX, (SkTileMode) tmY };
}

void add_indent(int indent) {
    SkDebugf("%*c", indent, ' ');
}

const char* type_to_str(ShaderType type) {
    switch (type) {
#define CASE(T) case ShaderType::k##T: return #T;
        SHADER_TYPES(CASE)
#undef CASE
    }

    SkUNREACHABLE;
}

#endif // SK_DEBUG

} // anonymous namespace

//--------------------------------------------------------------------------------------------------

namespace skgpu::graphite {

// The base class for all the objects stored in the arena
// The base option object consists of some number of child slots each of which is an array of
// options. The slots and their associated linked list of options are allocated separately in
// the arena.
class Option {
public:
    // The Slot holds the combinatorial options for a single child slot of an
    // option (in a linked list).
    class Slot {
    public:
        void addOption(Option* newOption);

        int numCombinations() const;

        void addToKey(const KeyContext&, int desiredCombination, PaintParamsKeyBuilder*);

#ifdef SK_DEBUG
        void dump(int indent) const;
#endif

    private:
        Option* fHead = nullptr;
        Option* fTail = nullptr;
    };

    Option(ShaderType type, int numSlots)
            : fType(type)
            , fNumSlots(numSlots) {}

    ShaderType type() const { return fType; }
    int numSlots() const { return fNumSlots; }

#ifdef SK_DEBUG
    int epoch() const { return fEpoch; }
    void setEpoch(int epoch) {
        SkASSERT(fEpoch == kInvalidEpoch && epoch != kInvalidEpoch);
        fEpoch = epoch;
    }
#endif

    void setSlotsArray(Slot* slots) {
        SkASSERT(!fSlots);
        fSlots = slots;
    }

    Slot* getSlot(int slotIndex) {
        if (slotIndex >= fNumSlots) {
            return nullptr;
        }

        return &fSlots[slotIndex];
    }

    void addOption(int slotIndex, Option* newOption) {
        if (slotIndex >= fNumSlots) {
            return;
        }

        Slot* slot = this->getSlot(slotIndex);
        slot->addOption(newOption);
    }

    int numChildCombinations() const {
        int numChildCombinations = 1;
        for (int i = 0; i < fNumSlots; ++i) {
            numChildCombinations *= fSlots[i].numCombinations();
        }
        return numChildCombinations;
    }

    int numCombinations() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* keyBuilder) {
        SkASSERT(desiredCombination < this->numCombinations());

        int intrinsicCombination = desiredCombination / this->numChildCombinations();
        int childCombination = desiredCombination % this->numChildCombinations();

        this->beginBlock(keyContext, intrinsicCombination, keyBuilder);

        if (fNumSlots) {
            int numCombinationsSeen = 0;
            for (int slotIndex = 0; slotIndex < fNumSlots; ++slotIndex) {
                Slot* slot = this->getSlot(slotIndex);

                numCombinationsSeen += slot->numCombinations();
                int numCombosLeft = this->numChildCombinations() / numCombinationsSeen;

                int slotCombination;
                if (slotIndex+1 < fNumSlots) {
                    slotCombination = childCombination / (numCombosLeft ? numCombosLeft : 1);
                    childCombination %= numCombosLeft;
                } else {
                    slotCombination = childCombination;
                }

                slot->addToKey(keyContext, slotCombination, keyBuilder);
            }
        }

        keyBuilder->endBlock();
    }

#ifdef SK_DEBUG
    void dump(int indent = 0) const {
        SkDebugf("%s", type_to_str(fType));
        if (!this->numIntrinsicCombinations()) {
            SkDebugf("(%d)", this->numIntrinsicCombinations());
        }
        SkDebugf(" { %d, %d }", this->numCombinations(), this->numChildCombinations());
        if (this->numSlots()) {
            SkDebugf("\n");
        }
        int childIndex = 0;
        for (int i = 0; i < fNumSlots; ++i) {
            add_indent(indent+4);
            SkDebugf("%d: ", childIndex);
            fSlots[i].dump(indent+4);
            ++childIndex;
        }
    }
#endif

private:
    int numIntrinsicCombinations() const;
    void beginBlock(const KeyContext&, int intrinsicCombination, PaintParamsKeyBuilder*) const;

    SkDEBUGCODE(static constexpr int kInvalidEpoch = -1;)

    const ShaderType fType;
    const int        fNumSlots;
    SkDEBUGCODE(int  fEpoch = kInvalidEpoch;)
    Option*          fNext = nullptr;
    Slot*            fSlots = nullptr;  // an array of 'fNumSlots' Slots
};

void Option::Slot::addOption(Option* newOption) {
    SkASSERT(newOption->fNext == nullptr);

    if (fHead == nullptr) {
        SkASSERT(fTail == nullptr);
        fHead = fTail = newOption;
    } else {
        SkASSERT(fTail->fNext == nullptr);
        fTail->fNext = newOption;
        fTail = newOption;
    }
}

int Option::Slot::numCombinations() const {
    int numCombinations = 0;

    for (Option* option = fHead; option; option = option->fNext) {
        numCombinations += option->numCombinations();
    }

    return numCombinations;
}

void Option::Slot::addToKey(const KeyContext& keyContext,
                            int desiredCombination,
                            PaintParamsKeyBuilder* keyBuilder) {
    SkASSERT(desiredCombination < this->numCombinations());

    for (Option* option = fHead; option; option = option->fNext) {
        if (desiredCombination < option->numCombinations()) {
            option->addToKey(keyContext, desiredCombination, keyBuilder);
            return;
        }

        desiredCombination -= option->numCombinations();
    }
}

#ifdef SK_DEBUG
void Option::Slot::dump(int indent) const {
    SkDebugf("{ %d } ", this->numCombinations());

    for (const Option* option = fHead; option; option = option->fNext) {
        option->dump(indent);
        SkDebugf(" | ");
    }
    SkDebugf("\n");
}
#endif

#define CREATE_ARENA_OBJECT(T, numChildSlots, ...)                                                 \
struct ArenaData_##T : public Option {                                                             \
    static const ShaderType kType = ShaderType::k##T;                                              \
    static const int kNumChildSlots = numChildSlots;                                               \
    int numIntrinsicCombinationsDerived() const;                                                   \
    void beginBlock(const KeyContext&, int intrinsicCombination, PaintParamsKeyBuilder*) const;    \
    __VA_ARGS__                                                                                    \
};

CREATE_ARENA_OBJECT(SolidColor,      /* numChildSlots */ 0)
int ArenaData_SolidColor::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_SolidColor::beginBlock(const KeyContext& keyContext,
                                      int intrinsicCombination,
                                      PaintParamsKeyBuilder* builder) const {
    constexpr SkPMColor4f kUnusedColor = { 1, 0, 0, 1 };

    SkASSERT(intrinsicCombination == 0);
    SolidColorShaderBlock::BeginBlock(keyContext, builder, /*gatherer=*/nullptr, kUnusedColor);
}

CREATE_ARENA_OBJECT(LinearGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_LinearGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_LinearGradient::beginBlock(const KeyContext& keyContext,
                                          int intrinsicCombination,
                                          PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                     { SkShaderBase::GradientType::kLinear,
                                       fMinNumStops + intrinsicCombination });
}

CREATE_ARENA_OBJECT(RadialGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_RadialGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_RadialGradient::beginBlock(const KeyContext& keyContext,
                                          int intrinsicCombination,
                                          PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                     { SkShaderBase::GradientType::kRadial,
                                       fMinNumStops + intrinsicCombination });
}

CREATE_ARENA_OBJECT(SweepGradient,   /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_SweepGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_SweepGradient::beginBlock(const KeyContext& keyContext,
                                         int intrinsicCombination,
                                         PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                     { SkShaderBase::GradientType::kSweep,
                                       fMinNumStops + intrinsicCombination });
}

CREATE_ARENA_OBJECT(ConicalGradient, /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_ConicalGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_ConicalGradient::beginBlock(const KeyContext& keyContext,
                                           int intrinsicCombination,
                                           PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                     { SkShaderBase::GradientType::kConical,
                                       fMinNumStops + intrinsicCombination });
}

CREATE_ARENA_OBJECT(LocalMatrix,     /* numChildSlots */ 1)
int ArenaData_LocalMatrix::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_LocalMatrix::beginBlock(const KeyContext& keyContext,
                                       int intrinsicCombination,
                                       PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination == 0);

    LocalMatrixShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */nullptr,
                                       /* lmShaderData= */ nullptr);
}

// Split out due to constructor work
struct ArenaData_Image : public Option {
    static const ShaderType kType = ShaderType::kImage;
    static const int kNumChildSlots = 0;

    ArenaData_Image(const Option& init, SkSpan<TileModePair> tileModePairs)
            : Option(init)
            , fTileModeCombos(0) {
        for (auto tmPair : tileModePairs) {
            int index = tm_pair_to_index(tmPair);

#ifdef SK_DEBUG
            SkASSERT(index < 32);
            TileModePair tmp = index_to_tm_pair(index);
            SkASSERT(tmp == tmPair);
#endif

            fTileModeCombos |= 0x1 << index;
        }
    }

    int numIntrinsicCombinationsDerived() const;
    void beginBlock(const KeyContext&, int intrinsicCombination, PaintParamsKeyBuilder*) const;

    int32_t fTileModeCombos;
};
int ArenaData_Image::numIntrinsicCombinationsDerived() const {
    return SkPopCount(fTileModeCombos);
}
void ArenaData_Image::beginBlock(const KeyContext& keyContext,
                                 int intrinsicCombination,
                                 PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    ImageShaderBlock::BeginBlock(keyContext, builder,
                                 /* gatherer= */ nullptr, /* imgData= */ nullptr);
}

CREATE_ARENA_OBJECT(PorterDuffBlendShader, /* numChildSlots */ 2)
int ArenaData_PorterDuffBlendShader::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_PorterDuffBlendShader::beginBlock(const KeyContext& keyContext,
                                                 int intrinsicCombination,
                                                 PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination == 0);

    PorterDuffBlendShaderBlock::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                           {});  // the porter-duff constant is unused
}

CREATE_ARENA_OBJECT(BlendShader, /* numChildSlots */ 2)
int ArenaData_BlendShader::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_BlendShader::beginBlock(const KeyContext& keyContext,
                                       int intrinsicCombination,
                                       PaintParamsKeyBuilder* builder) const {
    SkASSERT(intrinsicCombination == 0);

    BlendShaderBlock::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                 {SkBlendMode::kSrc});  // the blend mode is unused
}

// Here to access the derived ArenaData objects
int Option::numIntrinsicCombinations() const {
    int numIntrinsicCombinations;

#define CASE(T)                                                                             \
    case ShaderType::k##T:                                                                  \
        numIntrinsicCombinations =                                                          \
                static_cast<const ArenaData_##T*>(this)->numIntrinsicCombinationsDerived(); \
        break;

    switch (this->type()) {
        SHADER_TYPES(CASE)
    }

#undef CASE

    SkASSERT(numIntrinsicCombinations >= 1); // There is always, at least, the existential combo
    return numIntrinsicCombinations;
}

// Here to access the derived ArenaData objects
void Option::beginBlock(const KeyContext& keyContext,
                        int intrinsicCombination,
                        PaintParamsKeyBuilder* builder) const {
#define CASE(T)                                                                   \
    case ShaderType::k##T:                                                        \
        static_cast<const ArenaData_##T*>(this)->beginBlock(keyContext,           \
                                                            intrinsicCombination, \
                                                            builder);             \
        break;

    switch (this->type()) {
        SHADER_TYPES(CASE)
    }

#undef CASE
}

//--------------------------------------------------------------------------------------------------
CombinationOption CombinationOption::addChildOption(int childIndex, ShaderType type) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return CombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    Option* child = fBuilder->addOptionInternal(type);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return CombinationOption(fBuilder, child);
}

CombinationOption CombinationOption::addChildOption(int childIndex, ShaderType type,
                                                    int minNumStops, int maxNumStops) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return CombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    Option* child = fBuilder->addOptionInternal(type, minNumStops, maxNumStops);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return CombinationOption(fBuilder, child);
}

CombinationOption CombinationOption::addChildOption(
        int childIndex,
        ShaderType type,
        SkSpan<TileModePair> tileModes) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return CombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    Option* child = fBuilder->addOptionInternal(type, tileModes);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return CombinationOption(fBuilder, child);
}

ShaderType CombinationOption::type() const { return fDataInArena->type(); }
int CombinationOption::numChildSlots() const { return fDataInArena->numSlots(); }
SkDEBUGCODE(int CombinationOption::epoch() const { return fDataInArena->epoch(); })

//--------------------------------------------------------------------------------------------------
CombinationBuilder::CombinationBuilder(ShaderCodeDictionary* dict)
        : fDictionary(dict) {
    fArena = std::make_unique<SkArenaAllocWithReset>(64);
    this->reset();
}

CombinationBuilder::~CombinationBuilder() = default;


template<typename T, typename... Args>
Option* CombinationBuilder::allocInArena(Args&&... args) {
    Option* arenaObject = fArena->make<T>(T{{ T::kType, T::kNumChildSlots },
                                            std::forward<Args>(args)... });
    if (!arenaObject) {
        return nullptr;
    }

    SkASSERT(arenaObject->type() == T::kType);
    SkASSERT(arenaObject->numSlots() == T::kNumChildSlots);

    SkDEBUGCODE(arenaObject->setEpoch(fEpoch));

    if (T::kNumChildSlots) {
        arenaObject->setSlotsArray(fArena->makeArrayDefault<Option::Slot>(T::kNumChildSlots));
    }

    return arenaObject;
}

void CombinationBuilder::addOption(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fBlendModes |= (0x1 << (int) bm);
}

void CombinationBuilder::addOption(SkBlendMode rangeStart, SkBlendMode rangeEnd) {
    for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
        this->addOption((SkBlendMode) i);
    }
}

void CombinationBuilder::addOption(BlendModeGroup group) {
    switch (group) {
        case BlendModeGroup::kPorterDuff:
            this->addOption(SkBlendMode::kClear, SkBlendMode::kScreen);
            break;
        case BlendModeGroup::kAdvanced:
            this->addOption(SkBlendMode::kOverlay, SkBlendMode::kMultiply);
            break;
        case BlendModeGroup::kColorAware:
            this->addOption(SkBlendMode::kHue, SkBlendMode::kLuminosity);
            break;
        case BlendModeGroup::kAll:
            this->addOption(SkBlendMode::kClear, SkBlendMode::kLastMode);
            break;
    }
}

void CombinationBuilder::addOption(BlenderID id) {
    SkASSERT(fDictionary->isValidID(id.asUInt()));

    fBlenders.add(id);
}

Option* CombinationBuilder::addOptionInternal(ShaderType shaderType) {

    // TODO: Can we use the X macro trick here to collapse this
    switch (shaderType) {
        case ShaderType::kSolidColor:
            return this->allocInArena<ArenaData_SolidColor>();
        case ShaderType::kLocalMatrix:
            return this->allocInArena<ArenaData_LocalMatrix>();
        case ShaderType::kPorterDuffBlendShader:
            return this->allocInArena<ArenaData_PorterDuffBlendShader>();
        case ShaderType::kBlendShader:
            return this->allocInArena<ArenaData_BlendShader>();
        default:
            return nullptr;
    }
}

Option* CombinationBuilder::addOptionInternal(ShaderType shaderType,
                                              int minNumStops, int maxNumStops) {

    // TODO: Can we use the X macro trick here to collapse this
    switch (shaderType) {
        case ShaderType::kLinearGradient:
            return this->allocInArena<ArenaData_LinearGradient>(minNumStops, maxNumStops);
        case ShaderType::kRadialGradient:
            return this->allocInArena<ArenaData_RadialGradient>(minNumStops, maxNumStops);
        case ShaderType::kSweepGradient:
            return this->allocInArena<ArenaData_SweepGradient>(minNumStops, maxNumStops);
        case ShaderType::kConicalGradient:
            return this->allocInArena<ArenaData_ConicalGradient>(minNumStops, maxNumStops);
        default:
            return nullptr;
    }
}

Option* CombinationBuilder::addOptionInternal(ShaderType shaderType,
                                              SkSpan<TileModePair> tileModes) {

    // TODO: Can we use the X macro trick here to collapse this
    switch (shaderType) {
        case ShaderType::kImage:
            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return nullptr;
    }
}

CombinationOption CombinationBuilder::addOption(ShaderType shaderType) {

    Option* newOption = this->addOptionInternal(shaderType);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

CombinationOption CombinationBuilder::addOption(ShaderType shaderType,
                                                int minNumStops, int maxNumStops) {

    Option* newOption = this->addOptionInternal(shaderType, minNumStops, maxNumStops);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

CombinationOption CombinationBuilder::addOption(ShaderType shaderType,
                                                SkSpan<TileModePair> tileModes) {

    Option* newOption = this->addOptionInternal(shaderType, tileModes);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

void CombinationBuilder::reset() {
    fShaderOptions.clear();
    fBlendModes = 0;
    fBlenders.reset();
    fArena->reset();
    SkDEBUGCODE(++fEpoch;)
}

int CombinationBuilder::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (Option* s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    // If no shader option is specified the builder will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int CombinationBuilder::numBlendModeCombinations() const {
    int numBlendModeCombinations = SkPopCount(fBlendModes) + fBlenders.count();

    // If no blend mode options are specified the builder will add kSrcOver as an option
    return numBlendModeCombinations ? numBlendModeCombinations : 1;
}

#ifdef SK_DEBUG
void CombinationBuilder::dump() const {
    for (Option* s : fShaderOptions) {
        s->dump();
        SkDebugf("\n");
    }
}
#endif

void CombinationBuilder::createKey(const KeyContext& keyContext,
                                   int desiredCombination,
                                   PaintParamsKeyBuilder* keyBuilder) {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    int numBlendModeCombos = this->numBlendModeCombinations();

    int desiredShaderCombination = desiredCombination / numBlendModeCombos;
    int desiredBlendCombination = desiredCombination % numBlendModeCombos;

    // Keys begin with solid color shaders to assign the paint's color, so add this to the key.
    keyBuilder->beginBlock(BuiltInCodeSnippetID::kSolidColorShader);
    keyBuilder->endBlock();

    // TODO: Once the ColorFilterID class is implemented, include the alpha color filtering step in
    // the combination builder's key assembly process.

    for (Option* shaderOption : fShaderOptions) {
        if (desiredShaderCombination < shaderOption->numCombinations()) {
            shaderOption->addToKey(keyContext, desiredShaderCombination, keyBuilder);
            break;
        }

        desiredShaderCombination -= shaderOption->numCombinations();
    }

    if (desiredBlendCombination < SkPopCount(fBlendModes)) {
        int ith_set_bit = SkNthSet(fBlendModes, desiredBlendCombination);

        SkASSERT(ith_set_bit < kSkBlendModeCount);
        SkBlendMode bm = (SkBlendMode) ith_set_bit;

        BlendModeBlock::BeginBlock(keyContext, keyBuilder, /*gatherer=*/nullptr, bm); // bm is used!
        keyBuilder->endBlock();
    } else {
        // TODO: need to handle fBlenders here
    }

}

void CombinationBuilder::buildCombinations(
        ShaderCodeDictionary* dict,
        const std::function<void(SkUniquePaintParamsID)>& func) {
    KeyContext keyContext(dict);
    PaintParamsKeyBuilder builder(dict);

    // Supply a default kSrcOver if no other blend mode option is provided
    if (fBlendModes == 0 && fBlenders.empty()) {
        this->addOption(SkBlendMode::kSrcOver);
    }

    // Supply a default solid color shader if no other shader option is provided
    if (fShaderOptions.empty()) {
        this->addOption(ShaderType::kSolidColor);
    }

    int numCombos = this->numCombinations();
    for (int i = 0; i < numCombos; ++i) {
        this->createKey(keyContext, i, &builder);

        auto entry = dict->findOrCreate(&builder);

        func(entry->uniqueID());
    }
}

} // namespace skgpu::graphite
