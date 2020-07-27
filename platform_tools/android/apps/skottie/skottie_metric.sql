SELECT RUN_METRIC('android/android_hwui_metric.sql');

CREATE VIEW dequeue_buffer AS
SELECT
  count(*) as dequeue_buffer_count,
  max(dur) as dequeue_buffer_max,
  min(dur) as dequeue_buffer_min,
  avg(dur) as dequeue_buffer_avg,
  sum(dur) as dequeue_buffer_sum,
  thread_track.utid as render_thread_id
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
WHERE slice.name='dequeueBuffer' AND slice.dur >= 0
GROUP BY thread_track.utid;

-- limit test results starting from second frame to 7 seconds after that
CREATE VIEW test_start_ts AS
SELECT
  ts as test_start,
  ts + 7000000000 as test_end,
  thread.upid as process_upid
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.utid = thread_track.utid)
WHERE slice.name='DrawFrame' AND slice.dur >= 0
ORDER BY slice.ts
LIMIT 1 OFFSET 1;
-- increase the offset if want to start from more than second frame

CREATE VIEW startup_time AS
SELECT
  min(ts) as first_draw_frame,
  test_start_ts.test_start as second_draw_frame,
  test_start_ts.test_start - min(ts) as start_time,
  thread_track.utid as render_thread_id
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.utid = thread_track.utid)
INNER JOIN test_start_ts ON (test_start_ts.process_upid = thread.upid)
WHERE slice.name='setSurface' AND slice.dur >= 0
GROUP BY thread_track.utid;

CREATE VIEW hwui_draw_frame2 AS
SELECT
  count(*) as draw_frame_count,
  max(dur) as draw_frame_max,
  min(dur) as draw_frame_min,
  avg(dur) as draw_frame_avg,
  thread_track.utid as render_thread_id
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.utid = thread_track.utid)
INNER JOIN test_start_ts ON (slice.ts >= test_start_ts.test_start AND slice.ts <= test_start_ts.test_end)
WHERE slice.name='DrawFrame' AND slice.dur >= 0
GROUP BY thread_track.utid;

CREATE VIEW skottie_animator AS
SELECT
  count(*) as skottie_animator_count,
  max(dur) as skottie_animator_max,
  min(dur) as skottie_animator_min,
  avg(dur) as skottie_animator_avg,
  sum(dur) as skottie_animator_sum,
  thread.upid as process_upid
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.name='SkottieAnimator' AND thread.utid = thread_track.utid)
INNER JOIN test_start_ts ON (slice.ts >= test_start_ts.test_start AND slice.ts <= test_start_ts.test_end)
WHERE slice.name='Choreographer#doFrame' AND slice.dur >= 0
GROUP BY thread_track.utid;

CREATE VIEW total_cpu_time AS
SELECT
  sum(sched_slice.dur) as cpu_time,
  thread.upid as process_upid
FROM sched_slice
INNER JOIN process ON (process.upid = thread.upid)
INNER JOIN thread ON (
                      (thread.name LIKE 'hwuiTask%' OR thread.name=substr(process.name,-15) OR thread.name LIKE '%skottie' OR thread.name='RenderThread' OR thread.name='SkottieAnimator')
                      AND thread.utid = sched_slice.utid
                     )
INNER JOIN test_start_ts ON (sched_slice.ts >= test_start_ts.test_start AND sched_slice.ts <= test_start_ts.test_end)
WHERE sched_slice.dur >= 0
GROUP BY thread.upid;

CREATE VIEW cpu_time_rt_hwui_tasks AS
SELECT
  sum(sched_slice.dur) as cpu_time,
  thread.upid as process_upid
FROM sched_slice
INNER JOIN thread ON (thread.name LIKE 'hwuiTask%' AND thread.utid = sched_slice.utid)
INNER JOIN test_start_ts ON (sched_slice.ts >= test_start_ts.test_start AND sched_slice.ts <= test_start_ts.test_end)
WHERE sched_slice.dur >= 0
GROUP BY thread.upid;

CREATE VIEW cpu_time_ui_thread AS
SELECT
  sum(sched_slice.dur) as cpu_time,
  thread.upid as process_upid
FROM sched_slice
INNER JOIN process ON (process.upid = thread.upid)
INNER JOIN thread ON ((thread.name=substr(process.name,-15) OR thread.name LIKE '%skottie') AND thread.utid = sched_slice.utid)
INNER JOIN test_start_ts ON (sched_slice.ts >= test_start_ts.test_start AND sched_slice.ts <= test_start_ts.test_end)
WHERE sched_slice.dur >= 0
GROUP BY thread.upid;

CREATE VIEW cpu_time_rt AS
SELECT
  sum(sched_slice.dur) as cpu_time,
  thread.upid as process_upid
