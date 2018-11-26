SkQP Render Test Algorithm
==========================

The following is a description of the render test validation algorithm that
will be used by the version of SkQP that will be released for Android Q-release.

There is a global macro constant: `SK_SKQP_GLOBAL_ERROR_TOLERANCE`, which
reflects the `gn` variable `skia_skqp_global_error_tolerance`.  This is usually
set to 8.

First, look for a file named `skqp/rendertests.txt` in the
`platform_tools/android/apps/skqp/src/main/assets` directory.  The format of
this file is:  each line contains one render test name, followed by a comma,
followed by an integer.  The integer is the `passing_threshold` for that test.

For each test, we have a `max_image` and a `min_image`.  These are PNG-encoded
images stored in SkQP's APK's asset directory (in the paths `gmkb/${TEST}/min.png`
and `gmkb/${TEST}/max.png`).

The test input is a rendered image.  This will be produced by running one of
the render tests against the either the `vk` (Vulkan) or `gles` (OpenGL ES)
Skia backend.

Here is psuedocode for the error calculation:

    function calculate_pixel_error(pixel_value, pixel_max, pixel_min):
        pixel_error = 0

        for color_channel in { red, green, blue, alpha }:
            value = get_color(pixel_value, color_channel)
            v_max = get_color(pixel_max,   color_channel)
            v_min = get_color(pixel_min,   color_channel)

            if value > v_max:
                channel_error = value - v_max
            elif value < v_min:
                channel_error = v_min - value
            else:
                channel_error = 0
            pixel_error = max(pixel_error, channel_error)

        return max(0, pixel_error - SK_SKQP_GLOBAL_ERROR_TOLERANCE);

    function get_error(rendered_image, max_image, min_image):
        assert(dimensions(rendered_image) == dimensions(max_image))
        assert(dimensions(rendered_image) == dimensions(min_image))

        max_error = 0
        bad_pixels = 0
        total_error = 0

        error_image = allocate_bitmap(dimensions(rendered_image))

        for xy in list_all_pixel_coordinates(rendered_image):
            pixel_error = calculate_pixel_error(rendered_image(xy),
                                                max_image(xy),
                                                min_image(xy))
            if pixel_error > 0:
                for neighboring_xy in find_neighbors(xy):
                    if not inside(neighboring_xy, dimensions(rendered_image)):
                        continue
                    pixel_error = min(pixel_error,
                                      calculate_pixel_error(rendered_image(xy),
                                                            max_image(neighboring_xy),
                                                            min_image(neighboring_xy)))

            if pixel_error > 0:
                max_error = max(max_error, pixel_error)
                bad_pixels += 1
                total_error += pixel_error

                error_image(xy) = linear_interpolation(black, red, pixel_error)
            else:
                error_image(xy) = white

        return ((total_error, max_error, bad_pixels), error_image)

For each render test, there is a threshold value for `total_error`, :
`passing_threshold`.

If `passing_threshold >= 0 && total_error > passing_threshold`, then the test
is a failure and is included in the report.  if `passing_threshold == -1`, then
the test always passes, but we do execute the test to verify that the driver
does not crash.

We generate a report with the following information for each test:

    backend_name,render_test_name,max_error,bad_pixels,total_error

in CSV format in the file `out.csv`.  A HTML report of just the failing tests
is written to the file `report.html`.  This version includes four images for
each test:  `rendered_image`, `max_image`, `min_image`, and `error_image`, as
well as the three metrics: `max_error`, `bad_pixels`, and `total_error`.



