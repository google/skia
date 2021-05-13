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
OpName %m1 "m1"
OpName %m5 "m5"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m5 "_2_m5"
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
OpDecorate %m1 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%85 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v2float %float_1 %float_2
%38 = OpCompositeConstruct %v2float %float_3 %float_4
%36 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
%44 = OpCompositeConstruct %mat2v2float %45 %46
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %44 0
%50 = OpFOrdEqual %v2bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %43 1
%53 = OpCompositeExtract %v2float %44 1
%54 = OpFOrdEqual %v2bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
OpBranch %42
%42 = OpLabel
%57 = OpPhi %bool %false %25 %56 %41
OpStore %ok %57
%61 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%62 = OpLoad %v2float %61
%63 = OpCompositeExtract %float %62 1
%65 = OpCompositeConstruct %v2float %63 %float_0
%66 = OpCompositeConstruct %v2float %float_0 %63
%64 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %m5 %64
%67 = OpLoad %bool %ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %mat2v2float %m5
%72 = OpCompositeConstruct %v2float %float_4 %float_0
%73 = OpCompositeConstruct %v2float %float_0 %float_4
%71 = OpCompositeConstruct %mat2v2float %72 %73
%74 = OpCompositeExtract %v2float %70 0
%75 = OpCompositeExtract %v2float %71 0
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpCompositeExtract %v2float %70 1
%79 = OpCompositeExtract %v2float %71 1
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %77 %81
OpBranch %69
%69 = OpLabel
%83 = OpPhi %bool %false %42 %82 %68
OpStore %ok %83
%84 = OpLoad %bool %ok
OpReturnValue %84
OpFunctionEnd
%main = OpFunction %v4float None %85
%86 = OpFunctionParameter %_ptr_Function_v2float
%87 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m5 = OpVariable %_ptr_Function_mat2v2float Function
%139 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%91 = OpCompositeConstruct %v2float %float_1 %float_2
%92 = OpCompositeConstruct %v2float %float_3 %float_4
%90 = OpCompositeConstruct %mat2v2float %91 %92
OpStore %_1_m1 %90
%93 = OpLoad %bool %_0_ok
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %mat2v2float %_1_m1
%98 = OpCompositeConstruct %v2float %float_1 %float_2
%99 = OpCompositeConstruct %v2float %float_3 %float_4
%97 = OpCompositeConstruct %mat2v2float %98 %99
%100 = OpCompositeExtract %v2float %96 0
%101 = OpCompositeExtract %v2float %97 0
%102 = OpFOrdEqual %v2bool %100 %101
%103 = OpAll %bool %102
%104 = OpCompositeExtract %v2float %96 1
%105 = OpCompositeExtract %v2float %97 1
%106 = OpFOrdEqual %v2bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %103 %107
OpBranch %95
%95 = OpLabel
%109 = OpPhi %bool %false %87 %108 %94
OpStore %_0_ok %109
%111 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%112 = OpLoad %v2float %111
%113 = OpCompositeExtract %float %112 1
%115 = OpCompositeConstruct %v2float %113 %float_0
%116 = OpCompositeConstruct %v2float %float_0 %113
%114 = OpCompositeConstruct %mat2v2float %115 %116
OpStore %_2_m5 %114
%117 = OpLoad %bool %_0_ok
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpLoad %mat2v2float %_2_m5
%122 = OpCompositeConstruct %v2float %float_4 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %float_4
%121 = OpCompositeConstruct %mat2v2float %122 %123
%124 = OpCompositeExtract %v2float %120 0
%125 = OpCompositeExtract %v2float %121 0
%126 = OpFOrdEqual %v2bool %124 %125
%127 = OpAll %bool %126
%128 = OpCompositeExtract %v2float %120 1
%129 = OpCompositeExtract %v2float %121 1
%130 = OpFOrdEqual %v2bool %128 %129
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %127 %131
OpBranch %119
%119 = OpLabel
%133 = OpPhi %bool %false %95 %132 %118
OpStore %_0_ok %133
%134 = OpLoad %bool %_0_ok
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpFunctionCall %bool %test_half_b
OpBranch %136
%136 = OpLabel
%138 = OpPhi %bool %false %119 %137 %135
OpSelectionMerge %143 None
OpBranchConditional %138 %141 %142
%141 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%147 = OpLoad %v4float %144
OpStore %139 %147
OpBranch %143
%142 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%149 = OpLoad %v4float %148
OpStore %139 %149
OpBranch %143
%143 = OpLabel
%150 = OpLoad %v4float %139
OpReturnValue %150
OpFunctionEnd
