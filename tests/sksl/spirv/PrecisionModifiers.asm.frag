OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %vcolor_Stage0 %vTransformedCoords_0_Stage0
OpExecutionMode %main OriginUpperLeft
OpSourceExtension "GL_ARB_separate_shader_objects"
OpSourceExtension "GL_ARB_shading_language_420pack"
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "um_Stage1_c0"
OpMemberName %uniformBuffer 1 "uv_Stage1_c0"
OpMemberName %uniformBuffer 2 "uleftBorderColor_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 3 "urightBorderColor_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 4 "umatrix_Stage1_c0_c0_c0_c0"
OpMemberName %uniformBuffer 5 "ustart_Stage1_c0_c0_c0_c1"
OpMemberName %uniformBuffer 6 "uend_Stage1_c0_c0_c0_c1"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %vcolor_Stage0 "vcolor_Stage0"
OpName %vTransformedCoords_0_Stage0 "vTransformedCoords_0_Stage0"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpName %_14_t "_14_t"
OpName %_15_t "_15_t"
OpName %_16_outColor "_16_outColor"
OpName %_17_coords "_17_coords"
OpName %_18_t "_18_t"
OpName %_19_color "_19_color"
OpMemberDecorate %uniformBuffer 0 Offset 16
OpMemberDecorate %uniformBuffer 0 ColMajor
OpMemberDecorate %uniformBuffer 0 MatrixStride 16
OpMemberDecorate %uniformBuffer 0 RelaxedPrecision
OpMemberDecorate %uniformBuffer 1 Offset 80
OpMemberDecorate %uniformBuffer 1 RelaxedPrecision
OpMemberDecorate %uniformBuffer 2 Offset 96
OpMemberDecorate %uniformBuffer 2 RelaxedPrecision
OpMemberDecorate %uniformBuffer 3 Offset 112
OpMemberDecorate %uniformBuffer 3 RelaxedPrecision
OpMemberDecorate %uniformBuffer 4 Offset 128
OpMemberDecorate %uniformBuffer 4 ColMajor
OpMemberDecorate %uniformBuffer 4 MatrixStride 16
OpMemberDecorate %uniformBuffer 5 Offset 176
OpMemberDecorate %uniformBuffer 5 RelaxedPrecision
OpMemberDecorate %uniformBuffer 6 Offset 192
OpMemberDecorate %uniformBuffer 6 RelaxedPrecision
OpDecorate %uniformBuffer Block
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %vcolor_Stage0 RelaxedPrecision
OpDecorate %vcolor_Stage0 Location 0
OpDecorate %vTransformedCoords_0_Stage0 Location 1
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_14_t RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %_15_t RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %_16_outColor RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %_18_t RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %_19_color RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%uniformBuffer = OpTypeStruct %mat4v4float %v4float %v4float %v4float %mat3v3float %v4float %v4float
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
%3 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%vcolor_Stage0 = OpVariable %_ptr_Input_v4float Input
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vTransformedCoords_0_Stage0 = OpVariable %_ptr_Input_v2float Input
%void = OpTypeVoid
%22 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%float_9_99999975en06 = OpConstant %float 9.99999975e-06
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_3 = OpConstant %int 3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
%int_0 = OpConstant %int 0
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_1 = OpConstant %int 1
%main = OpFunction %void None %22
%23 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_14_t = OpVariable %_ptr_Function_float Function
%_15_t = OpVariable %_ptr_Function_v4float Function
%_16_outColor = OpVariable %_ptr_Function_v4float Function
%_17_coords = OpVariable %_ptr_Function_v2float Function
%_18_t = OpVariable %_ptr_Function_float Function
%_19_color = OpVariable %_ptr_Function_v4float Function
%27 = OpLoad %v4float %vcolor_Stage0
OpStore %outputColor_Stage0 %27
OpStore %outputCoverage_Stage0 %29
%33 = OpLoad %v2float %vTransformedCoords_0_Stage0
%34 = OpCompositeExtract %float %33 0
%36 = OpFAdd %float %34 %float_9_99999975en06
OpStore %_14_t %36
%38 = OpLoad %float %_14_t
%40 = OpCompositeConstruct %v4float %38 %float_1 %float_0 %float_0
OpStore %_15_t %40
%42 = OpLoad %v4float %_15_t
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdLessThan %bool %43 %float_0
OpSelectionMerge %47 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2
%52 = OpLoad %v4float %50
OpStore %_16_outColor %52
OpBranch %47
%46 = OpLabel
%53 = OpLoad %v4float %_15_t
%54 = OpCompositeExtract %float %53 0
%55 = OpFOrdGreaterThan %bool %54 %float_1
OpSelectionMerge %58 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %3 %int_3
%61 = OpLoad %v4float %60
OpStore %_16_outColor %61
OpBranch %58
%57 = OpLabel
%64 = OpLoad %v4float %_15_t
%65 = OpCompositeExtract %float %64 0
%66 = OpCompositeConstruct %v2float %65 %float_0
OpStore %_17_coords %66
%68 = OpLoad %v2float %_17_coords
%69 = OpCompositeExtract %float %68 0
OpStore %_18_t %69
%72 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%73 = OpLoad %v4float %72
%75 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%76 = OpLoad %v4float %75
%77 = OpLoad %float %_18_t
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
%70 = OpExtInst %v4float %1 FMix %73 %76 %78
OpStore %_16_outColor %70
OpBranch %58
%58 = OpLabel
OpBranch %47
%47 = OpLabel
%80 = OpLoad %v4float %_16_outColor
OpStore %_19_color %80
%81 = OpLoad %v4float %_19_color
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%84 = OpLoad %v4float %_19_color
%85 = OpCompositeExtract %float %84 3
%83 = OpExtInst %float %1 FMax %85 %float_9_99999975en05
%87 = OpFDiv %float %float_1 %83
%88 = OpVectorTimesScalar %v3float %82 %87
%89 = OpCompositeExtract %float %88 0
%90 = OpCompositeExtract %float %88 1
%91 = OpCompositeExtract %float %88 2
%92 = OpLoad %v4float %_19_color
%93 = OpCompositeExtract %float %92 3
%94 = OpCompositeConstruct %v4float %89 %90 %91 %93
OpStore %_19_color %94
%96 = OpAccessChain %_ptr_Uniform_mat4v4float %3 %int_0
%98 = OpLoad %mat4v4float %96
%99 = OpLoad %v4float %_19_color
%100 = OpMatrixTimesVector %v4float %98 %99
%102 = OpAccessChain %_ptr_Uniform_v4float %3 %int_1
%103 = OpLoad %v4float %102
%104 = OpFAdd %v4float %100 %103
OpStore %_19_color %104
%106 = OpLoad %v4float %_19_color
%107 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%108 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%105 = OpExtInst %v4float %1 FClamp %106 %107 %108
OpStore %_19_color %105
%109 = OpLoad %v4float %_19_color
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%111 = OpLoad %v4float %_19_color
%112 = OpCompositeExtract %float %111 3
%113 = OpVectorTimesScalar %v3float %110 %112
%114 = OpLoad %v4float %_19_color
%115 = OpVectorShuffle %v4float %114 %113 4 5 6 3
OpStore %_19_color %115
%116 = OpLoad %v4float %_19_color
OpStore %output_Stage1 %116
%117 = OpLoad %v4float %output_Stage1
%118 = OpLoad %v4float %outputCoverage_Stage0
%119 = OpFMul %v4float %117 %118
OpStore %sk_FragColor %119
OpReturn
OpFunctionEnd
