# select_multi works around a restriction in native select() that prevents multiple
# keys from being matched unless one is a strict subset of another. For some features,
# we allow multiple of that component to be active. For example, with codecs, we let
# the clients mix and match anywhere from 0 built in codecs to all of them.
#
# select_multi takes a given map and turns it into several distinct select statements
# that have the effect of using any values associated with any active keys.
# For example, if the following parameters are passed in:
#    values_map = {
#        ":alpha": ["apple", "apricot"],
#        ":beta": ["banana"],
#        ":gamma": ["grapefruit"],
#    },
#    default = []
# it will be unrolled into the following select statements
#    [] + select({
#        ":apple": ["apple", "apricot"],
#        "//conditions:default": [],
#    }) + select({
#        ":beta": ["banana"],
#        "//conditions:default": [],
#    }) + select({
#        ":gamma": ["grapefruit"],
#        "//conditions:default": [],
#    })
def select_multi(values_map, default):
    if len(values_map) == 0:
        return default
    rv = []
    for key, value in values_map.items():
        rv += select({
            key: value,
            "//conditions:default": default,
        })
    return rv
