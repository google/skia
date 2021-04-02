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
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
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
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %19
%20 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%64 = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %12 %s
%23 = OpImageSampleImplicitLod %v4float %24 %27
%28 = OpCompositeExtract %float %23 0
%29 = OpCompositeExtract %float %23 1
%30 = OpCompositeExtract %float %23 2
%31 = OpCompositeExtract %float %23 3
%32 = OpCompositeConstruct %v4float %28 %29 %30 %31
OpStore %tmpColor %32
%33 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%37 = OpLoad %mat4v4float %33
%40 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%41 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%42 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%43 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%38 = OpCompositeConstruct %mat4v4float %40 %41 %42 %43
%45 = OpCompositeExtract %v4float %37 0
%46 = OpCompositeExtract %v4float %38 0
%47 = OpFOrdNotEqual %v4bool %45 %46
%48 = OpAny %bool %47
%49 = OpCompositeExtract %v4float %37 1
%50 = OpCompositeExtract %v4float %38 1
%51 = OpFOrdNotEqual %v4bool %49 %50
%52 = OpAny %bool %51
%53 = OpLogicalOr %bool %48 %52
%54 = OpCompositeExtract %v4float %37 2
%55 = OpCompositeExtract %v4float %38 2
%56 = OpFOrdNotEqual %v4bool %54 %55
%57 = OpAny %bool %56
%58 = OpLogicalOr %bool %53 %57
%59 = OpCompositeExtract %v4float %37 3
%60 = OpCompositeExtract %v4float %38 3
%61 = OpFOrdNotEqual %v4bool %59 %60
%62 = OpAny %bool %61
%63 = OpLogicalOr %bool %58 %62
OpSelectionMerge %67 None
OpBranchConditional %63 %65 %66
%65 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%70 = OpLoad %mat4v4float %69
%71 = OpLoad %v4float %tmpColor
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%74 = OpCompositeExtract %float %72 0
%75 = OpCompositeExtract %float %72 1
%76 = OpCompositeExtract %float %72 2
%77 = OpCompositeConstruct %v4float %74 %75 %76 %float_1
%78 = OpMatrixTimesVector %v4float %70 %77
%79 = OpVectorShuffle %v3float %78 %78 0 1 2
%80 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%81 = OpLoad %v4float %tmpColor
%82 = OpCompositeExtract %float %81 3
%83 = OpCompositeConstruct %v3float %82 %82 %82
%68 = OpExtInst %v3float %1 FClamp %79 %80 %83
%84 = OpCompositeExtract %float %68 0
%85 = OpCompositeExtract %float %68 1
%86 = OpCompositeExtract %float %68 2
%87 = OpLoad %v4float %tmpColor
%88 = OpCompositeExtract %float %87 3
%89 = OpCompositeConstruct %v4float %84 %85 %86 %88
OpStore %64 %89
OpBranch %67
%66 = OpLabel
%90 = OpLoad %v4float %tmpColor
OpStore %64 %90
OpBranch %67
%67 = OpLabel
%91 = OpLoad %v4float %64
%92 = OpCompositeExtract %float %91 0
%93 = OpCompositeExtract %float %91 1
%94 = OpCompositeExtract %float %91 2
%95 = OpCompositeExtract %float %91 3
%96 = OpCompositeConstruct %v4float %92 %93 %94 %95
OpStore %sk_FragColor %96
OpReturn
OpFunctionEnd
