### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '8[%sk_RTAdjust]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %sk_RTAdjust = OpVariable %_ptr_Uniform_v4float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %pos
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_RTAdjust "sk_RTAdjust"
OpName %pos "pos"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %sk_RTAdjust DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%sk_RTAdjust = OpVariable %_ptr_Uniform_v4float Uniform
%_ptr_Input_v4float = OpTypePointer Input %v4float
%pos = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %13
%14 = OpLabel
%15 = OpLoad %v4float %pos
%18 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %18 %15
%20 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%21 = OpLoad %v4float %20
%22 = OpVectorShuffle %v2float %21 %21 0 1
%24 = OpLoad %v4float %sk_RTAdjust
%25 = OpVectorShuffle %v2float %24 %24 0 2
%26 = OpFMul %v2float %22 %25
%27 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%28 = OpLoad %v4float %27
%29 = OpVectorShuffle %v2float %28 %28 3 3
%30 = OpLoad %v4float %sk_RTAdjust
%31 = OpVectorShuffle %v2float %30 %30 1 3
%32 = OpFMul %v2float %29 %31
%33 = OpFAdd %v2float %26 %32
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%37 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%38 = OpLoad %v4float %37
%39 = OpCompositeExtract %float %38 3
%40 = OpCompositeConstruct %v4float %34 %35 %float_0 %39
%41 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %41 %40
OpReturn
OpFunctionEnd

1 error
