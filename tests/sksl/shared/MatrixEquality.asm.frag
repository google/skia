OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
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
%22 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%127 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%27 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%31 = OpLoad %mat2v2float %27
%37 = OpCompositeConstruct %v2float %float_1 %float_2
%38 = OpCompositeConstruct %v2float %float_3 %float_4
%36 = OpCompositeConstruct %mat2v2float %37 %38
%40 = OpCompositeExtract %v2float %31 0
%41 = OpCompositeExtract %v2float %36 0
%42 = OpFOrdEqual %v2bool %40 %41
%43 = OpAll %bool %42
%44 = OpCompositeExtract %v2float %31 1
%45 = OpCompositeExtract %v2float %36 1
%46 = OpFOrdEqual %v2bool %44 %45
%47 = OpAll %bool %46
%48 = OpLogicalAnd %bool %43 %47
OpStore %_0_ok %48
%50 = OpLoad %bool %_0_ok
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%56 = OpLoad %mat3v3float %53
%63 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%64 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%65 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%62 = OpCompositeConstruct %mat3v3float %63 %64 %65
%67 = OpCompositeExtract %v3float %56 0
%68 = OpCompositeExtract %v3float %62 0
%69 = OpFOrdEqual %v3bool %67 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v3float %56 1
%72 = OpCompositeExtract %v3float %62 1
%73 = OpFOrdEqual %v3bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
%76 = OpCompositeExtract %v3float %56 2
%77 = OpCompositeExtract %v3float %62 2
%78 = OpFOrdEqual %v3bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
OpBranch %52
%52 = OpLabel
%81 = OpPhi %bool %false %23 %80 %51
OpStore %_0_ok %81
%82 = OpLoad %bool %_0_ok
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%86 = OpLoad %mat2v2float %85
%90 = OpCompositeConstruct %v2float %float_100 %float_0
%91 = OpCompositeConstruct %v2float %float_0 %float_100
%88 = OpCompositeConstruct %mat2v2float %90 %91
%92 = OpCompositeExtract %v2float %86 0
%93 = OpCompositeExtract %v2float %88 0
%94 = OpFOrdNotEqual %v2bool %92 %93
%95 = OpAny %bool %94
%96 = OpCompositeExtract %v2float %86 1
%97 = OpCompositeExtract %v2float %88 1
%98 = OpFOrdNotEqual %v2bool %96 %97
%99 = OpAny %bool %98
%100 = OpLogicalOr %bool %95 %99
OpBranch %84
%84 = OpLabel
%101 = OpPhi %bool %false %52 %100 %83
OpStore %_0_ok %101
%102 = OpLoad %bool %_0_ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%106 = OpLoad %mat3v3float %105
%108 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%109 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%110 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%107 = OpCompositeConstruct %mat3v3float %108 %109 %110
%111 = OpCompositeExtract %v3float %106 0
%112 = OpCompositeExtract %v3float %107 0
%113 = OpFOrdNotEqual %v3bool %111 %112
%114 = OpAny %bool %113
%115 = OpCompositeExtract %v3float %106 1
%116 = OpCompositeExtract %v3float %107 1
%117 = OpFOrdNotEqual %v3bool %115 %116
%118 = OpAny %bool %117
%119 = OpLogicalOr %bool %114 %118
%120 = OpCompositeExtract %v3float %106 2
%121 = OpCompositeExtract %v3float %107 2
%122 = OpFOrdNotEqual %v3bool %120 %121
%123 = OpAny %bool %122
%124 = OpLogicalOr %bool %119 %123
OpBranch %104
%104 = OpLabel
%125 = OpPhi %bool %false %84 %124 %103
OpStore %_0_ok %125
%126 = OpLoad %bool %_0_ok
OpSelectionMerge %131 None
OpBranchConditional %126 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %132
OpStore %127 %135
OpBranch %131
%130 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%138 = OpLoad %v4float %136
OpStore %127 %138
OpBranch %131
%131 = OpLabel
%139 = OpLoad %v4float %127
OpReturnValue %139
OpFunctionEnd
