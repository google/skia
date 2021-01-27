OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
OpName %sk_RTAdjust "sk_RTAdjust"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %8 Binding 0
OpDecorate %8 DescriptorSet 0
OpDecorate %sk_RTAdjust DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Private_v4float = OpTypePointer Private %v4float
%sk_RTAdjust = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%14 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %14
%15 = OpLabel
%20 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %20 %17
%22 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%23 = OpLoad %v4float %22
%24 = OpVectorShuffle %v2float %23 %23 0 1
%26 = OpLoad %v4float %sk_RTAdjust
%27 = OpVectorShuffle %v2float %26 %26 0 2
%28 = OpFMul %v2float %24 %27
%29 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%30 = OpLoad %v4float %29
%31 = OpVectorShuffle %v2float %30 %30 3 3
%32 = OpLoad %v4float %sk_RTAdjust
%33 = OpVectorShuffle %v2float %32 %32 1 3
%34 = OpFMul %v2float %31 %33
%35 = OpFAdd %v2float %28 %34
%36 = OpCompositeExtract %float %35 0
%37 = OpCompositeExtract %float %35 1
%39 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%40 = OpLoad %v4float %39
%41 = OpCompositeExtract %float %40 3
%42 = OpCompositeConstruct %v4float %36 %37 %float_0 %41
%43 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %43 %42
OpReturn
OpFunctionEnd
