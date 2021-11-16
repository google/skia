"""
This file contains general helper macros that make our BUILD.bazel files easier to read.
"""

def select_multi(values_map, default, name = ""):
    """select() but allowing multiple matches of the keys.

    select_multi works around a restriction in native select() that prevents multiple
    keys from being matched unless one is a strict subset of another. For some features,
    we allow multiple of that component to be active. For example, with codecs, we let
    the clients mix and match anywhere from 0 built in codecs to all of them.

    select_multi takes a given map and turns it into several distinct select statements
    that have the effect of using any values associated with any active keys.
    For example, if the following parameters are passed in:
        values_map = {
            ":alpha": ["apple", "apricot"],
            ":beta": ["banana"],
            ":gamma": ["grapefruit"],
        },
        default = []
    it will be unrolled into the following select statements
        [] + select({
            ":apple": ["apple", "apricot"],
            "//conditions:default": [],
        }) + select({
            ":beta": ["banana"],
            "//conditions:default": [],
        }) + select({
            ":gamma": ["grapefruit"],
            "//conditions:default": [],
        })

    Args:
        values_map: dictionary of labels to a list of labels, just like select()
        default: list of labels, the value that should be used if any of the options do not match.
            This is typically an empty list
        name: string unused, https://github.com/bazelbuild/buildtools/blob/master/WARNINGS.md#unnamed-macro

    Returns:
        A list of values that is filled in by the generated select statements.
    """
    if len(values_map) == 0:
        return default
    rv = []
    for key, value in values_map.items():
        rv += select({
            key: value,
            "//conditions:default": default,
        })
    return rv
