OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %pos
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %pos "pos"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %sk_RTAdjust DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%pos = OpVariable %_ptr_Input_v4float Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Private_v4float = OpTypePointer Private %v4float
%sk_RTAdjust = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%16 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %16
%17 = OpLabel
%18 = OpLoad %v4float %pos
%21 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %21 %18
%23 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%24 = OpLoad %v4float %23
%25 = OpVectorShuffle %v2float %24 %24 0 1
%27 = OpLoad %v4float %sk_RTAdjust
%28 = OpVectorShuffle %v2float %27 %27 0 2
%29 = OpFMul %v2float %25 %28
%30 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%31 = OpLoad %v4float %30
%32 = OpVectorShuffle %v2float %31 %31 3 3
%33 = OpLoad %v4float %sk_RTAdjust
%34 = OpVectorShuffle %v2float %33 %33 1 3
%35 = OpFMul %v2float %32 %34
%36 = OpFAdd %v2float %29 %35
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%40 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%41 = OpLoad %v4float %40
%42 = OpCompositeExtract %float %41 3
%43 = OpCompositeConstruct %v4float %37 %38 %float_0 %42
%44 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %44 %43
OpReturn
OpFunctionEnd
