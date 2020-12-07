### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '8[%sk_RTAdjust]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %sk_RTAdjust = OpVariable %_ptr_Uniform_v4float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_RTAdjust "sk_RTAdjust"
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
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%main = OpFunction %void None %11
%12 = OpLabel
%17 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %17 %13
%19 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%20 = OpLoad %v4float %19
%21 = OpVectorShuffle %v2float %20 %20 0 1
%23 = OpLoad %v4float %sk_RTAdjust
%24 = OpVectorShuffle %v2float %23 %23 0 2
%25 = OpFMul %v2float %21 %24
%26 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%27 = OpLoad %v4float %26
%28 = OpVectorShuffle %v2float %27 %27 3 3
%29 = OpLoad %v4float %sk_RTAdjust
%30 = OpVectorShuffle %v2float %29 %29 1 3
%31 = OpFMul %v2float %28 %30
%32 = OpFAdd %v2float %25 %31
%33 = OpCompositeExtract %float %32 0
%34 = OpCompositeExtract %float %32 1
%36 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%37 = OpLoad %v4float %36
%38 = OpCompositeExtract %float %37 3
%39 = OpCompositeConstruct %v4float %33 %34 %float_0 %38
%40 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %40 %39
OpReturn
OpFunctionEnd

1 error
