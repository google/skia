/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkContourMeasure.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRSXform.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkCurve.h"
#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkReflected.h"
#include "src/core/SkMakeUnique.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"

void SkParticleBinding::visitFields(SkFieldVisitor* v) {
    v->visit("Name", fName);
}

class SkParticleExternalValue : public SkSL::ExternalValue {
public:
    SkParticleExternalValue(const char* name, SkSL::Compiler& compiler, const SkSL::Type& type)
        : INHERITED(name, type)
        , fCompiler(compiler)
        , fRandom(nullptr) {
    }

    void setRandom(SkRandom* random) { fRandom = random; }

protected:
    SkSL::Compiler& fCompiler;
    SkRandom* fRandom;
    typedef SkSL::ExternalValue INHERITED;
};

// Exposes an SkCurve as an external, callable value. c(x) returns a float.
class SkCurveExternalValue : public SkParticleExternalValue {
public:
    SkCurveExternalValue(const char* name, SkSL::Compiler& compiler, const SkCurve& curve)
        : INHERITED(name, compiler, *compiler.context().fFloat_Type)
        , fCurve(curve) { }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        *outReturn = fCurve.eval(*arguments, fRandom[index]);
    }

private:
    SkCurve fCurve;
    typedef SkParticleExternalValue INHERITED;
};

class SkCurveBinding : public SkParticleBinding {
public:
    SkCurveBinding(const char* name = "", const SkCurve& curve = 0.0f)
        : SkParticleBinding(name)
        , fCurve(curve) {}

    REFLECTED(SkCurveBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("Curve", fCurve);
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
                new SkCurveExternalValue(fName.c_str(), compiler, fCurve));
    }

private:
    SkCurve fCurve;
};

// Exposes an SkColorCurve as an external, callable value. c(x) returns a float4.
class SkColorCurveExternalValue : public SkParticleExternalValue {
public:
    SkColorCurveExternalValue(const char* name, SkSL::Compiler& compiler, const SkColorCurve& curve)
        : INHERITED(name, compiler, *compiler.context().fFloat4_Type)
        , fCurve(curve) {
    }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        SkColor4f color = fCurve.eval(*arguments, fRandom[index]);
        memcpy(outReturn, color.vec(), 4 * sizeof(float));
    }

private:
    SkColorCurve fCurve;
    typedef SkParticleExternalValue INHERITED;
};

class SkColorCurveBinding : public SkParticleBinding {
public:
    SkColorCurveBinding(const char* name = "",
                        const SkColorCurve& curve = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f })
        : SkParticleBinding(name)
        , fCurve(curve) {
    }

    REFLECTED(SkColorCurveBinding, SkParticleBinding)

        void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("Curve", fCurve);
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkColorCurveExternalValue(fName.c_str(), compiler, fCurve));
    }

private:
    SkColorCurve fCurve;
};

struct SkPathContours {
    SkScalar fTotalLength;
    SkTArray<sk_sp<SkContourMeasure>> fContours;

    void reset() {
        fTotalLength = 0;
        fContours.reset();
    }
};

// Exposes an SkPath as an external, callable value. p(x) returns a float4 { pos.xy, normal.xy }
class SkPathExternalValue : public SkParticleExternalValue {
public:
    SkPathExternalValue(const char* name, SkSL::Compiler& compiler, const SkPathContours* path)
        : INHERITED(name, compiler, *compiler.context().fFloat4_Type)
        , fPath(path) { }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        SkScalar len = fPath->fTotalLength * arguments[0];
        int idx = 0;
        while (idx < fPath->fContours.count() && len > fPath->fContours[idx]->length()) {
            len -= fPath->fContours[idx++]->length();
        }
        SkVector localXAxis;
        if (!fPath->fContours[idx]->getPosTan(len, (SkPoint*)outReturn, &localXAxis)) {
            outReturn[0] = outReturn[1] = 0.0f;
            localXAxis = { 1, 0 };
        }
        outReturn[2] = localXAxis.fY;
        outReturn[3] = -localXAxis.fX;
    }

private:
    const SkPathContours* fPath;
    typedef SkParticleExternalValue INHERITED;
};

class SkPathBinding : public SkParticleBinding {
public:
    SkPathBinding(const char* name = "", const char* path = "")
            : SkParticleBinding(name)
            , fPath(path) {
        this->rebuild();
    }

    REFLECTED(SkPathBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkString oldPath = fPath;

        SkParticleBinding::visitFields(v);
        v->visit("Path", fPath);

        if (fPath != oldPath) {
            this->rebuild();
        }
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

private:
    SkString fPath;
    void rebuild() {
        SkPath path;
        if (!SkParsePath::FromSVGString(fPath.c_str(), &path)) {
            return;
        }

        fContours.reset();

        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.fContours.push_back(contour);
            fContours.fTotalLength += contour->length();
        }
    }

