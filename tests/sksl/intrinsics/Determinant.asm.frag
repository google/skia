OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_n2 = OpConstant %float -2
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%28 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%32 = OpLoad %mat2v2float %28
%27 = OpExtInst %float %1 Determinant %32
%34 = OpFOrdEqual %bool %27 %float_n2
%36 = OpCompositeConstruct %v4bool %34 %34 %34 %34
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%41 = OpLoad %v4float %38
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%44 = OpLoad %v4float %42
%37 = OpSelect %v4float %36 %41 %44
OpReturnValue %37
OpFunctionEnd
