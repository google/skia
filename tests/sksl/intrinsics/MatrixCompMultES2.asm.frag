OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %h22 "h22"
OpName %f22 "f22"
OpName %h33 "h33"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %h22 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %h33 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%34 = OpConstantComposite %v2float %float_0 %float_5
%35 = OpConstantComposite %v2float %float_10 %float_15
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_2 = OpConstant %float 2
%63 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_4 = OpConstant %float 4
%92 = OpConstantComposite %v2float %float_1 %float_0
%93 = OpConstantComposite %v2float %float_0 %float_4
%float_6 = OpConstant %float 6
%float_8 = OpConstant %float 8
%float_12 = OpConstant %float 12
%float_14 = OpConstant %float 14
%float_16 = OpConstant %float 16
%float_18 = OpConstant %float 18
%114 = OpConstantComposite %v3float %float_2 %float_4 %float_6
%115 = OpConstantComposite %v3float %float_8 %float_10 %float_12
%116 = OpConstantComposite %v3float %float_14 %float_16 %float_18
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%h22 = OpVariable %_ptr_Function_mat2v2float Function
%f22 = OpVariable %_ptr_Function_mat2v2float Function
%h33 = OpVariable %_ptr_Function_mat3v3float Function
%134 = OpVariable %_ptr_Function_v4float Function
%36 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %h22 %36
%39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%43 = OpLoad %mat2v2float %39
%46 = OpCompositeConstruct %v2float %float_1 %float_0
%47 = OpCompositeConstruct %v2float %float_0 %float_1
%45 = OpCompositeConstruct %mat2v2float %46 %47
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %45 0
%50 = OpFMul %v2float %48 %49
%51 = OpCompositeExtract %v2float %43 1
%52 = OpCompositeExtract %v2float %45 1
%53 = OpFMul %v2float %51 %52
%54 = OpCompositeConstruct %mat2v2float %50 %53
OpStore %f22 %54
%58 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%61 = OpLoad %mat3v3float %58
%64 = OpCompositeConstruct %mat3v3float %63 %63 %63
%65 = OpCompositeExtract %v3float %61 0
%66 = OpCompositeExtract %v3float %64 0
%67 = OpFMul %v3float %65 %66
%68 = OpCompositeExtract %v3float %61 1
%69 = OpCompositeExtract %v3float %64 1
%70 = OpFMul %v3float %68 %69
%71 = OpCompositeExtract %v3float %61 2
%72 = OpCompositeExtract %v3float %64 2
%73 = OpFMul %v3float %71 %72
%74 = OpCompositeConstruct %mat3v3float %67 %70 %73
OpStore %h33 %74
%76 = OpLoad %mat2v2float %h22
%77 = OpCompositeConstruct %mat2v2float %34 %35
%79 = OpCompositeExtract %v2float %76 0
%80 = OpCompositeExtract %v2float %77 0
%81 = OpFOrdEqual %v2bool %79 %80
%82 = OpAll %bool %81
%83 = OpCompositeExtract %v2float %76 1
%84 = OpCompositeExtract %v2float %77 1
%85 = OpFOrdEqual %v2bool %83 %84
%86 = OpAll %bool %85
%87 = OpLogicalAnd %bool %82 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %mat2v2float %f22
%94 = OpCompositeConstruct %mat2v2float %92 %93
%95 = OpCompositeExtract %v2float %90 0
%96 = OpCompositeExtract %v2float %94 0
%97 = OpFOrdEqual %v2bool %95 %96
%98 = OpAll %bool %97
%99 = OpCompositeExtract %v2float %90 1
%100 = OpCompositeExtract %v2float %94 1
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpLogicalAnd %bool %98 %102
OpBranch %89
%89 = OpLabel
%104 = OpPhi %bool %false %28 %103 %88
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %mat3v3float %h33
%117 = OpCompositeConstruct %mat3v3float %114 %115 %116
%119 = OpCompositeExtract %v3float %107 0
%120 = OpCompositeExtract %v3float %117 0
%121 = OpFOrdEqual %v3bool %119 %120
%122 = OpAll %bool %121
%123 = OpCompositeExtract %v3float %107 1
%124 = OpCompositeExtract %v3float %117 1
%125 = OpFOrdEqual %v3bool %123 %124
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %122 %126
%128 = OpCompositeExtract %v3float %107 2
%129 = OpCompositeExtract %v3float %117 2
%130 = OpFOrdEqual %v3bool %128 %129
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %127 %131
OpBranch %106
%106 = OpLabel
%133 = OpPhi %bool %false %89 %132 %105
OpSelectionMerge %138 None
OpBranchConditional %133 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%142 = OpLoad %v4float %139
OpStore %134 %142
OpBranch %138
%137 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%145 = OpLoad %v4float %143
OpStore %134 %145
OpBranch %138
%138 = OpLabel
%146 = OpLoad %v4float %134
OpReturnValue %146
OpFunctionEnd
