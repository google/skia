### Compilation failed:

error: 3: symbol 'sk_FragCoord' was already defined
layout(builtin=15) in float4 sk_FragCoord;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: symbol 'sk_Clockwise' was already defined
layout(builtin=17) in bool sk_Clockwise;  // Similar to gl_FrontFacing, but defined in device space.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: symbol 'sk_FragColor' was already defined
layout(location=0,index=0,builtin=10001) out half4 sk_FragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: symbol 'sk_LastFragColor' was already defined
layout(builtin=10008) half4 sk_LastFragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: symbol 'sk_SecondaryFragColor' was already defined
layout(builtin=10012) out half4 sk_SecondaryFragColor;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
5 errors
