### Compilation failed:

error: SPIR-V validation error: Operand 3 of TypeImage requires one of these capabilities: Sampled1D Image1D 
  %13 = OpTypeImage %float 1D 0 0 0 1 Unknown

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %one "one"
OpName %two "two"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %one RelaxedPrecision
OpDecorate %one Binding 0
OpDecorate %one DescriptorSet 0
OpDecorate %two RelaxedPrecision
OpDecorate %two Binding 1
OpDecorate %two DescriptorSet 0
OpDecorate %24 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 1D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%one = OpVariable %_ptr_UniformConstant_12 UniformConstant
%17 = OpTypeImage %float 2D 0 0 0 1 Unknown
%16 = OpTypeSampledImage %17
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%two = OpVariable %_ptr_UniformConstant_16 UniformConstant
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%float_n0_5 = OpConstant %float -0.5
%v2float = OpTypeVector %float 2
%31 = OpConstantComposite %v2float %float_0 %float_0
%v3float = OpTypeVector %float 3
%39 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%main = OpFunction %void None %19
%20 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %one
%23 = OpImageSampleImplicitLod %v4float %24 %float_0 Bias %float_n0_5
OpStore %a %23
%29 = OpLoad %16 %two
%28 = OpImageSampleImplicitLod %v4float %29 %31 Bias %float_n0_5
OpStore %b %28
%34 = OpLoad %12 %one
%33 = OpImageSampleProjImplicitLod %v4float %34 %31 Bias %float_n0_5
OpStore %c %33
%37 = OpLoad %16 %two
%36 = OpImageSampleProjImplicitLod %v4float %37 %39 Bias %float_n0_5
OpStore %d %36
%40 = OpLoad %v4float %a
%41 = OpCompositeExtract %float %40 0
%42 = OpLoad %v4float %b
%43 = OpCompositeExtract %float %42 0
%44 = OpLoad %v4float %c
%45 = OpCompositeExtract %float %44 0
%46 = OpLoad %v4float %d
%47 = OpCompositeExtract %float %46 0
%48 = OpCompositeConstruct %v4float %41 %43 %45 %47
OpStore %sk_FragColor %48
OpReturn
OpFunctionEnd

1 error
