/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTargetCommands_DEFINED
#define GrTargetCommands_DEFINED

#include "GrDrawTarget.h"
#include "GrPath.h"
#include "GrPendingProgramElement.h"
#include "GrPrimitiveProcessor.h"
#include "GrRenderTarget.h"
#include "GrTRecorder.h"

#include "batches/GrBatch.h"

#include "SkRect.h"

class GrResourceProvider;
class GrBatchFlushState;

// TODO: Convert all commands into GrBatch and remove this class.
class GrTargetCommands : ::SkNoncopyable {
public:
    GrTargetCommands() : fCmdBuffer(kCmdBufferInitialSizeInBytes), fLastFlushToken(0) {}

    class Cmd : ::SkNoncopyable {
    public:
        enum CmdType {
            kDrawBatch_CmdType         = 4,
        };

        Cmd(CmdType type)
            : fType(type)
#if GR_BATCH_SPEW
            , fUniqueID(GenID(&gUniqueID))
#endif
        {}
        virtual ~Cmd() {}

        virtual void execute(GrBatchFlushState*) = 0;

        CmdType type() const { return fType; }

        GrBATCH_SPEW(uint32_t uniqueID() const { return fUniqueID;} )

    private:
        // TODO move this to a common header so it can be shared with GrBatch
        static uint32_t GenID(int32_t* idCounter) {
            uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
            if (!id) {
                SkFAIL("This should never wrap\n");
            }
            return id;
        }
        CmdType          fType;
        GrBATCH_SPEW(uint32_t fUniqueID);
        GrBATCH_SPEW(static int32_t gUniqueID;)
    };

    void reset();
    void flush(GrGpu*, GrResourceProvider*);

private:
    friend class GrCommandBuilder;
    friend class GrBufferedDrawTarget; // This goes away when State becomes just a pipeline
    friend class GrReorderCommandBuilder;

    typedef GrGpu::DrawArgs DrawArgs;

    struct DrawBatch : public Cmd {
        DrawBatch(GrBatch* batch)
            : Cmd(kDrawBatch_CmdType)
            , fBatch(SkRef(batch)){
            SkASSERT(!batch->isUsed());
        }

        GrBatch* batch() { return fBatch; }
        void execute(GrBatchFlushState*) override;

    private:
        SkAutoTUnref<GrBatch>   fBatch;
    };

    static const int kCmdBufferInitialSizeInBytes = 8 * 1024;

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    CmdBuffer* cmdBuffer() { return &fCmdBuffer; }

    CmdBuffer                           fCmdBuffer;
    GrBatchToken                        fLastFlushToken;
};

#endif
