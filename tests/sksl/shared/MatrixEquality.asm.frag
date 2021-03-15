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
OpName %test_equality "test_equality"
OpName %ok "ok"
OpName %main "main"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%23 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
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
%132 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %20
%21 = OpLabel
%22 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%test_equality = OpFunction %bool None %23
%24 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
%29 = OpLoad %bool %ok
OpSelectionMerge %31 None
OpBranchConditional %29 %30 %31
%30 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%36 = OpLoad %mat2v2float %32
%42 = OpCompositeConstruct %v2float %float_1 %float_2
%43 = OpCompositeConstruct %v2float %float_3 %float_4
%41 = OpCompositeConstruct %mat2v2float %42 %43
%45 = OpCompositeExtract %v2float %36 0
%46 = OpCompositeExtract %v2float %41 0
%47 = OpFOrdEqual %v2bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %36 1
%50 = OpCompositeExtract %v2float %41 1
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpBranch %31
%31 = OpLabel
%54 = OpPhi %bool %false %24 %53 %30
OpStore %ok %54
%55 = OpLoad %bool %ok
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%61 = OpLoad %mat3v3float %58
%68 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%69 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%70 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%67 = OpCompositeConstruct %mat3v3float %68 %69 %70
%72 = OpCompositeExtract %v3float %61 0
%73 = OpCompositeExtract %v3float %67 0
%74 = OpFOrdEqual %v3bool %72 %73
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v3float %61 1
%77 = OpCompositeExtract %v3float %67 1
%78 = OpFOrdEqual %v3bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
%81 = OpCompositeExtract %v3float %61 2
%82 = OpCompositeExtract %v3float %67 2
%83 = OpFOrdEqual %v3bool %81 %82
%84 = OpAll %bool %83
%85 = OpLogicalAnd %bool %80 %84
OpBranch %57
%57 = OpLabel
%86 = OpPhi %bool %false %31 %85 %56
OpStore %ok %86
%87 = OpLoad %bool %ok
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%91 = OpLoad %mat2v2float %90
%95 = OpCompositeConstruct %v2float %float_100 %float_0
%96 = OpCompositeConstruct %v2float %float_0 %float_100
%93 = OpCompositeConstruct %mat2v2float %95 %96
%97 = OpCompositeExtract %v2float %91 0
%98 = OpCompositeExtract %v2float %93 0
%99 = OpFOrdNotEqual %v2bool %97 %98
%100 = OpAny %bool %99
%101 = OpCompositeExtract %v2float %91 1
%102 = OpCompositeExtract %v2float %93 1
%103 = OpFOrdNotEqual %v2bool %101 %102
%104 = OpAny %bool %103
%105 = OpLogicalOr %bool %100 %104
OpBranch %89
%89 = OpLabel
%106 = OpPhi %bool %false %57 %105 %88
OpStore %ok %106
%107 = OpLoad %bool %ok
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%111 = OpLoad %mat3v3float %110
%113 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%114 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%115 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%112 = OpCompositeConstruct %mat3v3float %113 %114 %115
%116 = OpCompositeExtract %v3float %111 0
%117 = OpCompositeExtract %v3float %112 0
%118 = OpFOrdNotEqual %v3bool %116 %117
%119 = OpAny %bool %118
%120 = OpCompositeExtract %v3float %111 1
%121 = OpCompositeExtract %v3float %112 1
%122 = OpFOrdNotEqual %v3bool %120 %121
%123 = OpAny %bool %122
%124 = OpLogicalOr %bool %119 %123
%125 = OpCompositeExtract %v3float %111 2
%126 = OpCompositeExtract %v3float %112 2
%127 = OpFOrdNotEqual %v3bool %125 %126
%128 = OpAny %bool %127
%129 = OpLogicalOr %bool %124 %128
OpBranch %109
%109 = OpLabel
%130 = OpPhi %bool %false %89 %129 %108
OpStore %ok %130
%131 = OpLoad %bool %ok
OpReturnValue %131
OpFunctionEnd
%main = OpFunction %v4float None %132
%133 = OpLabel
%135 = OpVariable %_ptr_Function_v4float Function
%134 = OpFunctionCall %bool %test_equality
OpSelectionMerge %139 None
OpBranchConditional %134 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%143 = OpLoad %v4float %140
OpStore %135 %143
OpBranch %139
%138 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%146 = OpLoad %v4float %144
OpStore %135 %146
OpBranch %139
%139 = OpLabel
%147 = OpLoad %v4float %135
OpReturnValue %147
OpFunctionEnd
