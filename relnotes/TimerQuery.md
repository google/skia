`GrGLInterface` completeness requirements are modified to support using timer queries when available in the GL context.
The interface must have relevant functions initialized on OpenGL 3.3 or with GL_EXT_timer_query or GL_ARB_timerquery, on OpenGL ES with
GL_EXT_disjoint_timer_query, and on WebGL with GL_EXT_disjoint_timer_query or GL_EXT_disjoint_timer_query_webgl2.