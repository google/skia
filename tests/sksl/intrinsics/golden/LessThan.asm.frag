OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b %c %d %e %f
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%a = OpVariable %_ptr_Input_v4float Input
%b = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%v2uint = OpTypeVector %uint 2
%_ptr_Input_v2uint = OpTypePointer Input %v2uint
%c = OpVariable %_ptr_Input_v2uint Input
%d = OpVariable %_ptr_Input_v2uint Input
%int = OpTypeInt 32 1
%v3int = OpTypeVector %int 3
%_ptr_Input_v3int = OpTypePointer Input %v3int
%e = OpVariable %_ptr_Input_v3int Input
%f = OpVariable %_ptr_Input_v3int Input
%void = OpTypeVoid
%24 = OpTypeFunction %void
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%int_2 = OpConstant %int 2
%main = OpFunction %void None %24
%25 = OpLabel
%28 = OpLoad %v4float %a
%29 = OpLoad %v4float %b
%27 = OpFOrdLessThan %v4bool %28 %29
%31 = OpCompositeExtract %bool %27 0
%32 = OpSelect %int %31 %int_1 %int_0
%26 = OpConvertSToF %float %32
%35 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %35 %26
%39 = OpLoad %v2uint %c
%40 = OpLoad %v2uint %d
%38 = OpULessThan %v2bool %39 %40
%42 = OpCompositeExtract %bool %38 1
%43 = OpSelect %int %42 %int_1 %int_0
%37 = OpConvertSToF %float %43
%44 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %44 %37
%47 = OpLoad %v3int %e
%48 = OpLoad %v3int %f
%46 = OpSLessThan %v3bool %47 %48
%50 = OpCompositeExtract %bool %46 2
%51 = OpSelect %int %50 %int_1 %int_0
%45 = OpConvertSToF %float %51
%52 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %52 %45
OpReturn
OpFunctionEnd
