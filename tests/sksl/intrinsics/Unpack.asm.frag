OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %20 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%a = OpVariable %_ptr_Input_uint Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%main = OpFunction %void None %14
%15 = OpLabel
%17 = OpLoad %uint %a
%16 = OpExtInst %v2float %1 UnpackHalf2x16 %17
%19 = OpLoad %v4float %sk_FragColor
%20 = OpVectorShuffle %v4float %19 %16 4 5 2 3
OpStore %sk_FragColor %20
%22 = OpLoad %uint %a
%21 = OpExtInst %v2float %1 UnpackUnorm2x16 %22
%23 = OpLoad %v4float %sk_FragColor
%24 = OpVectorShuffle %v4float %23 %21 4 5 2 3
OpStore %sk_FragColor %24
%26 = OpLoad %uint %a
%25 = OpExtInst %v2float %1 UnpackSnorm2x16 %26
%27 = OpLoad %v4float %sk_FragColor
%28 = OpVectorShuffle %v4float %27 %25 4 5 2 3
OpStore %sk_FragColor %28
%30 = OpLoad %uint %a
%29 = OpExtInst %v4float %1 UnpackUnorm4x8 %30
OpStore %sk_FragColor %29
%32 = OpLoad %uint %a
%31 = OpExtInst %v4float %1 UnpackSnorm4x8 %32
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