FROM sched_slice
INNER JOIN thread ON (thread.name='RenderThread' AND thread.utid = sched_slice.utid)
INNER JOIN test_start_ts ON (sched_slice.ts >= test_start_ts.test_start AND sched_slice.ts <= test_start_ts.test_end)
WHERE sched_slice.dur >= 0
GROUP BY thread.upid;

CREATE VIEW cpu_time_skottie_animator AS
SELECT
  sum(sched_slice.dur) as cpu_time,
  thread.upid as process_upid
FROM sched_slice
INNER JOIN thread ON (thread.name='SkottieAnimator' AND thread.utid = sched_slice.utid)
INNER JOIN test_start_ts ON (sched_slice.ts >= test_start_ts.test_start AND sched_slice.ts <= test_start_ts.test_end)
WHERE sched_slice.dur >= 0
GROUP BY thread.upid;

CREATE VIEW hwui_gpu_completion2 AS
SELECT
  count(*) as gpu_completion_count,
  max(dur) as gpu_completion_max,
  min(dur) as gpu_completion_min,
  avg(dur) as gpu_completion_avg,
  sum(dur) as gpu_completion_sum,
  thread.upid as process_upid
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.name='GPU completion' AND thread.utid = thread_track.utid)
INNER JOIN test_start_ts ON (slice.ts >= test_start_ts.test_start AND slice.ts <= test_start_ts.test_end)
WHERE slice.name LIKE 'waiting for GPU completion%' AND slice.dur >= 0
GROUP BY thread_track.utid;

CREATE VIEW hwui_gpu_completion3 AS
SELECT
  sum(dur) as gpu_completion_sum,
  thread.upid as process_upid
FROM slice
INNER JOIN thread_track ON (thread_track.id = slice.track_id)
INNER JOIN thread ON (thread.utid = thread_track.utid)
INNER JOIN test_start_ts ON (slice.ts >= test_start_ts.test_start AND slice.ts <= test_start_ts.test_end)
WHERE slice.name LIKE 'waiting for frame%' AND slice.dur >= 0
GROUP BY thread_track.utid;

