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
OpDecorate %26 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%24 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v2float = OpTypeVector %float 2
%28 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%v4bool = OpTypeVector %bool 4
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %19
%20 = OpLabel
%tmpColor = OpVariable %_ptr_Function_v4float Function
%60 = OpVariable %_ptr_Function_v4float Function
%26 = OpLoad %12 %s
%25 = OpImageSampleImplicitLod %v4float %26 %28
OpStore %tmpColor %25
%29 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%33 = OpLoad %mat4v4float %29
%36 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%37 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%38 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%39 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%34 = OpCompositeConstruct %mat4v4float %36 %37 %38 %39
%41 = OpCompositeExtract %v4float %33 0
%42 = OpCompositeExtract %v4float %34 0
%43 = OpFOrdNotEqual %v4bool %41 %42
%44 = OpAny %bool %43
%45 = OpCompositeExtract %v4float %33 1
%46 = OpCompositeExtract %v4float %34 1
%47 = OpFOrdNotEqual %v4bool %45 %46
%48 = OpAny %bool %47
%49 = OpLogicalOr %bool %44 %48
%50 = OpCompositeExtract %v4float %33 2
%51 = OpCompositeExtract %v4float %34 2
%52 = OpFOrdNotEqual %v4bool %50 %51
%53 = OpAny %bool %52
%54 = OpLogicalOr %bool %49 %53
%55 = OpCompositeExtract %v4float %33 3
%56 = OpCompositeExtract %v4float %34 3
%57 = OpFOrdNotEqual %v4bool %55 %56
%58 = OpAny %bool %57
%59 = OpLogicalOr %bool %54 %58
OpSelectionMerge %63 None
OpBranchConditional %59 %61 %62
%61 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_mat4v4float %14 %int_0
%66 = OpLoad %mat4v4float %65
%67 = OpLoad %v4float %tmpColor
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%70 = OpCompositeExtract %float %68 0
%71 = OpCompositeExtract %float %68 1
%72 = OpCompositeExtract %float %68 2
%73 = OpCompositeConstruct %v4float %70 %71 %72 %float_1
%74 = OpMatrixTimesVector %v4float %66 %73
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%76 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%77 = OpLoad %v4float %tmpColor
%78 = OpCompositeExtract %float %77 3
%79 = OpCompositeConstruct %v3float %78 %78 %78
%64 = OpExtInst %v3float %1 FClamp %75 %76 %79
%80 = OpCompositeExtract %float %64 0
%81 = OpCompositeExtract %float %64 1
%82 = OpCompositeExtract %float %64 2
%83 = OpLoad %v4float %tmpColor
%84 = OpCompositeExtract %float %83 3
%85 = OpCompositeConstruct %v4float %80 %81 %82 %84
OpStore %60 %85
OpBranch %63
%62 = OpLabel
%86 = OpLoad %v4float %tmpColor
OpStore %60 %86
OpBranch %63
%63 = OpLabel
%87 = OpLoad %v4float %60
%88 = OpFMul %v4float %24 %87
OpStore %sk_FragColor %88
OpReturn
OpFunctionEnd
