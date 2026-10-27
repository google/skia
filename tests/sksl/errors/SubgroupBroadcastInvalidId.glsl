### Compilation failed:

error: 11: subgroupBroadcast id must be a constant expression
    outputBuffer[x] = subgroupBroadcast(x, lane) +
                                           ^^^^
error: 12: subgroupBroadcast id must be a constant expression
                      subgroupBroadcast(x, x) +
                                           ^
error: 13: subgroupBroadcast id must be a constant expression
                      subgroupBroadcast(x, subgroupBroadcastFirst(uint(3))) +
                                           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: subgroupBroadcast id must be less than 128
                      subgroupBroadcast(x, kTooBig) +
                                           ^^^^^^^
error: 15: subgroupBroadcast id must be less than 128
                      subgroupBroadcast(x, 200);
                                           ^^^
5 errors
