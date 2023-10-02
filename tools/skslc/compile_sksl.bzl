"""
This defines a macro to cut down on boiler plate for generating sksl test resources.
"""

load("//bazel:macros.bzl", "py_binary")

def compile_sksl(name, lang, inputs, settings = "settings"):
    """Creates a rule to compile the given sksl inputs

    This macro creates a py_binary rule to invoke the //gn:compile_sksl_tests.py script and a
    helper genrule which creates the list of all file names that should be processed by that
    python script.

    The generated py_binary rule is called compute_${name} and can be run via `bazel run`.

    Args:
        name: A string used to uniquely identify the generated compile and list rule.
        lang: A string passed into the compile_sksl_tests.py script.
        inputs: A filegroup label containing all the input files to be processed.
        settings: A string passed into the compile_sksl_tests.py script.
    """

    # https://bazel.build/reference/be/python#py_binary
    py_binary(
        name = "compile_" + name,
        main = ":sksl_compile_tests.py",
        srcs = [":sksl_compile_tests.py"],
        args = [
            # comments are the variable names in compile_sksl_tests.py
            "--" + lang,  # lang
            "--" + settings,  # settings
            "resources",  # input_root_dir
            "tests",  # output_root_dir
            "bazel-bin/tools/skslc/%s.txt" % name,  # input_file
        ],
        data = [
            ":%s.txt" % name,
            inputs,
            ":skslc",
            "//gn:compile_sksl_tests",
        ],
        tags = ["no-remote-exec"],
    )

    native.genrule(
        name = "enumerate_%s_list" % name,  # This name does not really matter.
        srcs = [inputs],
        outs = [name + ".txt"],
        # Put a space seperated list of file names into the one output
        # This is done because the list could be quite long and overflow
        # the command line length
        # https://bazel.build/reference/be/make-variables#predefined_genrule_variables
        cmd = "echo $(SRCS) > $@",
    )
