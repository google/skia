/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include <vector>
#include <string>
#include <sstream>

#include "sfntly/port/type.h"
#include "font_subsetter.h"

template <typename T>
class HexTo {
 public:
  explicit HexTo(const char* in) {
    std::stringstream ss;
    ss << std::hex << in;
    ss >> value_;
  }
  operator T() const { return value_; }

 private:
  T value_;
};

bool LoadFile(const char* input_file_path, sfntly::ByteVector* input_buffer) {
  assert(input_file_path);
  assert(input_buffer);

  FILE* input_file = NULL;
#if defined WIN32
  fopen_s(&input_file, input_file_path, "rb");
#else
  input_file = fopen(input_file_path, "rb");
#endif
  if (input_file == NULL) {
    return false;
  }
  fseek(input_file, 0, SEEK_END);
  size_t file_size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);
  input_buffer->resize(file_size);
  size_t bytes_read = fread(&((*input_buffer)[0]), 1, file_size, input_file);
  fclose(input_file);
  return bytes_read == file_size;
}

bool SaveFile(const char* output_file_path, const unsigned char* output_buffer,
              int buffer_length) {
  int byte_count = 0;
  if (buffer_length > 0) {
    FILE* output_file = NULL;
#if defined WIN32
    fopen_s(&output_file, output_file_path, "wb");
#else
    output_file = fopen(output_file_path, "wb");
#endif
    if (output_file) {
      byte_count = fwrite(output_buffer, 1, buffer_length, output_file);
      fflush(output_file);
      fclose(output_file);
    }
    return buffer_length == byte_count;
  }
  return false;
}

bool StringToGlyphId(const char* input, std::vector<unsigned int>* glyph_ids) {
  assert(input);
  std::string hex_csv = input;
  size_t start = 0;
  size_t end = hex_csv.find_first_of(",");
  while (end != std::string::npos) {
    glyph_ids->push_back(
        HexTo<unsigned int>(hex_csv.substr(start, end - start).c_str()));
    start = end + 1;
    end = hex_csv.find_first_of(",", start);
  }
  glyph_ids->push_back(HexTo<unsigned int>(hex_csv.substr(start).c_str()));
  return glyph_ids->size() > 0;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    fprintf(stderr,
        "Usage: %s <input path> <output path> <font name> <glyph ids>\n",
        argv[0]);
    fprintf(stderr, "\tGlyph ids are comma separated hex values\n");
    fprintf(stderr, "\te.g. 20,1a,3b,4f\n");
    return 0;
  }

  sfntly::ByteVector input_buffer;
  if (!LoadFile(argv[1], &input_buffer)) {
    fprintf(stderr, "ERROR: unable to load font file %s\n", argv[1]);
    return 0;
  }

  std::vector<unsigned int> glyph_ids;
  if (!StringToGlyphId(argv[4], &glyph_ids)) {
    fprintf(stderr, "ERROR: unable to parse input glyph id\n");
    return 0;
  }

  unsigned char* output_buffer = NULL;
  int output_length =
      SfntlyWrapper::SubsetFont(argv[3],
                                &(input_buffer[0]),
                                input_buffer.size(),
                                &(glyph_ids[0]),
                                glyph_ids.size(),
                                &output_buffer);

  int result = SaveFile(argv[2], output_buffer, output_length) ? 1 : 0;
  delete[] output_buffer;
  return result;
}
