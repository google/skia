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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m4 "m4"
OpName %m9 "m9"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m4 "_1_m4"
OpName %_2_m9 "_2_m9"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m4 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%89 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v2float %float_6 %float_0
%35 = OpCompositeConstruct %v2float %float_0 %float_6
%33 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %m4 %33
%37 = OpLoad %bool %ok
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %mat2v2float %m4
%42 = OpCompositeConstruct %v2float %float_6 %float_0
%43 = OpCompositeConstruct %v2float %float_0 %float_6
%41 = OpCompositeConstruct %mat2v2float %42 %43
%45 = OpCompositeExtract %v2float %40 0
%46 = OpCompositeExtract %v2float %41 0
%47 = OpFOrdEqual %v2bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %40 1
%50 = OpCompositeExtract %v2float %41 1
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpBranch %39
%39 = OpLabel
%54 = OpPhi %bool %false %25 %53 %38
OpStore %ok %54
%61 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%62 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%63 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%60 = OpCompositeConstruct %mat3v3float %61 %62 %63
OpStore %m9 %60
%64 = OpLoad %bool %ok
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpLoad %mat3v3float %m9
%69 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%70 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%71 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%68 = OpCompositeConstruct %mat3v3float %69 %70 %71
%73 = OpCompositeExtract %v3float %67 0
%74 = OpCompositeExtract %v3float %68 0
%75 = OpFOrdEqual %v3bool %73 %74
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v3float %67 1
%78 = OpCompositeExtract %v3float %68 1
%79 = OpFOrdEqual %v3bool %77 %78
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %76 %80
%82 = OpCompositeExtract %v3float %67 2
%83 = OpCompositeExtract %v3float %68 2
%84 = OpFOrdEqual %v3bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
OpBranch %66
%66 = OpLabel
%87 = OpPhi %bool %false %39 %86 %65
OpStore %ok %87
%88 = OpLoad %bool %ok
OpReturnValue %88
OpFunctionEnd
%main = OpFunction %v4float None %89
%90 = OpFunctionParameter %_ptr_Function_v2float
%91 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m9 = OpVariable %_ptr_Function_mat3v3float Function
%147 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%95 = OpCompositeConstruct %v2float %float_6 %float_0
%96 = OpCompositeConstruct %v2float %float_0 %float_6
%94 = OpCompositeConstruct %mat2v2float %95 %96
OpStore %_1_m4 %94
%97 = OpLoad %bool %_0_ok
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %mat2v2float %_1_m4
%102 = OpCompositeConstruct %v2float %float_6 %float_0
%103 = OpCompositeConstruct %v2float %float_0 %float_6
%101 = OpCompositeConstruct %mat2v2float %102 %103
%104 = OpCompositeExtract %v2float %100 0
%105 = OpCompositeExtract %v2float %101 0
%106 = OpFOrdEqual %v2bool %104 %105
%107 = OpAll %bool %106
%108 = OpCompositeExtract %v2float %100 1
%109 = OpCompositeExtract %v2float %101 1
%110 = OpFOrdEqual %v2bool %108 %109
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %107 %111
OpBranch %99
%99 = OpLabel
%113 = OpPhi %bool %false %91 %112 %98
OpStore %_0_ok %113
%116 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%117 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%118 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%115 = OpCompositeConstruct %mat3v3float %116 %117 %118
OpStore %_2_m9 %115
%119 = OpLoad %bool %_0_ok
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %mat3v3float %_2_m9
%124 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%125 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%126 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%123 = OpCompositeConstruct %mat3v3float %124 %125 %126
%127 = OpCompositeExtract %v3float %122 0
%128 = OpCompositeExtract %v3float %123 0
%129 = OpFOrdEqual %v3bool %127 %128
%130 = OpAll %bool %129
%131 = OpCompositeExtract %v3float %122 1
%132 = OpCompositeExtract %v3float %123 1
%133 = OpFOrdEqual %v3bool %131 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %130 %134
%136 = OpCompositeExtract %v3float %122 2
%137 = OpCompositeExtract %v3float %123 2
%138 = OpFOrdEqual %v3bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
OpBranch %121
%121 = OpLabel
%141 = OpPhi %bool %false %99 %140 %120
OpStore %_0_ok %141
%142 = OpLoad %bool %_0_ok
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpFunctionCall %bool %test_half_b
OpBranch %144
%144 = OpLabel
%146 = OpPhi %bool %false %121 %145 %143
OpSelectionMerge %151 None
OpBranchConditional %146 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%156 = OpLoad %v4float %152
OpStore %147 %156
OpBranch %151
%150 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%159 = OpLoad %v4float %157
OpStore %147 %159
OpBranch %151
%151 = OpLabel
%160 = OpLoad %v4float %147
OpReturnValue %160
OpFunctionEnd
