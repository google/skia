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
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
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
%36 = OpConstantComposite %v2float %float_0 %float_0
%v3float = OpTypeVector %float 3
%54 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%main = OpFunction %void None %19
%20 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %one
%23 = OpImageSampleImplicitLod %v4float %24 %float_0 Bias %float_n0_5
%27 = OpCompositeExtract %float %23 0
%28 = OpCompositeExtract %float %23 1
%29 = OpCompositeExtract %float %23 2
%30 = OpCompositeExtract %float %23 3
%31 = OpCompositeConstruct %v4float %27 %28 %29 %30
OpStore %a %31
%34 = OpLoad %16 %two
%33 = OpImageSampleImplicitLod %v4float %34 %36 Bias %float_n0_5
%37 = OpCompositeExtract %float %33 0
%38 = OpCompositeExtract %float %33 1
%39 = OpCompositeExtract %float %33 2
%40 = OpCompositeExtract %float %33 3
%41 = OpCompositeConstruct %v4float %37 %38 %39 %40
OpStore %b %41
%44 = OpLoad %12 %one
%43 = OpImageSampleProjImplicitLod %v4float %44 %36 Bias %float_n0_5
%45 = OpCompositeExtract %float %43 0
%46 = OpCompositeExtract %float %43 1
%47 = OpCompositeExtract %float %43 2
%48 = OpCompositeExtract %float %43 3
%49 = OpCompositeConstruct %v4float %45 %46 %47 %48
OpStore %c %49
%52 = OpLoad %16 %two
%51 = OpImageSampleProjImplicitLod %v4float %52 %54 Bias %float_n0_5
%55 = OpCompositeExtract %float %51 0
%56 = OpCompositeExtract %float %51 1
%57 = OpCompositeExtract %float %51 2
%58 = OpCompositeExtract %float %51 3
%59 = OpCompositeConstruct %v4float %55 %56 %57 %58
OpStore %d %59
%60 = OpLoad %v4float %a
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v4float %b
%63 = OpCompositeExtract %float %62 0
%64 = OpLoad %v4float %c
%65 = OpCompositeExtract %float %64 0
%66 = OpLoad %v4float %d
%67 = OpCompositeExtract %float %66 0
%68 = OpCompositeConstruct %v4float %61 %63 %65 %67
OpStore %sk_FragColor %68
OpReturn
OpFunctionEnd

1 error
