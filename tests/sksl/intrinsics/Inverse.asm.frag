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
OpDecorate %a RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
%a = OpVariable %_ptr_Private_mat4v4float Private
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpVariable %_ptr_Function_mat4v4float Function
%19 = OpLoad %mat4v4float %a
%18 = OpExtInst %mat4v4float %1 MatrixInverse %19
OpStore %16 %18
%22 = OpAccessChain %_ptr_Function_v4float %16 %int_0
%24 = OpLoad %v4float %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
