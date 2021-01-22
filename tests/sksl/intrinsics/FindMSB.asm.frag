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
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%a = OpVariable %_ptr_Input_int Input
%uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%b = OpVariable %_ptr_Input_uint Input
%void = OpTypeVoid
%17 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%main = OpFunction %void None %17
%18 = OpLabel
%20 = OpLoad %int %a
%19 = OpExtInst %int %1 FindSMsb %20
%21 = OpConvertSToF %float %19
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %22 %21
%26 = OpLoad %uint %b
%25 = OpExtInst %int %1 FindUMsb %26
%27 = OpConvertSToF %float %25
%28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %28 %27
OpReturn
OpFunctionEnd
