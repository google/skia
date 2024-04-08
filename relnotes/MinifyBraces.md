The `sksl-minify` tool can now eliminate unnecessary braces. For instance,
given the following SkSL code:

```
if (condition) {
    return 1;
} else {
    return 2;
}
```

The minifier will now emit:

```
if(a)return 1;else return 2;
```
