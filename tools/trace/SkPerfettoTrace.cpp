/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/trace/SkPerfettoTrace.h"

#include <fcntl.h>
#include <fstream>
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTraceEventCommon.h"
#include "tools/flags/CommandLineFlags.h"

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

static DEFINE_string(perfettoOutputDir, "./",
                     "Output directory for perfetto trace file(s).\n"
                     "Note: not the name of the file itself.\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_string(perfettoOutputFileName, "trace",
                     "Output file name (excluding path and file extension) for the perfetto trace"
                     "file.\nNote: When splitting trace files by benchmark (see "
                     "--splitPerfettoTracesByBenchmark), file name will be determined by the "
                     "benchmark name.\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_string(perfettoOutputFileExtension, ".perfetto-trace",
                     "Output file extension for perfetto trace file(s).\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_bool(longPerfettoTrace, false,
                   "Perfetto within Skia is optimized for tracing performance of 'smaller' traces"
                   "(~10 seconds or less). Set this flag to true to optimize for longer tracing"
                   "sessions.\n"
                   "Will only have an effect if perfetto tracing is enabled. See --trace.");

SkPerfettoTrace::SkPerfettoTrace() {
    fOutputPath = FLAGS_perfettoOutputDir[0];
    fOutputFileExtension = FLAGS_perfettoOutputFileExtension[0];
    this->openNewTracingSession(FLAGS_perfettoOutputFileName[0]);
}

SkPerfettoTrace::~SkPerfettoTrace() {
    this->closeTracingSession();
}

void SkPerfettoTrace::openNewTracingSession(const std::string& baseFileName) {
    perfetto::TracingInitArgs args;
    /* Store the current tracing session's output file path as a member attribute so it can
     * be referenced when closing a tracing session (needed for short traces where writing to
     * the output file occurs at the end of all tracing). */
    fCurrentSessionFullOutputPath = fOutputPath + baseFileName + fOutputFileExtension;

    /* Enable using only the in-process backend (recording only within the app itself). This is as
     * opposed to additionally including perfetto::kSystemBackend, which uses a Perfetto daemon. */
    args.backends |= perfetto::kInProcessBackend;

    if (FLAGS_longPerfettoTrace) {
        /* Set the shared memory buffer size higher than the default of 256 KB to
        reduce trace writer packet loss occurrences associated with larger traces. */
        args.shmem_size_hint_kb = 2000;
    }
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();

    // Set up event tracing configuration.
    perfetto::protos::gen::TrackEventConfig track_event_cfg;
    perfetto::TraceConfig cfg;

    /* Set the central memory buffer size - will record up to this amount of data. */
    cfg.add_buffers()->set_size_kb(32000);

    if (FLAGS_longPerfettoTrace) {
        /* Enable continuous file writing/"streaming mode" to output trace data throughout the
         * program instead of one large dump at the end. */
        cfg.set_write_into_file(true);
        /* If set to a value other than the default, set how often trace data gets written to the
         * output file. */
        cfg.set_file_write_period_ms(5000);
        /* Force periodic commitment of shared memory buffer pages to the central buffer.
         * Helps prevent out-of-order event slices with long traces. */
        cfg.set_flush_period_ms(10000);
    }

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");
    ds_cfg->set_track_event_config_raw(track_event_cfg.SerializeAsString());

    // Begin a tracing session.
    tracingSession = perfetto::Tracing::NewTrace();
    if (FLAGS_longPerfettoTrace) {
        fd = open(fCurrentSessionFullOutputPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
        tracingSession->Setup(cfg, fd);
    } else {
        tracingSession->Setup(cfg);
    }
    tracingSession->StartBlocking();
}

void SkPerfettoTrace::closeTracingSession() {
    perfetto::TrackEvent::Flush();
    tracingSession->StopBlocking();
    if (!FLAGS_longPerfettoTrace) {
        std::vector<char> trace_data(tracingSession->ReadTraceBlocking());
        std::ofstream output;
        output.open(fCurrentSessionFullOutputPath, std::ios::out | std::ios::binary);
        output.write(&trace_data[0], trace_data.size());
        output.close();
    } else {
        close(fd);
    }
}

SkEventTracer::Handle SkPerfettoTrace::addTraceEvent(char phase,
                                                     const uint8_t* categoryEnabledFlag,
                                                     const char* name,
                                                     uint64_t id,
                                                     int numArgs,
                                                     const char** argNames,
                                                     const uint8_t* argTypes,
                                                     const uint64_t* argValues,
                                                     uint8_t flags) {
    perfetto::DynamicCategory category{ this->getCategoryGroupName(categoryEnabledFlag) };
    if (TRACE_EVENT_PHASE_COMPLETE == phase ||
        TRACE_EVENT_PHASE_INSTANT == phase) {
        switch (numArgs) {
            case 0: {
                this->triggerTraceEvent(categoryEnabledFlag, name);
                break;
            }
            case 1: {
                this->triggerTraceEvent(categoryEnabledFlag, name, argNames[0], argTypes[0],
                                        argValues[0]);
                break;
            }
            case 2: {
                this->triggerTraceEvent(categoryEnabledFlag, name, argNames[0], argTypes[0],
                                        argValues[0], argNames[1], argTypes[1], argValues[1]);
                break;
            }
        }
    } else if (TRACE_EVENT_PHASE_END == phase) {
        TRACE_EVENT_END(category);
    }

    if (TRACE_EVENT_PHASE_INSTANT == phase) {
        TRACE_EVENT_END(category);
    }
    return 0;
}

void SkPerfettoTrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                               const char* name,
                                               SkEventTracer::Handle handle) {
    // This is only ever called from a scoped trace event, so we will just end the event.
    perfetto::DynamicCategory category{ this->getCategoryGroupName(categoryEnabledFlag) };
    TRACE_EVENT_END(category);
}

