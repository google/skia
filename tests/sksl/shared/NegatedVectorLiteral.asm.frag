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
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%main = OpFunction %void None %11
%12 = OpLabel
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_1
%18 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %18 %float_1
%20 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %20 %float_1
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %22 %float_1
%24 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %24 %float_1
%25 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %25 %float_1
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %26 %float_1
%27 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %27 %float_1
OpReturn
OpFunctionEnd
