/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSemaphore.h"
#include <ctype.h>
#include <stdio.h>
#include <thread>

static char buffer[4096];
static SkSemaphore renderer_to_gpu,
                   gpu_to_renderer;

static void renderer() {
    const char* msg = "hello";
    memcpy(buffer, msg, strlen(msg)+1);
    renderer_to_gpu.signal(1);

    while (true) {
        gpu_to_renderer.wait();
        buffer[0] = toupper(buffer[0]);
        renderer_to_gpu.signal(1);
    }
}

static void gpu() {
    renderer_to_gpu.wait();

    char msg[4096];
    memcpy(msg, buffer, 4096);

    printf("%s\n", msg);

    for (char* c = msg; *c != 0; c++) {
        buffer[0] = *c;
        gpu_to_renderer.signal(1);

        renderer_to_gpu.wait();
        printf("%c", buffer[0]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    std::thread(renderer).detach();
    gpu();
    return 0;
}