    // Cached
    SkPathContours fContours;
};

class SkTextBinding : public SkParticleBinding {
public:
    SkTextBinding(const char* name = "", const char* text = "", SkScalar fontSize = 96)
            : SkParticleBinding(name)
            , fText(text)
            , fFontSize(fontSize) {
        this->rebuild();
    }

    REFLECTED(SkTextBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkString oldText = fText;
        SkScalar oldSize = fFontSize;

        SkParticleBinding::visitFields(v);
        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);

        if (fText != oldText || fFontSize != oldSize) {
            this->rebuild();
        }
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

private:
    SkString fText;
    SkScalar fFontSize;
    void rebuild() {
        if (fText.isEmpty()) {
            return;
        }

        fContours.reset();

        SkFont font(nullptr, fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), SkTextEncoding::kUTF8, 0, 0, font, &path);
        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.fContours.push_back(contour);
            fContours.fTotalLength += contour->length();
        }
    }

    // Cached
    SkPathContours fContours;
};

void SkParticleBinding::RegisterBindingTypes() {
    REGISTER_REFLECTED(SkParticleBinding);
    REGISTER_REFLECTED(SkCurveBinding);
    REGISTER_REFLECTED(SkColorCurveBinding);
    REGISTER_REFLECTED(SkPathBinding);
    REGISTER_REFLECTED(SkTextBinding);
}

// Exposes a particle's random generator as an external, readable value. read returns a float [0, 1)
class SkRandomExternalValue : public SkParticleExternalValue {
public:
    SkRandomExternalValue(const char* name, SkSL::Compiler& compiler)
        : INHERITED(name, compiler, *compiler.context().fFloat_Type) {}

    bool canRead() const override { return true; }
    void read(int index, float* target) override {
        *target = fRandom[index].nextF();
    }

private:
    typedef SkParticleExternalValue INHERITED;
};

static const char* kDefaultCode =
R"(// float rand; Every read returns a random float [0 .. 1)
layout(ctype=float) in uniform float dt;
layout(ctype=float) in uniform float effectAge;

struct Particle {
  float  age;
  float  lifetime;
  float2 pos;
  float2 dir;
  float  scale;
  float2 vel;
  float  spin;
  float4 color;
  float  frame;
};

void spawn(inout Particle p) {
}

void update(inout Particle p) {
}
)";

SkParticleEffectParams::SkParticleEffectParams()
        : fMaxCount(128)
        , fEffectDuration(1.0f)
        , fRate(8.0f)
        , fDrawable(nullptr)
        , fCode(kDefaultCode) {
    this->rebuild();
}

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    SkString oldCode = fCode;

    v->visit("MaxCount", fMaxCount);
    v->visit("Duration", fEffectDuration);
    v->visit("Rate", fRate);

    v->visit("Drawable", fDrawable);

    v->visit("Code", fCode);

    v->visit("Bindings", fBindings);

    // TODO: Or, if any change to binding metadata?
    if (fCode != oldCode) {
        this->rebuild();
    }
}

void SkParticleEffectParams::rebuild() {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;

    SkTArray<std::unique_ptr<SkParticleExternalValue>> externalValues;

    auto rand = skstd::make_unique<SkRandomExternalValue>("rand", compiler);
    compiler.registerExternalValue(rand.get());
    externalValues.push_back(std::move(rand));

    for (const auto& binding : fBindings) {
        if (binding) {
            auto value = binding->toValue(compiler);
            compiler.registerExternalValue(value.get());
            externalValues.push_back(std::move(value));
        }
    }

    auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                            SkSL::String(fCode.c_str()), settings);
    if (!program) {
        SkDebugf("%s\n", compiler.errorText().c_str());
        return;
    }

    auto byteCode = compiler.toByteCode(*program);
    if (!byteCode) {
        SkDebugf("%s\n", compiler.errorText().c_str());
        return;
    }

    fByteCode = std::move(byteCode);
    fExternalValues.swap(externalValues);
}

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random)
        : fParams(std::move(params))
        , fRandom(random)
        , fLooping(false)
        , fSpawnTime(-1.0)
        , fCount(0)
        , fLastTime(-1.0)
        , fSpawnRemainder(0.0f) {
    this->setCapacity(fParams->fMaxCount);
}

void SkParticleEffect::start(double now, bool looping) {
    fCount = 0;
    fLastTime = fSpawnTime = now;
    fSpawnRemainder = 0.0f;
    fLooping = looping;
}

