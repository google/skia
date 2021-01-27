### Compilation failed:

error: SPIR-V validation error: ID 123456[%123456] has not been defined
  %28 = OpLoad %mat4v4float %123456

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %s "s"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorXform"
OpName %main "main"
OpName %tmpColor "tmpColor"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %s RelaxedPrecision
OpDecorate %s Binding 0
OpDecorate %s DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%s = OpVariable %_ptr_UniformConstant_12 UniformConstant
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%27 = OpConstantComposite %v2float %float_1 %float_1
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %19
%20 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%55 = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %s
%23 = OpImageSampleImplicitLod %v4float %24 %27
OpStore %tmpColor %23
%28 = OpLoad %mat4v4float %123456
%31 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%32 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%33 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%34 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%29 = OpCompositeConstruct %mat4v4float %31 %32 %33 %34
%36 = OpCompositeExtract %v4float %28 0
%37 = OpCompositeExtract %v4float %29 0
%38 = OpFOrdNotEqual %v4bool %36 %37
%39 = OpAny %bool %38
%40 = OpCompositeExtract %v4float %28 1
%41 = OpCompositeExtract %v4float %29 1
%42 = OpFOrdNotEqual %v4bool %40 %41
%43 = OpAny %bool %42
%44 = OpLogicalOr %bool %39 %43
%45 = OpCompositeExtract %v4float %28 2
%46 = OpCompositeExtract %v4float %29 2
%47 = OpFOrdNotEqual %v4bool %45 %46
%48 = OpAny %bool %47
%49 = OpLogicalOr %bool %44 %48
%50 = OpCompositeExtract %v4float %28 3
%51 = OpCompositeExtract %v4float %29 3
%52 = OpFOrdNotEqual %v4bool %50 %51
%53 = OpAny %bool %52
%54 = OpLogicalOr %bool %49 %53
OpSelectionMerge %58 None
OpBranchConditional %54 %56 %57
%56 = OpLabel
%60 = OpLoad %mat4v4float %123456
%61 = OpLoad %v4float %tmpColor
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%64 = OpCompositeExtract %float %62 0
%65 = OpCompositeExtract %float %62 1
%66 = OpCompositeExtract %float %62 2
%67 = OpCompositeConstruct %v4float %64 %65 %66 %float_1
%68 = OpMatrixTimesVector %v4float %60 %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%71 = OpLoad %v4float %tmpColor
%72 = OpCompositeExtract %float %71 3
%73 = OpCompositeConstruct %v3float %72 %72 %72
%59 = OpExtInst %v3float %1 FClamp %69 %70 %73
%74 = OpCompositeExtract %float %59 0
%75 = OpCompositeExtract %float %59 1
%76 = OpCompositeExtract %float %59 2
%77 = OpLoad %v4float %tmpColor
%78 = OpCompositeExtract %float %77 3
%79 = OpCompositeConstruct %v4float %74 %75 %76 %78
OpStore %55 %79
OpBranch %58
%57 = OpLabel
%80 = OpLoad %v4float %tmpColor
OpStore %55 %80
OpBranch %58
%58 = OpLabel
%81 = OpLoad %v4float %55
OpStore %sk_FragColor %81
OpReturn
OpFunctionEnd

1 error
