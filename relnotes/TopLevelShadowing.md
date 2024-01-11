SkSL will now properly report an error if a function contains a top-level variable with the same
name as a function parameter. SkSL intends to match the scoping rules of GLSL, in particular: "A
function’s parameter declarations and body together form a single scope nested in the global scope."

A program like this will now be rejected:

```
    void func(int var) {
        int var;
    }

    error: 2: symbol 'var' was already defined
        int var;
        ^^^^^^^
```
