### Compilation failed:

error: 9: 'subgroupBallot' not supported
    uint4 result = subgroupBallot(b);
                   ^^^^^^^^^^^^^^^^^
error: 10: 'subgroupAll' not supported
    result.x += uint(subgroupAll(b));
                     ^^^^^^^^^^^^^^
error: 11: 'subgroupAny' not supported
    result.y += uint(subgroupAny(b));
                     ^^^^^^^^^^^^^^
error: 12: 'subgroupBroadcast' not supported
    result.z += subgroupBroadcast(sk_GlobalInvocationID.x, 0);
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: 'subgroupBroadcastFirst' not supported
    result.w += subgroupBroadcastFirst(sk_GlobalInvocationID.x);
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
5 errors
