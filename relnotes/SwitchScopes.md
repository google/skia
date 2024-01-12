SkSL variables declared inside of a switch statement will now properly fall out of scope after the
closing brace of the switch-block, as one would expect.

In other words, SkSL code like this will now generate an error:

```
    switch (n) {
        case 1:
            int x = 123;
    }
    return x; // error: unknown identifier 'x'
```

Previously, `x` would remain accessible after the switch's closing brace.