CREATE VIEW skottie_metric_output AS
SELECT SkottieMetric(
  'process_info', (
    SELECT RepeatedField(
      ProcessRenderInfoEx(
        'hwui_process_info',
        ProcessRenderInfo(
            'process_name', process_name,
            'rt_cpu_time_ms', rt_cpu_time_ms,

            'draw_frame_count', hwui_draw_frame2.draw_frame_count,
            'draw_frame_max', hwui_draw_frame2.draw_frame_max,
            'draw_frame_min', hwui_draw_frame2.draw_frame_min,
            'draw_frame_avg', hwui_draw_frame2.draw_frame_avg,

            'flush_count', hwui_flush_commands.flush_count,
            'flush_max', hwui_flush_commands.flush_max,
            'flush_min', hwui_flush_commands.flush_min,
            'flush_avg', hwui_flush_commands.flush_avg,

            'prepare_tree_count', hwui_prepare_tree.prepare_tree_count,
            'prepare_tree_max', hwui_prepare_tree.prepare_tree_max,
            'prepare_tree_min', hwui_prepare_tree.prepare_tree_min,
            'prepare_tree_avg', hwui_prepare_tree.prepare_tree_avg,

            'gpu_completion_count', hwui_gpu_completion2.gpu_completion_count,
            'gpu_completion_max', hwui_gpu_completion2.gpu_completion_max,
            'gpu_completion_min', hwui_gpu_completion2.gpu_completion_min,
            'gpu_completion_avg', hwui_gpu_completion2.gpu_completion_avg,

            'ui_record_count', hwui_ui_record.ui_record_count,
            'ui_record_max', hwui_ui_record.ui_record_max,
            'ui_record_min', hwui_ui_record.ui_record_min,
            'ui_record_avg', hwui_ui_record.ui_record_avg,

            'shader_compile_count', hwui_shader_compile.shader_compile_count,
            'shader_compile_time', hwui_shader_compile.shader_compile_time,
            'shader_compile_avg', hwui_shader_compile.shader_compile_avg,

            'cache_hit_count', hwui_cache_hit.cache_hit_count,
            'cache_hit_time', hwui_cache_hit.cache_hit_time,
            'cache_hit_avg', hwui_cache_hit.cache_hit_avg,

            'cache_miss_count', hwui_cache_miss.cache_miss_count,
            'cache_miss_time', hwui_cache_miss.cache_miss_time,
            'cache_miss_avg', hwui_cache_miss.cache_miss_avg,

            'graphics_cpu_mem_max', CAST(hwui_graphics_cpu_mem.graphics_cpu_mem_max as INT64),
            'graphics_cpu_mem_min', CAST(hwui_graphics_cpu_mem.graphics_cpu_mem_min as INT64),
            'graphics_cpu_mem_avg', hwui_graphics_cpu_mem.graphics_cpu_mem_avg,

            'graphics_gpu_mem_max', CAST(hwui_graphics_gpu_mem.graphics_gpu_mem_max as INT64),
            'graphics_gpu_mem_min', CAST(hwui_graphics_gpu_mem.graphics_gpu_mem_min as INT64),
            'graphics_gpu_mem_avg', hwui_graphics_gpu_mem.graphics_gpu_mem_avg,

            'texture_mem_max', CAST(hwui_texture_mem.texture_mem_max as INT64),
            'texture_mem_min', CAST(hwui_texture_mem.texture_mem_min as INT64),
            'texture_mem_avg', hwui_texture_mem.texture_mem_avg,

            'all_mem_max', CAST(hwui_all_mem.all_mem_max as INT64),
            'all_mem_min', CAST(hwui_all_mem.all_mem_min as INT64),
            'all_mem_avg', hwui_all_mem.all_mem_avg
        ),
        'skottie_animator_count', ifnull(skottie_animator.skottie_animator_count, 0),
        'skottie_animator_max', ifnull(skottie_animator.skottie_animator_max, 0),
        'skottie_animator_min', ifnull(skottie_animator.skottie_animator_min, 0),
        'skottie_animator_avg', ifnull(skottie_animator.skottie_animator_avg, 0.0),

        'dequeue_buffer_count', dequeue_buffer.dequeue_buffer_count,
        'dequeue_buffer_max', dequeue_buffer.dequeue_buffer_max,
        'dequeue_buffer_min', dequeue_buffer.dequeue_buffer_min,
        'dequeue_buffer_avg', dequeue_buffer.dequeue_buffer_avg,

        'render_time_avg', ifnull(skottie_animator.skottie_animator_avg, 0.0) + hwui_draw_frame2.draw_frame_avg,
        'render_time_avg_no_dequeue', ifnull(skottie_animator.skottie_animator_avg, 0.0) + hwui_draw_frame2.draw_frame_avg - ifnull(dequeue_buffer.dequeue_buffer_avg, 0.0),

        'ui_thread_cpu_time', ifnull(cpu_time_ui_thread.cpu_time, 0),
        'rt_thread_cpu_time', ifnull(cpu_time_rt.cpu_time, 0),
        'hwui_tasks_cpu_time', ifnull(cpu_time_rt_hwui_tasks.cpu_time, 0),
        'skottie_animator_cpu_time', ifnull(cpu_time_skottie_animator.cpu_time, 0),

        'total_cpu_time', ifnull(total_cpu_time.cpu_time, 0),
        'total_gpu_time', ifnull(hwui_gpu_completion3.gpu_completion_sum, 0),
        'total_time', ifnull(total_cpu_time.cpu_time, 0) + ifnull(hwui_gpu_completion3.gpu_completion_sum, 0),

        'startup_time', startup_time.start_time
      )
    )
    FROM hwui_processes
    LEFT JOIN hwui_draw_frame2 ON (hwui_draw_frame2.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_flush_commands ON (hwui_flush_commands.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_prepare_tree ON (hwui_prepare_tree.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_gpu_completion2 ON (hwui_gpu_completion2.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_ui_record ON (hwui_ui_record.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_shader_compile ON (hwui_shader_compile.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_cache_hit ON (hwui_cache_hit.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_cache_miss ON (hwui_cache_miss.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN hwui_graphics_cpu_mem ON (hwui_graphics_cpu_mem.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_graphics_gpu_mem ON (hwui_graphics_gpu_mem.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_texture_mem ON (hwui_texture_mem.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_all_mem ON (hwui_all_mem.process_upid = hwui_processes.process_upid)
    LEFT JOIN skottie_animator ON (skottie_animator.process_upid = hwui_processes.process_upid)
    LEFT JOIN dequeue_buffer ON (dequeue_buffer.render_thread_id = hwui_processes.render_thread_id)
    LEFT JOIN total_cpu_time ON (total_cpu_time.process_upid = hwui_processes.process_upid)
    LEFT JOIN cpu_time_rt_hwui_tasks ON (cpu_time_rt_hwui_tasks.process_upid = hwui_processes.process_upid)
    LEFT JOIN cpu_time_rt ON (cpu_time_rt.process_upid = hwui_processes.process_upid)
    LEFT JOIN cpu_time_skottie_animator ON (cpu_time_skottie_animator.process_upid = hwui_processes.process_upid)
    LEFT JOIN cpu_time_ui_thread ON (cpu_time_ui_thread.process_upid = hwui_processes.process_upid)
    LEFT JOIN hwui_gpu_completion3 ON (hwui_gpu_completion3.process_upid = hwui_processes.process_upid)
    LEFT JOIN startup_time ON (startup_time.render_thread_id = hwui_processes.render_thread_id)
    JOIN (SELECT MAX(rt_cpu_time_ms), process_upid AS id FROM hwui_processes) max_render ON hwui_processes.process_upid = max_render.id
    -- process name is often missing on WearOs/Android P -> instead select process is highest CPU time in RenderThread.
    -- WHERE hwui_processes.process_name='org.skia.skottie'
  )
);


