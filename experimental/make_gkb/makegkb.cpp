/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
    makegkb : make GM Knowledgebase

    Usage:
        makegkp input_directory output_directory

    for each subdirectory of input_directory, look for files of the form:

        input_directory/subdirectory/[*].png

    then write new files

        output_directory/subdirectory/min.png
        output_directory/subdirectory/max.png

    with minimum and maximum values for each pixel's color channels.

    If necessary create output_directory.
*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

#include <png.h>

#include "PngInterface.h"
#include "ReadDirectory.h"

constexpr char DOT_PNG[] = ".png";
constexpr char MIN_PNG[] = "min.png";
constexpr char MAX_PNG[] = "max.png";

static std::string path_join(const char* u, const char* v) {
    return std::string(u) + "/" + v;
}

struct Err : public std::ostringstream {
    ~Err() { std::cerr << this->str(); }
};

static void process_directory(const char* input_directory,
                              const char* output_directory) {
    int w = -1;
    int h = -1;
    size_t s = 0;
    size_t image_size = 0;
    std::vector<unsigned char> img;
    std::vector<unsigned char> img_min;
    std::vector<unsigned char> img_max;

    std::vector<std::string> directory_listing = ReadDirectory(input_directory, DOT_PNG);
    for (const auto& filename : directory_listing) {
        std::string path = path_join(input_directory, filename.c_str());
        int width, height;
        if (!ReadPngRgba8888FromFile(path.c_str(), &width, &height, &img)) {
            Err() << "   error reading image: " << filename << '\n';
            continue;
        }
        if (w == -1) {
            // first pass.
            w = width;
            h = height;
            assert(img.size() == width * height * 4);
            img_min = img;
            img_max = img;
        } else if (w != width || h != height) {
            Err() << "   [" << width << ',' << height << "] bad size: " << filename << '\n';
            continue;
        } else {
            for (size_t i = 0; i < img.size(); ++i) {
                unsigned char value = img[i];
                if (value < img_min[i]) { img_min[i] = value; }
                if (value > img_max[i]) { img_max[i] = value; }
            }
        }
    }
    if (w == -1 || h == -1) {
        return;
    }
    MakeDirectory(output_directory);

    std::string outputPath = path_join(output_directory, MIN_PNG);
    if (!WritePngRgba8888ToFile(w, h, img_min.data(), outputPath.c_str())) {
        assert(false);
    }
    outputPath = path_join(output_directory, MAX_PNG);
    if (!WritePngRgba8888ToFile(w, h, img_max.data(), outputPath.c_str())) {
        assert(false);
    }
}

void thread_proc(const char* input,
                 const char* output,
                 const std::vector<std::string>& directory_listing,
                 unsigned thread_index,
                 unsigned thread_count) {
    for (unsigned i = 0; i < directory_listing.size(); ++i) {
        if (i % thread_count != thread_index) {
            continue;
        }
        const auto& subdirectory = directory_listing[i];
        Err() << "[[ " << subdirectory << " ]]\n";
        std::string subdirectory_path = path_join(input, subdirectory.c_str());
        std::string output_subdirectory = path_join(output, subdirectory.c_str());
        process_directory(subdirectory_path.c_str(), output_subdirectory.c_str());
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "\nUsage:\n  " << argv[0] << " INPUT_DIR OUTPUT_DIR\n\n";
        exit(1);
    }
    const char* input = argv[1];
    const char* output = argv[2];
    MakeDirectory(output);
    std::vector<std::string> directory_listing = ReadDirectory(input);

    unsigned N = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(N);
    for (unsigned i = 0; i < N; ++i) {
        threads[i] = std::thread(thread_proc, input, output, directory_listing, i, N);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}
