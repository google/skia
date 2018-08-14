# json.h #

[![Build status](https://ci.appveyor.com/api/projects/status/piell6hcrlwrcxp9?svg=true)](https://ci.appveyor.com/project/sheredom/json-h)

[![Build status](https://api.travis-ci.org/repositories/sheredom/json.h.svg)](https://travis-ci.org/sheredom/json.h)

A simple one header/one source solution to parsing JSON in C and C++.

JSON is parsed into a read-only, single allocation buffer.

The current supported compilers are gcc, clang and msvc.

The current supported platforms are Windows, Mac OSX and Linux.

## Usage ##

Just include json.h, json.c in your code!

### json_parse ###

Parse a json string into a DOM.

```
struct json_value_s *json_parse(
    const void *src,
    size_t src_size);
```

- `src` - a utf-8 json string to parse.
- `src_size` - the size of `src` in bytes.

Returns a `struct json_value_s*` pointing the root of the json DOM.

### struct json_value_s ###

The main struct for interacting with a parsed JSON Document Object Model (DOM) is the `struct json_value_s`.

```
struct json_value_s {
  void *payload;
  size_t type;
};
```

- `payload` - a pointer to the contents of the value.
- `type` - the type of struct `payload` points to, one of `json_type_e`. Note: if type is `json_type_true`, `json_type_false`, or `json_type_null`, payload will be NULL.

For example, if we had the JSON string *'{"a" : true, "b" : [false, null, "foo"]}'*, to get to each part of the parsed JSON we'd do:

```
const char json[] = "{\"a\" : true, \"b\" : [false, null, \"foo\"]}";
struct json_value_s* root = json_parse(json, strlen(json));
/* root->type is json_type_object */

struct json_object_s* object = (struct json_object_s*)root->payload;
/* object->length is 2 */

struct json_object_element_s* a = object->start;

struct json_string_s* a_name = a->name;
/* a_name->string is "a" */
/* a_name->string_size is strlen("a") */

struct json_value_s* a_value = a->value;
/* a_value->type is json_type_true */
/* a_value->payload is NULL */

struct json_object_element_s* b = a->next;
/* b->next is NULL */

struct json_string_s* b_name = b->name;
/* b_name->string is "b" */
/* b_name->string_size is strlen("b") */

struct json_value_s* b_value = b->value;
/* b_value->type is json_type_array */

struct json_array_s* array = (struct json_array_s*)b_value->payload;
/* array->length is 3 */

struct json_array_element_s* b_1st = array->start;

struct json_value_s* b_1st_value = b_1st->value;
/* b_1st_value->type is json_type_false */
/* b_1st_value->payload is NULL */

struct json_array_element_s* b_2nd = b_1st->next;

struct json_value_s* b_2nd_value = b_2nd->value;
/* b_2nd_value->type is json_type_null */
/* b_2nd_value->payload is NULL */

struct json_array_element_s* b_3rd = b_2nd->next;
/* b_3rd->next is NULL */

struct json_value_s* b_3rd_value = b_3rd->value;
/* b_3rd_value->type is json_type_string */

struct json_string_s* string = (struct json_string_s*)b_3rd_value->payload;
/* string->string is "foo" */
/* string->string_size is strlen("foo") */

/* Don't forget to free the one allocation! */
free(root);
```

### json_parse_ex ###

Extended parse a json string into a DOM.

```
struct json_value_s *json_parse_ex(
    const void *src,
    size_t src_size,
    size_t flags_bitset,
    void*(*alloc_func_ptr)(void *, size_t),
    void *user_data,
    struct json_parse_result_s *result);
```

- `src` - a utf-8 json string to parse.
- `src_size` - the size of `src` in bytes.
- `flags_bitset` - extra parsing flags, a bitset of flags specified in `enum json_parse_flags_e`.
- `alloc_func_ptr` - a callback function to use for doing the single allocation. If NULL, `malloc()` is used.
- `user_data` - user data to be passed as the first argument to `alloc_func_ptr`.
- `result` - the result of the parsing. If a parsing error occured this will contain what type of error, and where in the source it occured. Can be NULL.

Returns a `struct json_value_s*` pointing the root of the json DOM.

### enum json_parse_flags_e ###

The extra parsing flags that can be specified to `json_parse_ex()` are as follows:

```
enum json_parse_flags_e {
  json_parse_flags_default = 0,
  json_parse_flags_allow_trailing_comma = 0x1,
  json_parse_flags_allow_unquoted_keys = 0x2,
  json_parse_flags_allow_global_object = 0x4,
  json_parse_flags_allow_equals_in_object = 0x8,
  json_parse_flags_allow_no_commas = 0x10,
  json_parse_flags_allow_c_style_comments = 0x20,
  json_parse_flags_deprecated = 0x40,
  json_parse_flags_allow_location_information = 0x80,
  json_parse_flags_allow_simplified_json =
      (json_parse_flags_allow_trailing_comma |
       json_parse_flags_allow_unquoted_keys |
       json_parse_flags_allow_global_object |
       json_parse_flags_allow_equals_in_object |
       json_parse_flags_allow_no_commas)
};
```

- `json_parse_flags_default` - the default, no special behaviour is enabled.
- `json_parse_flags_allow_trailing_comma` - allow trailing commas in objects and arrays. For example, both `[true,]` and `{"a" : null,}` would be allowed with this option on.
- `json_parse_flags_allow_unquoted_keys` - allow unquoted keys for objects. For example, `{a : null}` would be allowed with this option on.
- `json_parse_flags_allow_global_object` - allow a global unbracketed object. For example, `a : null, b : true, c : {}` would be allowed with this option on.
- `json_parse_flags_allow_equals_in_object` - allow objects to use '=' as well as ':' between key/value pairs. For example, `{"a" = null, "b" : true}` would be allowed with this option on.
- `json_parse_flags_allow_no_commas` - allow that objects don't have to have comma separators between key/value pairs. For example, `{"a" : null "b" : true}` would be allowed with this option on.
- `json_parse_flags_allow_c_style_comments` - allow c-style comments (// or /* */) to be ignored in the input JSON file.
- `json_parse_flags_deprecated` - a deprecated option.
- `json_parse_flags_allow_location_information` - allow location information to be tracked for where values are in the input JSON. Useful for alerting users to errors with precise location information pertaining to the original source. When this option is enabled, all `json_value_s*`'s can be casted to `json_value_ex_s*`, and the `json_string_s*` of `json_object_element_s*`'s name member can be casted to `json_string_ex_s*` to retrieve specific locations on all the values and keys. Note this option will increase the memory budget required for the DOM used to record the JSON.
- `json_parse_flags_allow_simplified_json` - allow simplified JSON to be parsed. Simplified JSON is an enabling of a set of other parsing options. [See the Bitsquid blog introducing this here.](http://bitsquid.blogspot.com/2009/10/simplified-json-notation.html)

## Design ##

The json_parse function calls malloc once, and then slices up this single
allocation to support all the weird and wonderful JSON structures you can
imagine!

The structure of the data is always the JSON structs first (which encode the
structure of the original JSON), followed by the data.

## Todo ##

- Add debug output to specify why the printer failed (as suggested by [@hugin84](https://twitter.com/hugin84) in https://twitter.com/hugin84/status/668506811595677696)

## License ##

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
