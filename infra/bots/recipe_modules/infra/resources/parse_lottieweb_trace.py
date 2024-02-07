# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import sys

trace_output = sys.argv[1]
with open(trace_output, 'r') as f:
  trace_json = json.load(f)
output_json_file = sys.argv[2]
renderer = sys.argv[3]  # Unused for now but might be useful in the future.

# Output data about the GPU that was used.
print('GPU data:')
print(trace_json['metadata'].get('gpu-gl-renderer'))
print(trace_json['metadata'].get('gpu-driver'))
print(trace_json['metadata'].get('gpu-gl-vendor'))

erroneous_termination_statuses = [
    'replaced_by_new_reporter_at_same_stage',
    'did_not_produce_frame',
]
accepted_termination_statuses = [
    'missed_frame',
    'submitted_frame',
    'main_frame_aborted'
]

current_frame_duration = 0
total_frames = 0
frame_id_to_start_ts = {}
# Will contain tuples of frame_ids and their duration and status.
completed_frame_id_and_duration_status = []
# Will contain tuples of drawn frame_ids and their duration.
drawn_frame_id_and_duration = []
for trace in trace_json['traceEvents']:
  if 'PipelineReporter' in trace['name']:
    frame_id = trace['id']
    args = trace.get('args')
    if args and args.get('step') == 'BeginImplFrameToSendBeginMainFrame':
      frame_id_to_start_ts[frame_id] = trace['ts']
    elif args and (args.get('termination_status') in
                   accepted_termination_statuses):
      if not frame_id_to_start_ts.get(frame_id):
        print('[No start ts found for %s]' % frame_id)
        continue
      current_frame_duration = trace['ts'] - frame_id_to_start_ts[frame_id]
      total_frames += 1
      completed_frame_id_and_duration_status.append(
          (frame_id, current_frame_duration, args['termination_status']))
      if(args['termination_status'] == 'missed_frame' or
       args['termination_status'] == 'submitted_frame'):
        drawn_frame_id_and_duration.append((frame_id, current_frame_duration))

      # We are done with this frame_id so remove it from the dict.
      frame_id_to_start_ts.pop(frame_id)
      print('%d (%s with %s): %d' % (
          total_frames, frame_id, args['termination_status'],
          current_frame_duration))
    elif args and (args.get('termination_status') in
                   erroneous_termination_statuses):
      # Invalidate previously collected results for this frame_id.
      if frame_id_to_start_ts.get(frame_id):
        print('[Invalidating %s due to %s]' % (
            frame_id, args['termination_status']))
        frame_id_to_start_ts.pop(frame_id)

# Calculate metrics for total completed frames.
total_completed_frames = len(completed_frame_id_and_duration_status)
if total_completed_frames < 25:
  raise Exception('Even with 3 loops found only %d frames' %
                  total_completed_frames)
# Get frame avg/min/max for the middle 25 frames.
start = (total_completed_frames - 25) // 2
print('Got %d total completed frames. Using indexes [%d, %d).' % (
    total_completed_frames, start, start+25))
frame_max = 0
frame_min = 0
frame_cumulative = 0
aborted_frames = 0
for frame_id, duration, status in (
    completed_frame_id_and_duration_status[start:start+25]):
  frame_max = max(frame_max, duration)
  frame_min = min(frame_min, duration) if frame_min else duration
  frame_cumulative += duration
  if status == 'main_frame_aborted':
    aborted_frames += 1

perf_results = {}
perf_results['frame_max_us'] = frame_max
perf_results['frame_min_us'] = frame_min
perf_results['frame_avg_us'] = frame_cumulative/25
perf_results['aborted_frames'] = aborted_frames

# Now calculate metrics for only drawn frames.
drawn_frame_max = 0
drawn_frame_min = 0
drawn_frame_cumulative = 0
total_drawn_frames = len(drawn_frame_id_and_duration)
if total_drawn_frames < 25:
  raise Exception('Even with 3 loops found only %d drawn frames' %
                  total_drawn_frames)
# Get drawn frame avg/min/max from the middle 25 frames.
start = (total_drawn_frames - 25) // 2
print('Got %d total drawn frames. Using indexes [%d-%d).' % (
      total_drawn_frames, start, start+25))
for frame_id, duration in drawn_frame_id_and_duration[start:start+25]:
  drawn_frame_max = max(drawn_frame_max, duration)
  drawn_frame_min = (min(drawn_frame_min, duration)
                     if drawn_frame_min else duration)
  drawn_frame_cumulative += duration
# Add metrics to perf_results.
perf_results['drawn_frame_max_us'] = drawn_frame_max
perf_results['drawn_frame_min_us'] = drawn_frame_min
perf_results['drawn_frame_avg_us'] = drawn_frame_cumulative/25

print('Final perf_results dict: %s' % perf_results)

# Write perf_results to the output json.
with open(output_json_file, 'w') as f:
  f.write(json.dumps(perf_results))
