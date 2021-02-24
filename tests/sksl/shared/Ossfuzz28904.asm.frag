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
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%18 = OpConstantComposite %v2int %int_2 %int_1
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%26 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %11
%12 = OpLabel
%13 = OpSNegate %v2int %18
%20 = OpSNegate %v2int %18
%19 = OpSNegate %v2int %20
%21 = OpIEqual %v2bool %13 %19
%23 = OpAll %bool %21
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
