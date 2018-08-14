// The latest version of this library is available on GitHub;
//   https://github.com/sheredom/json.h

// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include "json.h"

#include <stdlib.h>

// work around MSVC 2013 and lower not having strtoull
#if defined(_MSC_VER) && (_MSC_VER <= 1800)
#define json_strtoull _strtoui64
#else
#define json_strtoull strtoull
#endif

#if defined(__clang__)
#pragma clang diagnostic push

// we do one big allocation via malloc, then cast aligned slices of this for
// our structures - we don't have a way to tell the compiler we know what we
// are doing, so disable the warning instead!
#pragma clang diagnostic ignored "-Wcast-align"
#elif defined(_MSC_VER)
#pragma warning(push)

// disable 'function selected for inline expansion' warning
#pragma warning(disable : 4711)
#endif

struct json_parse_state_s {
  const char *src;
  size_t size;
  size_t offset;
  size_t flags_bitset;
  char *data;
  char *dom;
  size_t dom_size;
  size_t data_size;
  size_t line_no;     // line counter for error reporting
  size_t line_offset; // (offset-line_offset) is the character number (in bytes)
  size_t error;
};

static int json_is_hexadecimal_digit(const char c) {
  return (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
          ('A' <= c && c <= 'F'));
}

static int json_skip_whitespace(struct json_parse_state_s *state) {
  size_t offset = state->offset;
  const size_t size = state->size;

  // the only valid whitespace according to ECMA-404 is ' ', '\n', '\r' and '\t'
  switch (state->src[offset]) {
  default:
    return 0;
  case ' ':
  case '\r':
  case '\t':
  case '\n':
    break;
  }

  for (; offset < size; offset++) {
    switch (state->src[offset]) {
    default:
      // Update offset
      state->offset = offset;
      return 1;
    case ' ':
    case '\r':
    case '\t':
      break;
    case '\n':
      state->line_no++;
      state->line_offset = offset;
      break;
    }
  }

  // Update offset
  state->offset = offset;
  return 1;
}

static int json_skip_c_style_comments(struct json_parse_state_s *state) {
  // do we have a comment?
  if ('/' == state->src[state->offset]) {
    // skip '/'
    state->offset++;

    if ('/' == state->src[state->offset]) {
      // we had a comment of the form //

      // skip second '/'
      state->offset++;

      while (state->offset < state->size) {
        switch (state->src[state->offset]) {
        default:
          // skip the character in the comment
          state->offset++;
          break;
        case '\n':
          // if we have a newline, our comment has ended! Skip the newline
          state->offset++;

          // we entered a newline, so move our line info forward
          state->line_no++;
          state->line_offset = state->offset;
          return 1;
        }
      }

      // we reached the end of the JSON file!
      return 1;
    } else if ('*' == state->src[state->offset]) {
      // we had a comment in the form /* */

      // skip '*'
      state->offset++;

      while (state->offset + 1 < state->size) {
        if (('*' == state->src[state->offset]) &&
            ('/' == state->src[state->offset + 1])) {
          // we reached the end of our comment!
          state->offset += 2;
          return 1;
        } else if ('\n' == state->src[state->offset]) {
          // we entered a newline, so move our line info forward
          state->line_no++;
          state->line_offset = state->offset;
        }

        // skip character within comment
        state->offset++;
      }

      // a comment like /* has to end with a */. We reached the end of the file
      // which is a failure!
      return 1;
    }
  }

  // we didn't have any comment, which is ok too!
  return 0;
}

static int json_skip_all_skippables(struct json_parse_state_s *state) {
  // skip all whitespace and other skippables until there are none left
  // note that the previous version suffered from read past errors should
  // the stream end on json_skip_c_style_comments eg. '{"a" ' with comments flag

  int did_consume = 0;
  const size_t size = state->size;

  if (json_parse_flags_allow_c_style_comments & state->flags_bitset) {
    do {
      if (state->offset == size) {
        state->error = json_parse_error_premature_end_of_buffer;
        return 1;
      }

      did_consume = json_skip_whitespace(state);

      // This should really be checked on access, not in front of every call
      if (state->offset == size) {
        state->error = json_parse_error_premature_end_of_buffer;
        return 1;
      }

      did_consume |= json_skip_c_style_comments(state);
    } while (0 != did_consume);
  } else {
    do {
      if (state->offset == size) {
        state->error = json_parse_error_premature_end_of_buffer;
        return 1;
      }

      did_consume = json_skip_whitespace(state);
    } while (0 != did_consume);
  }

  if (state->offset == size) {
    state->error = json_parse_error_premature_end_of_buffer;
    return 1;
  }

  return 0;
}

static int json_get_value_size(struct json_parse_state_s *state,
                               int is_global_object);

static int json_get_string_size(struct json_parse_state_s *state,
                                size_t is_key) {
  size_t offset = state->offset;
  const size_t size = state->size;
  size_t data_size = 0;
  const int is_single_quote = '\'' == state->src[offset];
  const char quote_to_use = is_single_quote ? '\'' : '"';

  if ((json_parse_flags_allow_location_information & state->flags_bitset) !=
          0 &&
      is_key != 0) {
    state->dom_size += sizeof(struct json_string_ex_s);
  } else {
    state->dom_size += sizeof(struct json_string_s);
  }

  if ('"' != state->src[offset]) {
    // if we are allowed single quoted strings check for that too
    if (!((json_parse_flags_allow_single_quoted_strings &
           state->flags_bitset) &&
          is_single_quote)) {
      state->error = json_parse_error_expected_opening_quote;
      state->offset = offset;
      return 1;
    }
  }

  // skip leading '"' or '\''
  offset++;

  while ((offset < size) && (quote_to_use != state->src[offset])) {
    // add space for the character
    data_size++;

    if ('\\' == state->src[offset]) {
      // skip reverse solidus character
      offset++;

      if (offset == size) {
        state->error = json_parse_error_premature_end_of_buffer;
        state->offset = offset;
        return 1;
      }

      switch (state->src[offset]) {
      default:
        state->error = json_parse_error_invalid_string_escape_sequence;
        state->offset = offset;
        return 1;
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        // all valid characters!
        offset++;
        break;
      case 'u':
        if (offset + 5 < size) {
          // invalid escaped unicode sequence!
          state->error = json_parse_error_invalid_string_escape_sequence;
          state->offset = offset;
          return 1;
        } else if (!json_is_hexadecimal_digit(state->src[offset + 1]) ||
                   !json_is_hexadecimal_digit(state->src[offset + 2]) ||
                   !json_is_hexadecimal_digit(state->src[offset + 3]) ||
                   !json_is_hexadecimal_digit(state->src[offset + 4])) {
          // escaped unicode sequences must contain 4 hexadecimal digits!
          state->error = json_parse_error_invalid_string_escape_sequence;
          state->offset = offset;
          return 1;
        }

        // valid sequence!
        state->offset += 5;

        // add space for the 5 character sequence too
        data_size += 5;
        break;
      }
    } else if (('\r' == state->src[offset]) || ('\n' == state->src[offset])) {
      if (!(json_parse_flags_allow_multi_line_strings & state->flags_bitset)) {
        // invalid escaped unicode sequence!
        state->error = json_parse_error_invalid_string_escape_sequence;
        state->offset = offset;
        return 1;
      }

      offset++;
    } else {
      // skip character (valid part of sequence)
      offset++;
    }
  }

  // skip trailing '"' or '\''
  offset++;

  // add enough space to store the string
  state->data_size += data_size;

  // one more byte for null terminator ending the string!
  state->data_size++;

  // update offset
  state->offset = offset;

  return 0;
}

