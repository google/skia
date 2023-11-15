"""This module defines the remove_indentation macro."""

def remove_indentation(string):
    """Removes indentation from a multiline string.

    This utility function allows us to write multiline templates in a context that requires
    indentation, for example inside a macro. It discards the first and last lines if they only
    contain spaces or tabs. Then, it computes an indentation prefix based on the first remaining
    line and removes that prefix from all lines.

    Example:

    ```
    def greeter_script():
        return remove_indentation('''
            #!/bin/bash
            echo "Hello, {name}!"
        ''').format(name = "world")
    ```

    This is equivalent to:

    ```
    TEMPLATE = '''#!/bin/bash
    echo "Hello, {name}!"
    '''

    def greeter_script():
        return TEMPLATE.format(name = "world")
    ```

    This macro is similar to
    https://github.com/bazelbuild/rules_rust/blob/937e63399b111a6d7ee53b187e4d113300b089e9/rust/private/utils.bzl#L386.

    Args:
        string: A multiline string.
    Returns:
        The input string minus any indentation.
    """

    def get_indentation(line):
        indentation = ""
        for char in line.elems():
            if char in [" ", "\t"]:
                indentation += char
            else:
                break

        # For some reason Buildifier thinks the below variable is uninitialized.
        # buildifier: disable=uninitialized
        return indentation

    lines = string.split("\n")

    # Skip first line if empty.
    if get_indentation(lines[0]) == lines[0]:
        lines = lines[1:]

    # Compute indentation based on the first remaining line, and remove indentation from all lines.
    indentation = get_indentation(lines[0])
    lines = [line.removeprefix(indentation) for line in lines]

    # Skip last line if empty.
    if get_indentation(lines[len(lines) - 1]) == lines[len(lines) - 1]:
        lines = lines[:-1]

    result = "\n".join(lines)
    if result[:-1] != "\n":
        # Ensure we always end with a newline.
        result += "\n"

    return result
