OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %testBlock "testBlock"
OpMemberName %testBlock 0 "x"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpMemberDecorate %testBlock 0 Offset 0
OpDecorate %testBlock Block
OpDecorate %3 Binding 456
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%testBlock = OpTypeStruct %float
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
%3 = OpVariable %_ptr_Uniform_testBlock Uniform
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
%main = OpFunction %void None %14
%15 = OpLabel
%18 = OpAccessChain %_ptr_Uniform_float %3 %int_0
%20 = OpLoad %float %18
%21 = OpCompositeConstruct %v4float %20 %20 %20 %20
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
