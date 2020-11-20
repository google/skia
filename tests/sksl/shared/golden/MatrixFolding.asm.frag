OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_n2 = OpConstant %float -2
%float_3 = OpConstant %float 3
%float_n4 = OpConstant %float -4
%float_5 = OpConstant %float 5
%float_n6 = OpConstant %float -6
%float_7 = OpConstant %float 7
%float_n8 = OpConstant %float -8
%float_9 = OpConstant %float 9
%float_10 = OpConstant %float 10
%float_11 = OpConstant %float 11
%float_12 = OpConstant %float 12
%float_13 = OpConstant %float 13
%float_14 = OpConstant %float 14
%main = OpFunction %void None %11
%12 = OpLabel
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_1
%19 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %19 %float_n2
%21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %21 %float_3
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %23 %float_n4
%25 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %25 %float_5
%27 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %27 %float_n6
%29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %29 %float_7
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %31 %float_n8
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %float_9
%35 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %35 %float_10
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %float_11
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %float_12
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %float_13
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %float_14
OpReturn
OpFunctionEnd
