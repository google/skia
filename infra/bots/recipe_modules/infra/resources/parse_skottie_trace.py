# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import sys

trace_output = sys.argv[1]
trace_json = json.loads(trace_output)
lottie_filename = sys.argv[2]
output_json_file = sys.argv[3]
seek_trace_name = sys.argv[4]
render_trace_name = sys.argv[5]
expected_dm_frames = int(sys.argv[6])

perf_results = {}
frame_max = 0
frame_min = 0
frame_cumulative = 0
current_frame_duration = 0
total_frames = 0
frame_start = False
for trace in trace_json:
  if seek_trace_name in trace['name']:
    if frame_start:
      raise Exception('We got consecutive Animation::seek without a ' +
                      'render. Something is wrong.')
    frame_start = True
    current_frame_duration = trace['dur']
  elif render_trace_name in trace['name']:
    if not frame_start:
      raise Exception('We got an Animation::render without a seek first. ' +
                      'Something is wrong.')

    current_frame_duration += trace['dur']
    frame_start = False
    total_frames += 1
    frame_max = max(frame_max, current_frame_duration)
    frame_min = (min(frame_min, current_frame_duration)
                 if frame_min else current_frame_duration)
    frame_cumulative += current_frame_duration

expected_dm_frames = expected_dm_frames
if total_frames != expected_dm_frames:
  raise Exception(
      'Got ' + str(total_frames) + ' frames instead of ' +
      str(expected_dm_frames))
perf_results['frame_max_us'] = frame_max
perf_results['frame_min_us'] = frame_min
perf_results['frame_avg_us'] = frame_cumulative/total_frames

# Write perf_results to the output json.
with open(output_json_file, 'w') as f:
  f.write(json.dumps(perf_results))