const uint8_t* SkPerfettoTrace::getCategoryGroupEnabled(const char* name) {
    return fCategories.getCategoryGroupEnabled(name);
}

const char* SkPerfettoTrace::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    return fCategories.getCategoryGroupName(categoryEnabledFlag);
}

void SkPerfettoTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                                        const char* eventName) {
    perfetto::DynamicCategory category{ this->getCategoryGroupName(categoryEnabledFlag) };
    TRACE_EVENT_BEGIN(category, nullptr, [&](perfetto::EventContext ctx) {
        ctx.event()->set_name(eventName);
    });
}

void SkPerfettoTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName,
                                        const char* arg1Name, const uint8_t& arg1Type,
                                        const uint64_t& arg1Val) {
    perfetto::DynamicCategory category{ this->getCategoryGroupName(categoryEnabledFlag) };

    switch (arg1Type) {
        case TRACE_VALUE_TYPE_BOOL: {
            TRACE_EVENT_BEGIN(category, nullptr, arg1Name, SkToBool(arg1Val),
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        case TRACE_VALUE_TYPE_UINT: {
            TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val,
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        case TRACE_VALUE_TYPE_INT: {
            TRACE_EVENT_BEGIN(category, nullptr, arg1Name, static_cast<int64_t>(arg1Val),
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        case TRACE_VALUE_TYPE_DOUBLE: {
            TRACE_EVENT_BEGIN(category, nullptr, arg1Name, sk_bit_cast<double>(arg1Val),
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        case TRACE_VALUE_TYPE_POINTER: {
            TRACE_EVENT_BEGIN(category, nullptr,
                              arg1Name, skia_private::TraceValueAsPointer(arg1Val),
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]]; // Perfetto always copies string data
        case TRACE_VALUE_TYPE_STRING: {
            TRACE_EVENT_BEGIN(category, nullptr,
                              arg1Name, skia_private::TraceValueAsString(arg1Val),
                              [&](perfetto::EventContext ctx) {
                              ctx.event()->set_name(eventName); });
            break;
        }
        default: {
            SkUNREACHABLE;
        }
    }
}

namespace {
/* Define a template to help handle all the possible TRACE_EVENT_BEGIN macro call
 * combinations with 2 arguments of all the types supported by SetTraceValue.
 */
template <typename T>
void begin_event_with_second_arg(const char * categoryName, const char* eventName,
                                 const char* arg1Name, T arg1Val, const char* arg2Name,
                                 const uint8_t& arg2Type, const uint64_t& arg2Val) {
      perfetto::DynamicCategory category{categoryName};

      switch (arg2Type) {
          case TRACE_VALUE_TYPE_BOOL: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, SkToBool(arg2Val),
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          case TRACE_VALUE_TYPE_UINT: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, arg2Val,
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          case TRACE_VALUE_TYPE_INT: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val,
                                arg2Name, static_cast<int64_t>(arg2Val),
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          case TRACE_VALUE_TYPE_DOUBLE: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val,
                                arg2Name, sk_bit_cast<double>(arg2Val),
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          case TRACE_VALUE_TYPE_POINTER: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val,
                                arg2Name, skia_private::TraceValueAsPointer(arg2Val),
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]];
          case TRACE_VALUE_TYPE_STRING: {
              TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val,
                                arg2Name, skia_private::TraceValueAsString(arg2Val),
                                [&](perfetto::EventContext ctx) {
                                ctx.event()->set_name(eventName); });
              break;
          }
          default: {
              SkUNREACHABLE;
              break;
          }
      }
}
} // anonymous namespace

void SkPerfettoTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                                        const char* eventName, const char* arg1Name,
                                        const uint8_t& arg1Type, const uint64_t& arg1Val,
                                        const char* arg2Name, const uint8_t& arg2Type,
                                        const uint64_t& arg2Val) {

    const char * category{ this->getCategoryGroupName(categoryEnabledFlag) };

    switch (arg1Type) {
        case TRACE_VALUE_TYPE_BOOL: {
            begin_event_with_second_arg(category, eventName, arg1Name, SkToBool(arg1Val),
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        case TRACE_VALUE_TYPE_UINT: {
            begin_event_with_second_arg(category, eventName, arg1Name, arg1Val,
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        case TRACE_VALUE_TYPE_INT: {
            begin_event_with_second_arg(category, eventName,
                                        arg1Name, static_cast<int64_t>(arg1Val),
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        case TRACE_VALUE_TYPE_DOUBLE: {
            begin_event_with_second_arg(category, eventName, arg1Name, sk_bit_cast<double>(arg1Val),
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        case TRACE_VALUE_TYPE_POINTER: {
            begin_event_with_second_arg(category, eventName,
                                        arg1Name, skia_private::TraceValueAsPointer(arg1Val),
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]];
        case TRACE_VALUE_TYPE_STRING: {
            begin_event_with_second_arg(category, eventName,
                                        arg1Name, skia_private::TraceValueAsString(arg1Val),
                                        arg2Name, arg2Type, arg2Val);
            break;
        }
        default: {
            SkUNREACHABLE;
        }
    }
}

void SkPerfettoTrace::newTracingSection(const char* name) {
    if (perfetto::Tracing::IsInitialized()) {
        this->closeTracingSession();
    }
    this->openNewTracingSession(name);
}
