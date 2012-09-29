#ifndef SCRIPT_IDENTIFY_H_
#define SCRIPT_IDENTIFY_H_

#include <stdint.h>

#include <harfbuzz-shaper.h>

static const uint32_t HB_InvalidCodePoint = 0xffffffffu;

// -----------------------------------------------------------------------------
// Return the next Unicode code point from a UTF-16 vector
//   chars: a pointer to @len words
//   iter: (input/output) an index into @chars. This is updated.
//   returns: HB_InvalidCodePoint on error and the code point otherwise.
// -----------------------------------------------------------------------------
uint32_t utf16_to_code_point(const uint16_t *chars, size_t len, ssize_t *iter);

// -----------------------------------------------------------------------------
// Like the above, except that the code points are traversed backwards. Thus,
// on the first call, |iter| should be |len| - 1.
// -----------------------------------------------------------------------------
uint32_t utf16_to_code_point(const uint16_t *chars, size_t len, ssize_t *iter);

// -----------------------------------------------------------------------------
// Return the script of the given code point
// -----------------------------------------------------------------------------
HB_Script code_point_to_script(uint32_t cp);

// -----------------------------------------------------------------------------
// Find the next script run in a UTF-16 string.
//
// A script run is a subvector of codepoints, all of which are in the same
// script. A run will never cut a surrogate pair in half at either end.
//
// num_code_points: (output, maybe NULL) the number of code points in the run
// output: (output) the @pos, @length and @script fields are set on success
// chars: the UTF-16 string
// len: the length of @chars, in words
// iter: (in/out) the current index into the string. This should be 0 for the
//   first call and is updated on exit.
//
// returns: non-zero if a script run was found and returned.
// -----------------------------------------------------------------------------
char hb_utf16_script_run_next(unsigned *num_code_points, HB_ScriptItem *output,
                              const uint16_t *chars, size_t len, ssize_t *iter);

// -----------------------------------------------------------------------------
// This is the same as above, except that the input is traversed backwards.
// Thus, on the first call, |iter| should be |len| - 1.
// -----------------------------------------------------------------------------
char hb_utf16_script_run_prev(unsigned *num_code_points, HB_ScriptItem *output,
                              const uint16_t *chars, size_t len, ssize_t *iter);

#endif
