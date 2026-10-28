### Compilation failed:

error: 9: 'subgroupAdd' not supported
    outputBuffer[x] = subgroupAdd(x) + subgroupMin(x) + subgroupMax(x) +
                      ^^^^^^^^^^^^^^
error: 9: 'subgroupMin' not supported
    outputBuffer[x] = subgroupAdd(x) + subgroupMin(x) + subgroupMax(x) +
                                       ^^^^^^^^^^^^^^
error: 9: 'subgroupMax' not supported
    outputBuffer[x] = subgroupAdd(x) + subgroupMin(x) + subgroupMax(x) +
                                                        ^^^^^^^^^^^^^^
error: 10: 'subgroupInclusiveAdd' not supported
                      subgroupInclusiveAdd(x) + subgroupExclusiveAdd(x);
                      ^^^^^^^^^^^^^^^^^^^^^^^
error: 10: 'subgroupExclusiveAdd' not supported
                      subgroupInclusiveAdd(x) + subgroupExclusiveAdd(x);
                                                ^^^^^^^^^^^^^^^^^^^^^^^
5 errors