static int is_valid_unquoted_key_char(const char c) {
  return (('0' <= c && c <= '9') || ('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z') || ('_' == c));
}

static int json_get_key_size(struct json_parse_state_s *state) {
  if (json_parse_flags_allow_unquoted_keys & state->flags_bitset) {
    // if we are allowing unquoted keys, first grok for a comma...
    if ('"' == state->src[state->offset]) {
      // ... if we got a comma, just parse the key as a string as normal
      return json_get_string_size(state, 1);
    } else if ((json_parse_flags_allow_single_quoted_strings &
                state->flags_bitset) &&
               ('\'' == state->src[state->offset])) {
      // ... if we got a comma, just parse the key as a string as normal
      return json_get_string_size(state, 1);
    } else {
      while ((state->offset < state->size) &&
             is_valid_unquoted_key_char(state->src[state->offset])) {
        state->offset++;
        state->data_size++;
      }

      // one more byte for null terminator ending the string!
      state->data_size++;

      if (json_parse_flags_allow_location_information & state->flags_bitset)
        state->dom_size += sizeof(struct json_string_ex_s);
      else
        state->dom_size += sizeof(struct json_string_s);

      return 0;
    }
  } else {
    // we are only allowed to have quoted keys, so just parse a string!
    return json_get_string_size(state, 1);
  }
}

