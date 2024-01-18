SkSL now properly reports an error if the body of a for-loop declares a variable which shadows the
for-loop induction variable.

In other words, SkSL code like this will now generate an error:

```
    for (int x = 0; x < 10; ++x) {
        int x = 123;  // error: symbol 'x' was already defined
    }
```

Previously, the declaration of `x` would be allowed, in violation of the GLSL scoping rules (6.3):
"For both for and while loops, the sub-statement does not introduce a new scope for variable names."
