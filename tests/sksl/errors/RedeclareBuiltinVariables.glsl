### Compilation failed:

error: 3: layout qualifier 'builtin' is not permitted here
layout(builtin=15) in float4 sk_FragCoord;
^^^^^^^^^^^^^^^^^^^^^
error: 3: symbol 'sk_FragCoord' was already defined
layout(builtin=15) in float4 sk_FragCoord;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: layout qualifier 'builtin' is not permitted here
layout(builtin=17) in bool sk_Clockwise;  // Similar to gl_FrontFacing, but defined in device space.
^^^^^^^^^^^^^^^^^^^^^
error: 4: symbol 'sk_Clockwise' was already defined
layout(builtin=17) in bool sk_Clockwise;  // Similar to gl_FrontFacing, but defined in device space.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: layout qualifier 'builtin' is not permitted here
layout(location=0,index=0,builtin=10001) out half4 sk_FragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: symbol 'sk_FragColor' was already defined
layout(location=0,index=0,builtin=10001) out half4 sk_FragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: layout qualifier 'builtin' is not permitted here
layout(builtin=10008) half4 sk_LastFragColor;
^^^^^^^^^^^^^^^^^^^^^
error: 7: symbol 'sk_LastFragColor' was already defined
layout(builtin=10008) half4 sk_LastFragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: layout qualifier 'builtin' is not permitted here
layout(builtin=10012) out half4 sk_SecondaryFragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: symbol 'sk_SecondaryFragColor' was already defined
layout(builtin=10012) out half4 sk_SecondaryFragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: layout qualifier 'builtin' is not permitted here
layout (builtin=123) in float mysteryIn;
^^^^^^^^^^^^^^^^^^^^^^^
error: 12: layout qualifier 'builtin' is not permitted here
layout (builtin=456) out float mysteryOut;
^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: layout qualifier 'builtin' is not permitted here
layout (builtin=789) float mysteryGlobal;
^^^^^^^^^^^^^^^^^^^^
13 errors