static int json_get_object_size(struct json_parse_state_s *state,
                                int is_global_object) {
  size_t elements = 0;
  int allow_comma = 0;

  if (is_global_object) {
    // if we found an opening '{' of an object, we actually have a normal JSON
    // object at the root of the DOM...
    if (!json_skip_all_skippables(state) && '{' == state->src[state->offset]) {
      // .. and we don't actually have a global object after all!
      is_global_object = 0;
    }
  }

  if (!is_global_object) {
    if ('{' != state->src[state->offset]) {
      state->error = json_parse_error_unknown;
      return 1;
    }

    // skip leading '{'
    state->offset++;
  }

  state->dom_size += sizeof(struct json_object_s);

  while (state->offset < state->size) {
    if (!is_global_object) {
      if (json_skip_all_skippables(state)) {
        state->error = json_parse_error_premature_end_of_buffer;
        return 1;
      }
      if ('}' == state->src[state->offset]) {
        // skip trailing '}'
        state->offset++;

        // finished the object!
        break;
      }
    } else {
      // we don't require brackets, so that means the object ends when the input
      // stream ends!
      if (json_skip_all_skippables(state)) {
        break;
      }
    }

    // if we parsed at least once element previously, grok for a comma
    if (allow_comma) {
      if (',' == state->src[state->offset]) {
        // skip comma
        state->offset++;
        allow_comma = 0;
      } else if (json_parse_flags_allow_no_commas & state->flags_bitset) {
        // we don't require a comma, and we didn't find one, which is ok!
        allow_comma = 0;
      } else {
        // otherwise we are required to have a comma, and we found none
        state->error = json_parse_error_expected_comma_or_closing_bracket;
        return 1;
      }

      if (json_parse_flags_allow_trailing_comma & state->flags_bitset) {
        continue;
      } else {
        if (json_skip_all_skippables(state)) {
          state->error = json_parse_error_premature_end_of_buffer;
          return 1;
        }
      }
    }

    if (json_get_key_size(state)) {
      // key parsing failed!
      state->error = json_parse_error_invalid_string;
      return 1;
    }

    if (json_skip_all_skippables(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (json_parse_flags_allow_equals_in_object & state->flags_bitset) {
      if ((':' != state->src[state->offset]) &&
          ('=' != state->src[state->offset])) {
        state->error = json_parse_error_expected_colon;
        return 1;
      }
    } else {
      if (':' != state->src[state->offset]) {
        state->error = json_parse_error_expected_colon;
        return 1;
      }
    }

    // skip colon
    state->offset++;

    if (json_skip_all_skippables(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (json_get_value_size(state, /* is_global_object = */ 0)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed a name/value pair!
    elements++;
    allow_comma = 1;
  }

  state->dom_size += sizeof(struct json_object_element_s) * elements;

  return 0;
}

static int json_get_array_size(struct json_parse_state_s *state) {
  size_t elements = 0;
  int allow_comma = 0;

  if ('[' != state->src[state->offset]) {
    // expected array to begin with leading '['
    state->error = json_parse_error_unknown;
    return 1;
  }

  // skip leading '['
  state->offset++;

  state->dom_size += sizeof(struct json_array_s);

  while (state->offset < state->size) {
    if (json_skip_all_skippables(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (']' == state->src[state->offset]) {
      // skip trailing ']'
      state->offset++;

      state->dom_size += sizeof(struct json_array_element_s) * elements;

      // finished the object!
      return 0;
    }

    // if we parsed at least once element previously, grok for a comma
    if (allow_comma) {
      if (',' == state->src[state->offset]) {
        // skip comma
        state->offset++;
        allow_comma = 0;
      } else if (!(json_parse_flags_allow_no_commas & state->flags_bitset)) {
        state->error = json_parse_error_expected_comma_or_closing_bracket;
        return 1;
      }

      if (json_parse_flags_allow_trailing_comma & state->flags_bitset) {
        allow_comma = 0;
        continue;
      } else {
        if (json_skip_all_skippables(state)) {
          state->error = json_parse_error_premature_end_of_buffer;
          return 1;
        }
      }
    }

    if (json_get_value_size(state, /* is_global_object = */ 0)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed an array element!
    elements++;
    allow_comma = 1;
  }

  // we consumed the entire input before finding the closing ']' of the array!
  state->error = json_parse_error_premature_end_of_buffer;
  return 1;
}

static int json_get_number_size(struct json_parse_state_s *state) {
  size_t offset = state->offset;
  const size_t size = state->size;
  int had_leading_digits = 0;

  state->dom_size += sizeof(struct json_number_s);

  if ((json_parse_flags_allow_hexadecimal_numbers & state->flags_bitset) &&
      (offset + 1 < size) && ('0' == state->src[offset]) &&
      (('x' == state->src[offset + 1]) || ('X' == state->src[offset + 1]))) {
    // skip the leading 0x that identifies a hexadecimal number
    offset += 2;

    // consume hexadecimal digits
    while ((offset < size) &&
           (('0' <= state->src[offset] && state->src[offset] <= '9') ||
            ('a' <= state->src[offset] && state->src[offset] <= 'f') ||
            ('A' <= state->src[offset] && state->src[offset] <= 'F'))) {
      offset++;
    }
  } else {
    int found_sign = 0;
    int inf_or_nan = 0;

    if ((offset < size) &&
        (('-' == state->src[offset]) ||
         ((json_parse_flags_allow_leading_plus_sign & state->flags_bitset) &&
          ('+' == state->src[offset])))) {
      // skip valid leading '-' or '+'
      offset++;

      found_sign = 1;
    }

    if (json_parse_flags_allow_inf_and_nan & state->flags_bitset) {
      const char inf[9] = "Infinity";
      const size_t inf_strlen = sizeof(inf) - 1;
      const char nan[4] = "NaN";
      const size_t nan_strlen = sizeof(nan) - 1;

      if (offset + inf_strlen < size) {
        int found = 1;
        size_t i;
        for (i = 0; i < inf_strlen; i++) {
          if (inf[i] != state->src[offset + i]) {
            found = 0;
            break;
          }
        }

        if (found) {
          // We found our special 'Infinity' keyword!
          offset += inf_strlen;

          inf_or_nan = 1;
        }
      }

      if (offset + nan_strlen < size) {
        int found = 1;
        size_t i;
        for (i = 0; i < nan_strlen; i++) {
          if (nan[i] != state->src[offset + i]) {
            found = 0;
            break;
          }
        }

        if (found) {
          // We found our special 'NaN' keyword!
          offset += nan_strlen;

          inf_or_nan = 1;
        }
      }
    }

    if (found_sign && !inf_or_nan && (offset < size) &&
        !('0' <= state->src[offset] && state->src[offset] <= '9')) {
      // check if we are allowing leading '.'
      if (!(json_parse_flags_allow_leading_or_trailing_decimal_point &
            state->flags_bitset) ||
          ('.' != state->src[offset])) {
        // a leading '-' must be immediately followed by any digit!
        state->error = json_parse_error_invalid_number_format;
        state->offset = offset;
        return 1;
      }
    }

    if ((offset < size) && ('0' == state->src[offset])) {
      // skip valid '0'
      offset++;

      // we need to record whether we had any leading digits for checks later
      had_leading_digits = 1;

      if ((offset < size) &&
          ('0' <= state->src[offset] && state->src[offset] <= '9')) {
        // a leading '0' must not be immediately followed by any digit!
        state->error = json_parse_error_invalid_number_format;
        state->offset = offset;
        return 1;
      }
    }

    // the main digits of our number next
    while ((offset < size) &&
           ('0' <= state->src[offset] && state->src[offset] <= '9')) {
      offset++;

      // we need to record whether we had any leading digits for checks later
      had_leading_digits = 1;
    }

    if ((offset < size) && ('.' == state->src[offset])) {
      offset++;

      if (!('0' <= state->src[offset] && state->src[offset] <= '9')) {
        if (!(json_parse_flags_allow_leading_or_trailing_decimal_point &
              state->flags_bitset) ||
            !had_leading_digits) {
          // a decimal point must be followed by at least one digit
          state->error = json_parse_error_invalid_number_format;
          state->offset = offset;
          return 1;
        }
      }

      // a decimal point can be followed by more digits of course!
      while ((offset < size) &&
             ('0' <= state->src[offset] && state->src[offset] <= '9')) {
        offset++;
      }
    }

    if ((offset < size) &&
        ('e' == state->src[offset] || 'E' == state->src[offset])) {
      // our number has an exponent!
      // skip 'e' or 'E'
      offset++;

      if ((offset < size) &&
          ('-' == state->src[offset] || '+' == state->src[offset])) {
        // skip optional '-' or '+'
        offset++;
      }

      // consume exponent digits
      while ((offset < size) &&
             ('0' <= state->src[offset] && state->src[offset] <= '9')) {
        offset++;
      }
    }
  }

  if (offset < size) {
    switch (state->src[offset]) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '}':
    case ',':
    case ']':
      // all of the above are ok
      break;
    case '=':
      if (json_parse_flags_allow_equals_in_object & state->flags_bitset) {
        break;
      }
    default:
      state->error = json_parse_error_invalid_number_format;
      state->offset = offset;
      return 1;
    }
  }

  state->data_size += offset - state->offset;

  // one more byte for null terminator ending the number string!
  state->data_size++;

  // update offset
  state->offset = offset;

  return 0;
}

static int json_get_value_size(struct json_parse_state_s *state,
                               int is_global_object) {
  if (json_parse_flags_allow_location_information & state->flags_bitset) {
    state->dom_size += sizeof(struct json_value_ex_s);
  } else {
    state->dom_size += sizeof(struct json_value_s);
  }

  if (is_global_object) {
    return json_get_object_size(state, /* is_global_object = */ 1);
  } else {
    if (json_skip_all_skippables(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }
    switch (state->src[state->offset]) {
    case '"':
      return json_get_string_size(state, 0);
    case '\'':
      if (json_parse_flags_allow_single_quoted_strings & state->flags_bitset) {
        return json_get_string_size(state, 0);
      } else {
        // invalid value!
        state->error = json_parse_error_invalid_value;
        return 1;
      }
    case '{':
      return json_get_object_size(state, /* is_global_object = */ 0);
    case '[':
      return json_get_array_size(state);
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return json_get_number_size(state);
    case '+':
      if (json_parse_flags_allow_leading_plus_sign & state->flags_bitset) {
        return json_get_number_size(state);
      } else {
        // invalid value!
        state->error = json_parse_error_invalid_number_format;
        return 1;
      }
    case '.':
      if (json_parse_flags_allow_leading_or_trailing_decimal_point &
          state->flags_bitset) {
        return json_get_number_size(state);
      } else {
        // invalid value!
        state->error = json_parse_error_invalid_number_format;
        return 1;
      }
    default:
      if ((state->offset + 4) <= state->size &&
          't' == state->src[state->offset + 0] &&
          'r' == state->src[state->offset + 1] &&
          'u' == state->src[state->offset + 2] &&
          'e' == state->src[state->offset + 3]) {
        state->offset += 4;
        return 0;
      } else if ((state->offset + 5) <= state->size &&
                 'f' == state->src[state->offset + 0] &&
                 'a' == state->src[state->offset + 1] &&
                 'l' == state->src[state->offset + 2] &&
                 's' == state->src[state->offset + 3] &&
                 'e' == state->src[state->offset + 4]) {
        state->offset += 5;
        return 0;
      } else if ((state->offset + 4) <= state->size &&
                 'n' == state->src[state->offset + 0] &&
                 'u' == state->src[state->offset + 1] &&
                 'l' == state->src[state->offset + 2] &&
                 'l' == state->src[state->offset + 3]) {
        state->offset += 4;
        return 0;
      } else if ((json_parse_flags_allow_inf_and_nan & state->flags_bitset) &&
                 (state->offset + 3) <= state->size &&
                 'N' == state->src[state->offset + 0] &&
                 'a' == state->src[state->offset + 1] &&
                 'N' == state->src[state->offset + 2]) {
        return json_get_number_size(state);
      } else if ((json_parse_flags_allow_inf_and_nan & state->flags_bitset) &&
                 (state->offset + 8) <= state->size &&
                 'I' == state->src[state->offset + 0] &&
                 'n' == state->src[state->offset + 1] &&
                 'f' == state->src[state->offset + 2] &&
                 'i' == state->src[state->offset + 3] &&
                 'n' == state->src[state->offset + 4] &&
                 'i' == state->src[state->offset + 5] &&
                 't' == state->src[state->offset + 6] &&
                 'y' == state->src[state->offset + 7]) {
        return json_get_number_size(state);
      }

      // invalid value!
      state->error = json_parse_error_invalid_value;
      return 1;
    }
  }
}

static void json_parse_value(struct json_parse_state_s *state,
                             int is_global_object, struct json_value_s *value);

static void json_parse_string(struct json_parse_state_s *state,
                              struct json_string_s *string) {
  size_t offset = state->offset;
  const size_t size = state->size;
  size_t bytes_written = 0;
  const char quote_to_use = '\'' == state->src[offset] ? '\'' : '"';

  string->string = state->data;

  // skip leading '"' or '\''
  offset++;

  do {
    if ('\\' == state->src[offset]) {
      // skip the reverse solidus
      offset++;

      switch (state->src[offset++]) {
      default:
        return; // we cannot ever reach here
      case '"':
        state->data[bytes_written++] = '"';
        break;
      case '\\':
        state->data[bytes_written++] = '\\';
        break;
      case '/':
        state->data[bytes_written++] = '/';
        break;
      case 'b':
        state->data[bytes_written++] = '\b';
        break;
      case 'f':
        state->data[bytes_written++] = '\f';
        break;
      case 'n':
        state->data[bytes_written++] = '\n';
        break;
      case 'r':
        state->data[bytes_written++] = '\r';
        break;
      case 't':
        state->data[bytes_written++] = '\t';
        break;
      case '\r':
        state->data[bytes_written++] = '\r';

        // check if we have a "\r\n" sequence
        if ('\n' == state->src[offset]) {
          state->data[bytes_written++] = '\n';
          offset++;
        }

        break;
      case '\n':
        state->data[bytes_written++] = '\n';
        break;
      }
    } else {
      // copy the character
      state->data[bytes_written++] = state->src[offset++];
    }
  } while (offset < size && (quote_to_use != state->src[offset]));

  // skip trailing '"' or '\''
  offset++;

  // record the size of the string
  string->string_size = bytes_written;

  // add null terminator to string
  state->data[bytes_written++] = '\0';

  // move data along
  state->data += bytes_written;

  // update offset
  state->offset = offset;
}

static void json_parse_key(struct json_parse_state_s *state,
                           struct json_string_s *string) {
  if (json_parse_flags_allow_unquoted_keys & state->flags_bitset) {
    // if we are allowing unquoted keys, check for quoted anyway...
    if (('"' == state->src[state->offset]) ||
        ('\'' == state->src[state->offset])) {
      // ... if we got a quote, just parse the key as a string as normal
      json_parse_string(state, string);
    } else {
      size_t size = 0;

      string->string = state->data;

      while ((state->offset < state->size) &&
             is_valid_unquoted_key_char(state->src[state->offset])) {
        state->data[size++] = state->src[state->offset++];
      }

      // add null terminator to string
      state->data[size] = '\0';

      // record the size of the string
      string->string_size = size++;

      // move data along
      state->data += size;
    }
  } else {
    // we are only allowed to have quoted keys, so just parse a string!
    json_parse_string(state, string);
  }
}

static void json_parse_object(struct json_parse_state_s *state,
                              int is_global_object,
                              struct json_object_s *object) {
  size_t elements = 0;
  int allow_comma = 0;
  struct json_object_element_s *previous = 0;

  if (is_global_object) {
    // if we skipped some whitespace, and then found an opening '{' of an
    // object, we actually have a normal JSON object at the root of the DOM...
    if ('{' == state->src[state->offset]) {
      // .. and we don't actually have a global object after all!
      is_global_object = 0;
    }
  }

  if (!is_global_object) {
    // skip leading '{'
    state->offset++;
  }

  (void)json_skip_all_skippables(state);

  // reset elements
  elements = 0;

  while (state->offset < state->size) {
    struct json_object_element_s *element = 0;
    struct json_string_s *string = 0;
    struct json_value_s *value = 0;

    if (!is_global_object) {
      (void)json_skip_all_skippables(state);

      if ('}' == state->src[state->offset]) {
        // skip trailing '}'
        state->offset++;

        // finished the object!
        break;
      }
    } else {
      if (json_skip_all_skippables(state)) {
        // global object ends when the file ends!
        break;
      }
    }

    // if we parsed at least one element previously, grok for a comma
    if (allow_comma) {
      if (',' == state->src[state->offset]) {
        // skip comma
        state->offset++;
        allow_comma = 0;
        continue;
      }
    }

    element = (struct json_object_element_s *)state->dom;

    state->dom += sizeof(struct json_object_element_s);

    if (0 == previous) {
      // this is our first element, so record it in our object
      object->start = element;
    } else {
      previous->next = element;
    }

    previous = element;

    if (json_parse_flags_allow_location_information & state->flags_bitset) {
      struct json_string_ex_s *string_ex =
          (struct json_string_ex_s *)state->dom;
      state->dom += sizeof(struct json_string_ex_s);

      string_ex->offset = state->offset;
      string_ex->line_no = state->line_no;
      string_ex->row_no = state->offset - state->line_offset;

      string = &(string_ex->string);
    } else {
      string = (struct json_string_s *)state->dom;
      state->dom += sizeof(struct json_string_s);
    }

    element->name = string;

    (void)json_parse_key(state, string);

    (void)json_skip_all_skippables(state);

    // skip colon or equals
    state->offset++;

    (void)json_skip_all_skippables(state);

    if (json_parse_flags_allow_location_information & state->flags_bitset) {
      struct json_value_ex_s *value_ex = (struct json_value_ex_s *)state->dom;
      state->dom += sizeof(struct json_value_ex_s);

      value_ex->offset = state->offset;
      value_ex->line_no = state->line_no;
      value_ex->row_no = state->offset - state->line_offset;

      value = &(value_ex->value);
    } else {
      value = (struct json_value_s *)state->dom;
      state->dom += sizeof(struct json_value_s);
    }

    element->value = value;

    json_parse_value(state, /* is_global_object = */ 0, value);

    // successfully parsed a name/value pair!
    elements++;
    allow_comma = 1;
  }

  // if we had at least one element, end the linked list
  if (previous) {
    previous->next = 0;
  }

  if (0 == elements) {
    object->start = 0;
  }

  object->length = elements;
}

static void json_parse_array(struct json_parse_state_s *state,
                             struct json_array_s *array) {
  size_t elements = 0;
  int allow_comma = 0;
  struct json_array_element_s *previous = 0;

  // skip leading '['
  state->offset++;

  (void)json_skip_all_skippables(state);

  // reset elements
  elements = 0;

  while (state->offset < state->size) {
    struct json_array_element_s *element = 0;
    struct json_value_s *value = 0;

    (void)json_skip_all_skippables(state);

    if (']' == state->src[state->offset]) {
      // skip trailing ']'
      state->offset++;

      // finished the array!
      break;
    }

    // if we parsed at least one element previously, grok for a comma
    if (allow_comma) {
      if (',' == state->src[state->offset]) {
        // skip comma
        state->offset++;
        allow_comma = 0;
        continue;
      }
    }

    element = (struct json_array_element_s *)state->dom;

    state->dom += sizeof(struct json_array_element_s);

    if (0 == previous) {
      // this is our first element, so record it in our array
      array->start = element;
    } else {
      previous->next = element;
    }

    previous = element;

    if (json_parse_flags_allow_location_information & state->flags_bitset) {
      struct json_value_ex_s *value_ex = (struct json_value_ex_s *)state->dom;
      state->dom += sizeof(struct json_value_ex_s);

      value_ex->offset = state->offset;
      value_ex->line_no = state->line_no;
      value_ex->row_no = state->offset - state->line_offset;

      value = &(value_ex->value);
    } else {
      value = (struct json_value_s *)state->dom;
      state->dom += sizeof(struct json_value_s);
    }

    element->value = value;

    json_parse_value(state, /* is_global_object = */ 0, value);

    // successfully parsed an array element!
    elements++;
    allow_comma = 1;
  }

  // end the linked list
  if (previous) {
    previous->next = 0;
  }

  if (0 == elements) {
    array->start = 0;
  }

  array->length = elements;
}

static void json_parse_number(struct json_parse_state_s *state,
                              struct json_number_s *number) {
  size_t offset = state->offset;
  const size_t size = state->size;
  size_t bytes_written = 0;

  number->number = state->data;

  if (json_parse_flags_allow_hexadecimal_numbers & state->flags_bitset) {
    if (('0' == state->src[offset]) &&
        (('x' == state->src[offset + 1]) || ('X' == state->src[offset + 1]))) {
      // consume hexadecimal digits
      while ((offset < size) &&
             (('0' <= state->src[offset] && state->src[offset] <= '9') ||
              ('a' <= state->src[offset] && state->src[offset] <= 'f') ||
              ('A' <= state->src[offset] && state->src[offset] <= 'F') ||
              ('x' == state->src[offset]) || ('X' == state->src[offset]))) {
        state->data[bytes_written++] = state->src[offset++];
      }
    }
  }

  while (offset < size) {
    int end = 0;

    switch (state->src[offset]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
    case 'e':
    case 'E':
    case '+':
    case '-':
      state->data[bytes_written++] = state->src[offset++];
      break;
    default:
      end = 1;
      break;
    }

    if (0 != end) {
      break;
    }
  }

  if (json_parse_flags_allow_inf_and_nan & state->flags_bitset) {
    const char inf[9] = "Infinity";
    const size_t inf_strlen = sizeof(inf) - 1;
    const char nan[4] = "NaN";
    const size_t nan_strlen = sizeof(nan) - 1;

    if (offset + inf_strlen < size) {
      if ('I' == state->src[offset]) {
        size_t i;
        // We found our special 'Infinity' keyword!
        for (i = 0; i < inf_strlen; i++) {
          state->data[bytes_written++] = state->src[offset++];
        }
      }
    }

    if (offset + nan_strlen < size) {
      if ('N' == state->src[offset]) {
        size_t i;
        // We found our special 'NaN' keyword!
        for (i = 0; i < nan_strlen; i++) {
          state->data[bytes_written++] = state->src[offset++];
        }
      }
    }
  }

  // record the size of the number
  number->number_size = bytes_written;
  // add null terminator to number string
  state->data[bytes_written++] = '\0';
  // move data along
  state->data += bytes_written;
  // update offset
  state->offset = offset;
}

static void json_parse_value(struct json_parse_state_s *state,
                             int is_global_object, struct json_value_s *value) {
  (void)json_skip_all_skippables(state);

  if (is_global_object) {
    value->type = json_type_object;
    value->payload = state->dom;
    state->dom += sizeof(struct json_object_s);
    json_parse_object(state, /* is_global_object = */ 1,
                      (struct json_object_s *)value->payload);
  } else {
    switch (state->src[state->offset]) {
    case '"':
    case '\'':
      value->type = json_type_string;
      value->payload = state->dom;
      state->dom += sizeof(struct json_string_s);
      json_parse_string(state, (struct json_string_s *)value->payload);
      break;
    case '{':
      value->type = json_type_object;
      value->payload = state->dom;
      state->dom += sizeof(struct json_object_s);
      json_parse_object(state, /* is_global_object = */ 0,
                        (struct json_object_s *)value->payload);
      break;
    case '[':
      value->type = json_type_array;
      value->payload = state->dom;
      state->dom += sizeof(struct json_array_s);
      json_parse_array(state, (struct json_array_s *)value->payload);
      break;
    case '-':
    case '+':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      value->type = json_type_number;
      value->payload = state->dom;
      state->dom += sizeof(struct json_number_s);
      json_parse_number(state, (struct json_number_s *)value->payload);
      break;
    default:
      if ((state->offset + 4) <= state->size &&
          't' == state->src[state->offset + 0] &&
          'r' == state->src[state->offset + 1] &&
          'u' == state->src[state->offset + 2] &&
          'e' == state->src[state->offset + 3]) {
        value->type = json_type_true;
        value->payload = 0;
        state->offset += 4;
      } else if ((state->offset + 5) <= state->size &&
                 'f' == state->src[state->offset + 0] &&
                 'a' == state->src[state->offset + 1] &&
                 'l' == state->src[state->offset + 2] &&
                 's' == state->src[state->offset + 3] &&
                 'e' == state->src[state->offset + 4]) {
        value->type = json_type_false;
        value->payload = 0;
        state->offset += 5;
      } else if ((state->offset + 4) <= state->size &&
                 'n' == state->src[state->offset + 0] &&
                 'u' == state->src[state->offset + 1] &&
                 'l' == state->src[state->offset + 2] &&
                 'l' == state->src[state->offset + 3]) {
        value->type = json_type_null;
        value->payload = 0;
        state->offset += 4;
      } else if ((json_parse_flags_allow_inf_and_nan & state->flags_bitset) &&
                 (state->offset + 3) <= state->size &&
                 'N' == state->src[state->offset + 0] &&
                 'a' == state->src[state->offset + 1] &&
                 'N' == state->src[state->offset + 2]) {
        value->type = json_type_number;
        value->payload = state->dom;
        state->dom += sizeof(struct json_number_s);
        json_parse_number(state, (struct json_number_s *)value->payload);
      } else if ((json_parse_flags_allow_inf_and_nan & state->flags_bitset) &&
                 (state->offset + 8) <= state->size &&
                 'I' == state->src[state->offset + 0] &&
                 'n' == state->src[state->offset + 1] &&
                 'f' == state->src[state->offset + 2] &&
                 'i' == state->src[state->offset + 3] &&
                 'n' == state->src[state->offset + 4] &&
                 'i' == state->src[state->offset + 5] &&
                 't' == state->src[state->offset + 6] &&
                 'y' == state->src[state->offset + 7]) {
        value->type = json_type_number;
        value->payload = state->dom;
        state->dom += sizeof(struct json_number_s);
        json_parse_number(state, (struct json_number_s *)value->payload);
      }
      break;
    }
  }
}

struct json_value_s *
json_parse_ex(const void *src, size_t src_size, size_t flags_bitset,
              void *(*alloc_func_ptr)(void *user_data, size_t size),
              void *user_data, struct json_parse_result_s *result) {
  struct json_parse_state_s state;
  void *allocation;
  struct json_value_s *value;
  size_t total_size;
  int input_error;

  if (result) {
    result->error = json_parse_error_none;
    result->error_offset = 0;
    result->error_line_no = 0;
    result->error_row_no = 0;
  }

  if (0 == src) {
    // invalid src pointer was null!
    return 0;
  }

  state.src = (const char *)src;
  state.size = src_size;
  state.offset = 0;
  state.line_no = 1;
  state.line_offset = 0;
  state.error = json_parse_error_none;
  state.dom_size = 0;
  state.data_size = 0;
  state.flags_bitset = flags_bitset;

  input_error = json_get_value_size(
      &state, /* is_global_object = */ (json_parse_flags_allow_global_object &
                                        state.flags_bitset));

  if (0 == input_error) {
    json_skip_all_skippables(&state);

    if (state.offset != state.size) {
      /* our parsing didn't have an error, but there are characters remaining in
       * the input that weren't part of the JSON! */

      state.error = json_parse_error_unexpected_trailing_characters;
      input_error = 1;
    }
  }

  if (input_error) {
    // parsing value's size failed (most likely an invalid JSON DOM!)
    if (result) {
      result->error = state.error;
      result->error_offset = state.offset;
      result->error_line_no = state.line_no;
      result->error_row_no = state.offset - state.line_offset;
    }
    return 0;
  }

  // our total allocation is the combination of the dom and data sizes (we
  // first encode the structure of the JSON, and then the data referenced by
  // the JSON values)
  total_size = state.dom_size + state.data_size;

  if (0 == alloc_func_ptr) {
    allocation = malloc(total_size);
  } else {
    allocation = alloc_func_ptr(user_data, total_size);
  }

  if (0 == allocation) {
    // malloc failed!
    if (result) {
      result->error = json_parse_error_allocator_failed;
      result->error_offset = 0;
      result->error_line_no = 0;
      result->error_row_no = 0;
    }

    return 0;
  }

  // reset offset so we can reuse it
  state.offset = 0;

  // reset the line information so we can reuse it
  state.line_no = 1;
  state.line_offset = 0;

  state.dom = (char *)allocation;
  state.data = state.dom + state.dom_size;

  if (json_parse_flags_allow_location_information & state.flags_bitset) {
    struct json_value_ex_s *value_ex = (struct json_value_ex_s *)state.dom;
    state.dom += sizeof(struct json_value_ex_s);

    value_ex->offset = state.offset;
    value_ex->line_no = state.line_no;
    value_ex->row_no = state.offset - state.line_offset;

    value = &(value_ex->value);
  } else {
    value = (struct json_value_s *)state.dom;
    state.dom += sizeof(struct json_value_s);
  }

  json_parse_value(&state, /* is_global_object = */
                   (json_parse_flags_allow_global_object & state.flags_bitset),
                   value);

  return (struct json_value_s *)allocation;
}

struct json_value_s *json_parse(const void *src, size_t src_size) {
  return json_parse_ex(src, src_size, json_parse_flags_default, 0, 0, 0);
}

static int json_write_minified_get_value_size(const struct json_value_s *value,
                                              size_t *size);

static int json_write_get_number_size(const struct json_number_s *number,
                                      size_t *size) {
  unsigned long long parsed_number;
  size_t i;

  if (number->number_size >= 2) {
    switch (number->number[1]) {
    default:
      break;
    case 'x':
    case 'X':
      // the number is a json_parse_flags_allow_hexadecimal_numbers hexadecimal
      // so we have to do extra work to convert it to a non-hexadecimal for JSON
      // output
      parsed_number = json_strtoull(number->number, 0, 0);

      i = 0;

      while (0 != parsed_number) {
        parsed_number /= 10;
        i++;
      }

      *size += i;
      return 0;
    }
  }

  // check to see if the number has leading/trailing decimal point
  i = 0;

  // skip any leading '+' or '-'
  if ((i < number->number_size) &&
      (('+' == number->number[i]) || ('-' == number->number[i]))) {
    i++;
  }

  // check if we have infinity
  if ((i < number->number_size) && ('I' == number->number[i])) {
    char *inf = "Infinity";
    size_t k;

    for (k = i; k < number->number_size; k++) {
      const char c = *inf++;

      // Check if we found the Infinity string!
      if ('\0' == c) {
        break;
      } else if (c != number->number[k]) {
        break;
      }
    }

    if ('\0' == *inf) {
      // Inf becomes 1.7976931348623158e308 because JSON can't support it
      *size += 22;

      // if we had a leading '-' we need to record it in the JSON output
      if ('-' == number->number[0]) {
        *size += 1;
      }
    }

    return 0;
  }

  // check if we have nan
  if ((i < number->number_size) && ('N' == number->number[i])) {
    char *nan = "NaN";
    size_t k;

    for (k = i; k < number->number_size; k++) {
      const char c = *nan++;

      // Check if we found the NaN string!
      if ('\0' == c) {
        break;
      } else if (c != number->number[k]) {
        break;
      }
    }

    if ('\0' == *nan) {
      // NaN becomes 1 because JSON can't support it
      *size += 1;

      return 0;
    }
  }

  // if we had a leading decimal point
  if ((i < number->number_size) && ('.' == number->number[i])) {
    // 1 + because we had a leading decimal point
    *size += 1;
    goto cleanup;
  }

  for (; i < number->number_size; i++) {
    const char c = number->number[i];
    if (!('0' <= c && c <= '9')) {
      break;
    }
  }

  // if we had a trailing decimal point
  if ((i + 1 == number->number_size) && ('.' == number->number[i])) {
    // 1 + because we had a trailing decimal point
    *size += 1;
    goto cleanup;
  }

cleanup:
  *size += number->number_size; // the actual string of the number

  // if we had a leading '+' we don't record it in the JSON output
  if ('+' == number->number[0]) {
    *size -= 1;
  }

  return 0;
}

static int json_write_get_string_size(const struct json_string_s *string,
                                      size_t *size) {
  size_t i;
  for (i = 0; i < string->string_size; i++) {
    switch (string->string[i]) {
    case '"':
    case '\\':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
      *size += 2;
      break;
    default:
      *size += 1;
      break;
    }
  }

  *size += 2; // need to encode the surrounding '"' characters

  return 0;
}

static int json_write_minified_get_array_size(const struct json_array_s *array,
                                              size_t *size) {
  struct json_array_element_s *element;

  *size += 2; // '[' and ']'

  if (1 < array->length) {
    *size += array->length - 1; // ','s seperate each element
  }

  for (element = array->start; 0 != element; element = element->next) {
    if (json_write_minified_get_value_size(element->value, size)) {
      // value was malformed!
      return 1;
    }
  }

  return 0;
}

static int
json_write_minified_get_object_size(const struct json_object_s *object,
                                    size_t *size) {
  struct json_object_element_s *element;

  *size += 2; // '{' and '}'

  *size += object->length; // ':'s seperate each name/value pair

  if (1 < object->length) {
    *size += object->length - 1; // ','s seperate each element
  }

  for (element = object->start; 0 != element; element = element->next) {
    if (json_write_get_string_size(element->name, size)) {
      // string was malformed!
      return 1;
    }

    if (json_write_minified_get_value_size(element->value, size)) {
      // value was malformed!
      return 1;
    }
  }

  return 0;
}

static int json_write_minified_get_value_size(const struct json_value_s *value,
                                              size_t *size) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 1;
  case json_type_number:
    return json_write_get_number_size((struct json_number_s *)value->payload,
                                      size);
  case json_type_string:
    return json_write_get_string_size((struct json_string_s *)value->payload,
                                      size);
  case json_type_array:
    return json_write_minified_get_array_size(
        (struct json_array_s *)value->payload, size);
  case json_type_object:
    return json_write_minified_get_object_size(
        (struct json_object_s *)value->payload, size);
  case json_type_true:
    *size += 4; // the string "true"
    return 0;
  case json_type_false:
    *size += 5; // the string "false"
    return 0;
  case json_type_null:
    *size += 4; // the string "null"
    return 0;
  }
}

static char *json_write_minified_value(const struct json_value_s *value,
                                       char *data);

static char *json_write_number(const struct json_number_s *number, char *data) {
  unsigned long long parsed_number, backup;
  size_t i;

  if (number->number_size >= 2) {
    switch (number->number[1]) {
    default:
      break;
    case 'x':
    case 'X':
      // The number is a json_parse_flags_allow_hexadecimal_numbers hexadecimal
      // so we have to do extra work to convert it to a non-hexadecimal for JSON
      // output.
      parsed_number = json_strtoull(number->number, 0, 0);

      // We need a copy of parsed number twice, so take a backup of it.
      backup = parsed_number;

      i = 0;

      while (0 != parsed_number) {
        parsed_number /= 10;
        i++;
      }

      // Restore parsed_number to its original value stored in the backup.
      parsed_number = backup;

      // Now use backup to take a copy of i, or the length of the string.
      backup = i;

      do {
        *(data + i - 1) = '0' + (char)(parsed_number % 10);
        parsed_number /= 10;
        i--;
      } while (0 != parsed_number);

      data += backup;

      return data;
    }
  }

  // check to see if the number has leading/trailing decimal point
  i = 0;

  // skip any leading '-'
  if ((i < number->number_size) &&
      (('+' == number->number[i]) || ('-' == number->number[i]))) {
    i++;
  }

  // check if we have infinity
  if ((i < number->number_size) && ('I' == number->number[i])) {
    char *inf = "Infinity";
    size_t k;

    for (k = i; k < number->number_size; k++) {
      const char c = *inf++;

      // Check if we found the Infinity string!
      if ('\0' == c) {
        break;
      } else if (c != number->number[k]) {
        break;
      }
    }

    if ('\0' == *inf++) {
      char *dbl_max;

      // if we had a leading '-' we need to record it in the JSON output
      if ('-' == number->number[0]) {
        *data++ = '-';
      }

      // Inf becomes 1.7976931348623158e308 because JSON can't support it
      for (dbl_max = "1.7976931348623158e308"; '\0' != *dbl_max; dbl_max++) {
        *data++ = *dbl_max;
      }

      return data;
    }
  }

  // check if we have nan
  if ((i < number->number_size) && ('N' == number->number[i])) {
    char *nan = "NaN";
    size_t k;

    for (k = i; k < number->number_size; k++) {
      const char c = *nan++;

      // Check if we found the NaN string!
      if ('\0' == c) {
        break;
      } else if (c != number->number[k]) {
        break;
      }
    }

    if ('\0' == *nan++) {
      // NaN becomes 0 because JSON can't support it
      *data++ = '0';
      return data;
    }
  }

  // if we had a leading decimal point
  if ((i < number->number_size) && ('.' == number->number[i])) {
    i = 0;

    // skip any leading '+'
    if ('+' == number->number[i]) {
      i++;
    }

    // output the leading '-' if we had one
    if ('-' == number->number[i]) {
      *data++ = '-';
      i++;
    }

    // insert a '0' to fix the leading decimal point for JSON output
    *data++ = '0';

    // and output the rest of the number as normal
    for (; i < number->number_size; i++) {
      *data++ = number->number[i];
    }

    return data;
  }

  for (; i < number->number_size; i++) {
    const char c = number->number[i];
    if (!('0' <= c && c <= '9')) {
      break;
    }
  }

  // if we had a trailing decimal point
  if ((i + 1 == number->number_size) && ('.' == number->number[i])) {
    i = 0;

    // skip any leading '+'
    if ('+' == number->number[i]) {
      i++;
    }

    // output the leading '-' if we had one
    if ('-' == number->number[i]) {
      *data++ = '-';
      i++;
    }

    // and output the rest of the number as normal
    for (; i < number->number_size; i++) {
      *data++ = number->number[i];
    }

    // insert a '0' to fix the trailing decimal point for JSON output
    *data++ = '0';

    return data;
  }

  i = 0;

  // skip any leading '+'
  if ('+' == number->number[i]) {
    i++;
  }

  for (; i < number->number_size; i++) {
    *data++ = number->number[i];
  }

  return data;
}

static char *json_write_string(const struct json_string_s *string, char *data) {
  size_t i;

  *data++ = '"'; // open the string

  for (i = 0; i < string->string_size; i++) {
    switch (string->string[i]) {
    case '"':
      *data++ = '\\'; // escape the control character
      *data++ = '"';
      break;
    case '\\':
      *data++ = '\\'; // escape the control character
      *data++ = '\\';
      break;
    case '\b':
      *data++ = '\\'; // escape the control character
      *data++ = 'b';
      break;
    case '\f':
      *data++ = '\\'; // escape the control character
      *data++ = 'f';
      break;
    case '\n':
      *data++ = '\\'; // escape the control character
      *data++ = 'n';
      break;
    case '\r':
      *data++ = '\\'; // escape the control character
      *data++ = 'r';
      break;
    case '\t':
      *data++ = '\\'; // escape the control character
      *data++ = 't';
      break;
    default:
      *data++ = string->string[i];
      break;
    }
  }

  *data++ = '"'; // close the string

  return data;
}

static char *json_write_minified_array(const struct json_array_s *array,
                                       char *data) {
  struct json_array_element_s *element = 0;

  *data++ = '['; // open the array

  for (element = array->start; 0 != element; element = element->next) {
    if (element != array->start) {
      *data++ = ','; // ','s seperate each element
    }

    data = json_write_minified_value(element->value, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  *data++ = ']'; // close the array

  return data;
}

static char *json_write_minified_object(const struct json_object_s *object,
                                        char *data) {
  struct json_object_element_s *element = 0;

  *data++ = '{'; // open the object

  for (element = object->start; 0 != element; element = element->next) {
    if (element != object->start) {
      *data++ = ','; // ','s seperate each element
    }

    data = json_write_string(element->name, data);

    if (0 == data) {
      // string was malformed!
      return 0;
    }

    *data++ = ':'; // ':'s seperate each name/value pair

    data = json_write_minified_value(element->value, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  *data++ = '}'; // close the object

  return data;
}

static char *json_write_minified_value(const struct json_value_s *value,
                                       char *data) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 0;
  case json_type_number:
    return json_write_number((struct json_number_s *)value->payload, data);
  case json_type_string:
    return json_write_string((struct json_string_s *)value->payload, data);
  case json_type_array:
    return json_write_minified_array((struct json_array_s *)value->payload,
                                     data);
  case json_type_object:
    return json_write_minified_object((struct json_object_s *)value->payload,
                                      data);
  case json_type_true:
    data[0] = 't';
    data[1] = 'r';
    data[2] = 'u';
    data[3] = 'e';
    return data + 4;
  case json_type_false:
    data[0] = 'f';
    data[1] = 'a';
    data[2] = 'l';
    data[3] = 's';
    data[4] = 'e';
    return data + 5;
  case json_type_null:
    data[0] = 'n';
    data[1] = 'u';
    data[2] = 'l';
    data[3] = 'l';
    return data + 4;
  }
}

void *json_write_minified(const struct json_value_s *value, size_t *out_size) {
  size_t size = 0;
  char *data = 0;
  char *data_end = 0;

  if (0 == value) {
    return 0;
  }

  if (json_write_minified_get_value_size(value, &size)) {
    // value was malformed!
    return 0;
  }

  size += 1; // for the '\0' null terminating character

  data = (char *)malloc(size);

  if (0 == data) {
    // malloc failed!
    return 0;
  }

  data_end = json_write_minified_value(value, data);

  if (0 == data_end) {
    // bad chi occurred!
    free(data);
    return 0;
  }

  // null terminated the string
  *data_end = '\0';

  if (0 != out_size) {
    *out_size = size;
  }

  return data;
}

static int json_write_pretty_get_value_size(const struct json_value_s *value,
                                            size_t depth, size_t indent_size,
                                            size_t newline_size, size_t *size);

static int json_write_pretty_get_array_size(const struct json_array_s *array,
                                            size_t depth, size_t indent_size,
                                            size_t newline_size, size_t *size) {
  struct json_array_element_s *element;

  *size += 1; // '['

  if (0 < array->length) {
    // if we have any elements we need to add a newline after our '['
    *size += newline_size;

    *size += array->length - 1; // ','s seperate each element

    for (element = array->start; 0 != element; element = element->next) {
      // each element gets an indent
      *size += (depth + 1) * indent_size;

      if (json_write_pretty_get_value_size(element->value, depth + 1,
                                           indent_size, newline_size, size)) {
        // value was malformed!
        return 1;
      }

      // each element gets a newline too
      *size += newline_size;
    }

    // since we wrote out some elements, need to add a newline and indentation
    // to the trailing ']'
    *size += depth * indent_size;
  }

  *size += 1; // ']'

  return 0;
}

static int json_write_pretty_get_object_size(const struct json_object_s *object,
                                             size_t depth, size_t indent_size,
                                             size_t newline_size,
                                             size_t *size) {
  struct json_object_element_s *element;

  *size += 1; // '{'

  if (0 < object->length) {
    *size += newline_size; // need a newline next

    *size += object->length - 1; // ','s seperate each element

    for (element = object->start; 0 != element; element = element->next) {
      // each element gets an indent and newline
      *size += (depth + 1) * indent_size;
      *size += newline_size;

      if (json_write_get_string_size(element->name, size)) {
        // string was malformed!
        return 1;
      }

      *size += 3; // seperate each name/value pair with " : "

      if (json_write_pretty_get_value_size(element->value, depth + 1,
                                           indent_size, newline_size, size)) {
        // value was malformed!
        return 1;
      }
    }

    *size += depth * indent_size;
  }

  *size += 1; // '}'

  return 0;
}

static int json_write_pretty_get_value_size(const struct json_value_s *value,
                                            size_t depth, size_t indent_size,
                                            size_t newline_size, size_t *size) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 1;
  case json_type_number:
    return json_write_get_number_size((struct json_number_s *)value->payload,
                                      size);
  case json_type_string:
    return json_write_get_string_size((struct json_string_s *)value->payload,
                                      size);
  case json_type_array:
    return json_write_pretty_get_array_size(
        (struct json_array_s *)value->payload, depth, indent_size, newline_size,
        size);
  case json_type_object:
    return json_write_pretty_get_object_size(
        (struct json_object_s *)value->payload, depth, indent_size,
        newline_size, size);
  case json_type_true:
    *size += 4; // the string "true"
    return 0;
  case json_type_false:
    *size += 5; // the string "false"
    return 0;
  case json_type_null:
    *size += 4; // the string "null"
    return 0;
  }
}

static char *json_write_pretty_value(const struct json_value_s *value,
                                     size_t depth, const char *indent,
                                     const char *newline, char *data);

static char *json_write_pretty_array(const struct json_array_s *array,
                                     size_t depth, const char *indent,
                                     const char *newline, char *data) {
  size_t k, m;
  struct json_array_element_s *element;

  *data++ = '['; // open the array

  if (0 < array->length) {
    for (k = 0; '\0' != newline[k]; k++) {
      *data++ = newline[k];
    }

    for (element = array->start; 0 != element; element = element->next) {
      if (element != array->start) {
        *data++ = ','; // ','s seperate each element

        for (k = 0; '\0' != newline[k]; k++) {
          *data++ = newline[k];
        }
      }

      for (k = 0; k < depth + 1; k++) {
        for (m = 0; '\0' != indent[m]; m++) {
          *data++ = indent[m];
        }
      }

      data = json_write_pretty_value(element->value, depth + 1, indent, newline,
                                     data);

      if (0 == data) {
        // value was malformed!
        return 0;
      }
    }

    for (k = 0; '\0' != newline[k]; k++) {
      *data++ = newline[k];
    }

    for (k = 0; k < depth; k++) {
      for (m = 0; '\0' != indent[m]; m++) {
        *data++ = indent[m];
      }
    }
  }

  *data++ = ']'; // close the array

  return data;
}

static char *json_write_pretty_object(const struct json_object_s *object,
                                      size_t depth, const char *indent,
                                      const char *newline, char *data) {
  size_t k, m;
  struct json_object_element_s *element;

  *data++ = '{'; // open the object

  if (0 < object->length) {
    for (k = 0; '\0' != newline[k]; k++) {
      *data++ = newline[k];
    }

    for (element = object->start; 0 != element; element = element->next) {
      if (element != object->start) {
        *data++ = ','; // ','s seperate each element

        for (k = 0; '\0' != newline[k]; k++) {
          *data++ = newline[k];
        }
      }

      for (k = 0; k < depth + 1; k++) {
        for (m = 0; '\0' != indent[m]; m++) {
          *data++ = indent[m];
        }
      }

      data = json_write_string(element->name, data);

      if (0 == data) {
        // string was malformed!
        return 0;
      }

      // " : "s seperate each name/value pair
      *data++ = ' ';
      *data++ = ':';
      *data++ = ' ';

      data = json_write_pretty_value(element->value, depth + 1, indent, newline,
                                     data);

      if (0 == data) {
        // value was malformed!
        return 0;
      }
    }

    for (k = 0; '\0' != newline[k]; k++) {
      *data++ = newline[k];
    }

    for (k = 0; k < depth; k++) {
      for (m = 0; '\0' != indent[m]; m++) {
        *data++ = indent[m];
      }
    }
  }

  *data++ = '}'; // close the object

  return data;
}

static char *json_write_pretty_value(const struct json_value_s *value,
                                     size_t depth, const char *indent,
                                     const char *newline, char *data) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 0;
  case json_type_number:
    return json_write_number((struct json_number_s *)value->payload, data);
  case json_type_string:
    return json_write_string((struct json_string_s *)value->payload, data);
  case json_type_array:
    return json_write_pretty_array((struct json_array_s *)value->payload, depth,
                                   indent, newline, data);
  case json_type_object:
    return json_write_pretty_object((struct json_object_s *)value->payload,
                                    depth, indent, newline, data);
  case json_type_true:
    data[0] = 't';
    data[1] = 'r';
    data[2] = 'u';
    data[3] = 'e';
    return data + 4;
  case json_type_false:
    data[0] = 'f';
    data[1] = 'a';
    data[2] = 'l';
    data[3] = 's';
    data[4] = 'e';
    return data + 5;
  case json_type_null:
    data[0] = 'n';
    data[1] = 'u';
    data[2] = 'l';
    data[3] = 'l';
    return data + 4;
  }
}

void *json_write_pretty(const struct json_value_s *value, const char *indent,
                        const char *newline, size_t *out_size) {
  size_t size = 0;
  size_t indent_size = 0;
  size_t newline_size = 0;
  char *data = 0;
  char *data_end = 0;

  if (0 == value) {
    return 0;
  }

  if (0 == indent) {
    indent = "  "; // default to two spaces
  }

  if (0 == newline) {
    newline = "\n"; // default to linux newlines
  }

  while ('\0' != indent[indent_size]) {
    ++indent_size; // skip non-null terminating characters
  }

  while ('\0' != newline[newline_size]) {
    ++newline_size; // skip non-null terminating characters
  }

  if (json_write_pretty_get_value_size(value, 0, indent_size, newline_size,
                                       &size)) {
    // value was malformed!
    return 0;
  }

  size += 1; // for the '\0' null terminating character

  data = (char *)malloc(size);

  if (0 == data) {
    // malloc failed!
    return 0;
  }

  data_end = json_write_pretty_value(value, 0, indent, newline, data);

  if (0 == data_end) {
    // bad chi occurred!
    free(data);
    return 0;
  }

  // null terminated the string
  *data_end = '\0';

  if (0 != out_size) {
    *out_size = size;
  }

  return data;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