void SkParticleEffect::update(double now) {
    if (!this->isAlive() || !fParams->fDrawable) {
        return;
    }

    float deltaTime = static_cast<float>(now - fLastTime);
    if (deltaTime <= 0.0f) {
        return;
    }
    fLastTime = now;

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    float effectAge = static_cast<float>((now - fSpawnTime) / fParams->fEffectDuration);
    effectAge = fLooping ? fmodf(effectAge, 1.0f) : SkTPin(effectAge, 0.0f, 1.0f);

    float updateParams[2] = { deltaTime, effectAge };

    // Advance age for existing particles, and remove any that have reached their end of life
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kAge][i] +=
                fParticles.fData[SkParticles::kLifetime][i] * deltaTime;
        if (fParticles.fData[SkParticles::kAge][i] > 1.0f) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            for (int j = 0; j < SkParticles::kNumChannels; ++j) {
                fParticles.fData[j][i] = fParticles.fData[j][fCount - 1];
            }
            fStableRandoms[i] = fStableRandoms[fCount - 1];
            --i;
            --fCount;
        }
    }

    auto runProgram = [](const SkParticleEffectParams* params, const char* entry,
                         SkParticles& particles, float updateParams[], int start, int count) {
        if (const auto& byteCode = params->fByteCode) {
            float* args[SkParticles::kNumChannels];
            for (int i = 0; i < SkParticles::kNumChannels; ++i) {
                args[i] = particles.fData[i].get() + start;
            }
            SkRandom* randomBase = particles.fRandom.get() + start;
            for (const auto& value : params->fExternalValues) {
                value->setRandom(randomBase);
            }
            SkAssertResult(byteCode->runStriped(byteCode->getFunction(entry),
                                                args, SkParticles::kNumChannels, count,
                                                updateParams, 2, nullptr, 0));
        }
    };

    // Spawn new particles
    float desired = fParams->fRate * deltaTime + fSpawnRemainder;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
    if (numToSpawn) {
        const int spawnBase = fCount;

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our SkRandom so each particle definitely gets a different generator
            fRandom.nextU();
            fParticles.fData[SkParticles::kAge            ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kLifetime       ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kPositionX      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kPositionY      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kHeadingX       ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kHeadingY       ][fCount] = -1.0f;
            fParticles.fData[SkParticles::kScale          ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kVelocityX      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kVelocityY      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kVelocityAngular][fCount] = 0.0f;
            fParticles.fData[SkParticles::kColorR         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorG         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorB         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorA         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kSpriteFrame    ][fCount] = 0.0f;
            fParticles.fRandom[fCount] = fRandom;
            fCount++;
        }

        // Run the spawn script
        runProgram(fParams.get(), "spawn", fParticles, updateParams, spawnBase, numToSpawn);

        // Now stash copies of the random generators and compute inverse particle lifetimes
        // (so that subsequent updates are faster)
        for (int i = spawnBase; i < fCount; ++i) {
            fParticles.fData[SkParticles::kLifetime][i] =
                    sk_ieee_float_divide(1.0f, fParticles.fData[SkParticles::kLifetime][i]);
            fStableRandoms[i] = fParticles.fRandom[i];
        }
    }

    // Restore all stable random generators so update affectors get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles.fRandom[i] = fStableRandoms[i];
    }

    // Run the update script
    runProgram(fParams.get(), "update", fParticles, updateParams, 0, fCount);

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kPositionX][i] +=
                fParticles.fData[SkParticles::kVelocityX][i] * deltaTime;
        fParticles.fData[SkParticles::kPositionY][i] +=
                fParticles.fData[SkParticles::kVelocityY][i] * deltaTime;

        SkScalar s = SkScalarSin(fParticles.fData[SkParticles::kVelocityAngular][i] * deltaTime),
                 c = SkScalarCos(fParticles.fData[SkParticles::kVelocityAngular][i] * deltaTime);
        float oldHeadingX = fParticles.fData[SkParticles::kHeadingX][i],
              oldHeadingY = fParticles.fData[SkParticles::kHeadingY][i];
        fParticles.fData[SkParticles::kHeadingX][i] = oldHeadingX * c - oldHeadingY * s;
        fParticles.fData[SkParticles::kHeadingY][i] = oldHeadingX * s + oldHeadingY * c;
    }

    // Mark effect as dead if we've reached the end (and are not looping)
    if (!fLooping && (now - fSpawnTime) > fParams->fEffectDuration) {
        fSpawnTime = -1.0;
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive() && fParams->fDrawable) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        fParams->fDrawable->draw(canvas, fParticles, fCount, &paint);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    for (int i = 0; i < SkParticles::kNumChannels; ++i) {
        fParticles.fData[i].realloc(capacity);
    }
    fParticles.fRandom.realloc(capacity);
    fStableRandoms.realloc(capacity);

    fCapacity = capacity;
    fCount = SkTMin(fCount, fCapacity);
}
