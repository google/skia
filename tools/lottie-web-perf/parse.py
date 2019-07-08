#!/usr/bin/env python
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import sys

trace_output = sys.argv[1]
output_json_file = sys.argv[2]

# trace_json = json.loads(trace_output)
with open(trace_output, 'r') as f:
  trace_json = json.load(f)

frame_max = 0
frame_min = 0
frame_cumulative = 0
total_frames = 0
frame_id_to_start_ts = {}
for trace in trace_json['traceEvents']:
  if 'PipelineReporter' in trace['name']:
    frame_id = trace['id']
    args = trace.get('args')
    if args and args.get('step') == 'BeginImplFrameToSendBeginMainFrame':
      frame_id_to_start_ts[frame_id] = trace['ts']
    elif args and (args.get('termination_status') == 'missed_frame' or
                   args.get('termination_status') == 'submitted_frame'):
      current_frame_duration = trace['ts'] - frame_id_to_start_ts[frame_id]
      total_frames += 1
      frame_max = max(frame_max, current_frame_duration)
      frame_min = (min(frame_min, current_frame_duration)
                   if frame_min else current_frame_duration)
      frame_cumulative += current_frame_duration
      # We are done with this frame_id so remove it from the dict.
      frame_id_to_start_ts.pop(frame_id)
      # Print stuff out.
      print '%d (%s with %s): %d' % (
          total_frames, frame_id, args['termination_status'],
          current_frame_duration)
    elif args and (args.get('termination_status') == 'replaced_by_new_reporter_at_same_stage' or
                 args.get('termination_status') == 'did_not_produce_frame' or
                 args.get('termination_status') == 'main_frame_aborted'):
      # Invalidate all previously collected results for this frame_id.
      if frame_id_to_start_ts.get(frame_id):
        print 'Invalidating %s with %s' % (frame_id, args['termination_status'])
        frame_id_to_start_ts.pop(frame_id)

perf_results = {}
perf_results['frame_max_us'] = frame_max
perf_results['frame_min_us'] = frame_min
perf_results['frame_avg_us'] = frame_cumulative/total_frames
print 'For %d frames got: %s' % (total_frames, perf_results)

# Write perf_results to the output json.
with open(output_json_file, 'w') as f:
  f.write(json.dumps(perf_results))
