OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %17 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4bool = OpTypeVector %bool 4
%_ptr_Private_v4bool = OpTypePointer Private %v4bool
%a = OpVariable %_ptr_Private_v4bool Private
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %14
%15 = OpLabel
%17 = OpLoad %v4bool %a
%16 = OpAll %bool %17
%18 = OpSelect %int %16 %int_1 %int_0
%22 = OpConvertSToF %float %18
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %23 %22
OpReturn
OpFunctionEnd
