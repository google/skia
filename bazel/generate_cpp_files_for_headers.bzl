"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

The macro defined in this file allows us to generate .cpp files for header files. The content
is a copy of the header file.

This allows us to actually "compile" the header file, which allows tools like IWYU to properly
analyze a header file on its own.

"""

def generate_cpp_files_for_headers(name, headers):
    """Generate a filegroup containing generate .cpp files for the given header files

    Args:
        name: The name of the filegroup to hold the generated files.
        headers: A list of header files, from this folder), that should have a .cpp file generated
                 for them that includes the header. If a header already has a .cpp file, it should
                 not be in this list. The generated files will not be checked into the Skia repo,
                 they will exist in Bazel's cache folder.
    """
    rules = []
    for hdr in headers:
        cpp = hdr + ".cpp"
        native.genrule(
            name = "gen_" + cpp,
            srcs = headers,
            outs = ["gen/" + cpp],
            # Copy the header as the output .cpp file
            # https://bazel.build/reference/be/make-variables#predefined_genrule_variables
            # execpath returns the path to the given label relative to the Skia root.
            cmd = "cp $(execpath :%s) $@" % hdr,
        )
        rules.append(":gen/" + cpp)

    native.filegroup(
        name = name,
        srcs = rules,
        visibility = ["//tools:__pkg__"],
    )
