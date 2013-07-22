#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import print_function
from math import *


COPYRIGHT = '''/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */'''


HELP = '// To regenerate SkPMetricUtil_generated.h, simply run ./generate_pmetric_tables.py'


# From Barten SPIE 1989
def contrast_sensitivity(cycles_per_degree, luminance):
    a = 440.0 * pow(1.0 + 0.7 / luminance, -0.2)
    b = 0.3 * pow(1 + 100.0 / luminance, 0.15)
    return a * cycles_per_degree * exp(-b * cycles_per_degree) * sqrt(1.0 + 0.06 * exp(b * cycles_per_degree))


# From Ward Larson Siggraph 1997
def threshold_vs_intensity(adaptation_luminance):
    log_lum = float('-inf') # Works in Python 2.6+
    try:
        log_lum = log10(adaptation_luminance)
    except ValueError:
        pass

    x = 0.0

    if log_lum < -3.94:
        x = -2.86

    elif log_lum < -1.44:
        x = pow(0.405 * log_lum + 1.6, 2.18) - 2.86

    elif log_lum < -0.0184:
        x = log_lum - 0.395

    elif log_lum < 1.9:
        x = pow(0.249 * log_lum + 0.65, 2.7) - 0.72

    else:
        x = log_lum - 1.255

    return pow(10.0, x)


# From Daly 1993
def visual_mask(contrast):
    x = pow(392.498 * contrast, 0.7)
    x = pow(0.0153 * x, 4.0)
    return pow(1.0 + x, 0.25)


# float gCubeRootTable[]
CUBE_ROOT_ACCESS_FUNCTION = '''
static float get_cube_root(float value) {
    SkASSERT(value >= 0.0f);
    SkASSERT(value * 1023.0f < 1024.0f);
    return gCubeRootTable[(int)(value * 1023.0f)];
}
'''
def generate_cube_root_table(stream):
    print('static float gCubeRootTable[] = {', end='', file=stream)
    for i in range(1024):
        if i % 6 == 0:
            print('\n    ', end='', file=stream)
        print("%.10f" % pow(i / 1024.0, 1.0 / 3.0), end='f,', file=stream)
    print('\n};', end='', file=stream)
    print(CUBE_ROOT_ACCESS_FUNCTION, file=stream)


# float gGammaTable[]
GAMMA_ACCESS_FUNCTION = '''
static float get_gamma(unsigned char value) {
    return gGammaTable[value];
}
'''
def generate_gamma_table(stream):
    print('static float gGammaTable[] = {', end='', file=stream)
    for i in range(256):
        if i % 6 == 0:
            print('\n    ', end='', file=stream)
        print("%.10f" % pow(i / 255.0, 2.2), end='f,', file=stream)
    print('\n};', end='', file=stream)
    print(GAMMA_ACCESS_FUNCTION, file=stream)


# float gTVITable[]
TVI_ACCESS_FUNCTION = '''
static float get_threshold_vs_intensity(float value) {
    SkASSERT(value >= 0.0f);
    SkASSERT(value < 100.0f);
    return gTVITable[(int)(value * 100.0f)];
}
'''
def generate_tvi_table(stream):
    print('static float gTVITable[] = {', end='', file=stream)
    for i in range(10000):
        if i % 6 == 0:
            print('\n    ', end='', file=stream)
        print("%.10f" % threshold_vs_intensity(i / 100.0), end='f,', file=stream)
    print('\n};', end='', file=stream)
    print(TVI_ACCESS_FUNCTION, file=stream)


# float gVisualMaskTable[]
VISUAL_MASK_DOMAIN = 4000
VISUAL_MASK_ACCESS_FUNCTION = '''
static float get_visual_mask(float value) {{
    SkASSERT(value >= 0.0f);
    SkASSERT(value < {}.0f);
    return gVisualMaskTable[(int)value];
}}'''
def generate_visual_mask_table(stream):
    print('static float gVisualMaskTable[] = {', end='', file=stream)
    for i in range(VISUAL_MASK_DOMAIN):
        if i % 6 == 0:
            print('\n    ', end='', file=stream)
        print("%.10f" % visual_mask(i), end='f,', file=stream)
    print('\n};', end='', file=stream)
    print(VISUAL_MASK_ACCESS_FUNCTION.format(VISUAL_MASK_DOMAIN), file=stream)


def generate_lookup_tables(stream):
    print(COPYRIGHT, file=stream)
    print(HELP, file=stream)
    print('namespace SkPMetricUtil {', file=stream)
    generate_cube_root_table(stream)
    generate_gamma_table(stream)
    generate_tvi_table(stream)
    generate_visual_mask_table(stream)
    print('}', file=stream)


def main():
    pmetric_util_out = open('SkPMetricUtil_generated.h', 'wb')
    generate_lookup_tables(pmetric_util_out)
    pmetric_util_out.close()


if __name__ == '__main__':
    main()
