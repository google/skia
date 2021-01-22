OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %17 RelaxedPrecision
OpDecorate %18 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_float = OpTypePointer Input %float
%a = OpVariable %_ptr_Input_float Input
%b = OpVariable %_ptr_Input_float Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%main = OpFunction %void None %14
%15 = OpLabel
%17 = OpLoad %float %a
%18 = OpLoad %float %b
%16 = OpExtInst %float %1 FMax %17 %18
%19 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %19 %16
OpReturn
OpFunctionEnd
