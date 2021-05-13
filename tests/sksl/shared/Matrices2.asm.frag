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
OpName %m2 "m2"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m2 "_2_m2"
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
OpDecorate %m2 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
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
%float_5 = OpConstant %float 5
%60 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%86 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
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
%62 = OpCompositeExtract %float %60 0
%63 = OpCompositeExtract %float %60 1
%64 = OpCompositeExtract %float %60 2
%65 = OpCompositeExtract %float %60 3
%66 = OpCompositeConstruct %v2float %62 %63
%67 = OpCompositeConstruct %v2float %64 %65
%61 = OpCompositeConstruct %mat2v2float %66 %67
OpStore %m2 %61
%68 = OpLoad %bool %ok
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %mat2v2float %m2
%73 = OpCompositeConstruct %v2float %float_5 %float_5
%74 = OpCompositeConstruct %v2float %float_5 %float_5
%72 = OpCompositeConstruct %mat2v2float %73 %74
%75 = OpCompositeExtract %v2float %71 0
%76 = OpCompositeExtract %v2float %72 0
%77 = OpFOrdEqual %v2bool %75 %76
%78 = OpAll %bool %77
%79 = OpCompositeExtract %v2float %71 1
%80 = OpCompositeExtract %v2float %72 1
%81 = OpFOrdEqual %v2bool %79 %80
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %78 %82
OpBranch %70
%70 = OpLabel
%84 = OpPhi %bool %false %42 %83 %69
OpStore %ok %84
%85 = OpLoad %bool %ok
OpReturnValue %85
OpFunctionEnd
%main = OpFunction %v4float None %86
%87 = OpFunctionParameter %_ptr_Function_v2float
%88 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%141 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%92 = OpCompositeConstruct %v2float %float_1 %float_2
%93 = OpCompositeConstruct %v2float %float_3 %float_4
%91 = OpCompositeConstruct %mat2v2float %92 %93
OpStore %_1_m1 %91
%94 = OpLoad %bool %_0_ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %mat2v2float %_1_m1
%99 = OpCompositeConstruct %v2float %float_1 %float_2
%100 = OpCompositeConstruct %v2float %float_3 %float_4
%98 = OpCompositeConstruct %mat2v2float %99 %100
%101 = OpCompositeExtract %v2float %97 0
%102 = OpCompositeExtract %v2float %98 0
%103 = OpFOrdEqual %v2bool %101 %102
%104 = OpAll %bool %103
%105 = OpCompositeExtract %v2float %97 1
%106 = OpCompositeExtract %v2float %98 1
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
OpBranch %96
%96 = OpLabel
%110 = OpPhi %bool %false %88 %109 %95
OpStore %_0_ok %110
%113 = OpCompositeExtract %float %60 0
%114 = OpCompositeExtract %float %60 1
%115 = OpCompositeExtract %float %60 2
%116 = OpCompositeExtract %float %60 3
%117 = OpCompositeConstruct %v2float %113 %114
%118 = OpCompositeConstruct %v2float %115 %116
%112 = OpCompositeConstruct %mat2v2float %117 %118
OpStore %_2_m2 %112
%119 = OpLoad %bool %_0_ok
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %mat2v2float %_2_m2
%124 = OpCompositeConstruct %v2float %float_5 %float_5
%125 = OpCompositeConstruct %v2float %float_5 %float_5
%123 = OpCompositeConstruct %mat2v2float %124 %125
%126 = OpCompositeExtract %v2float %122 0
%127 = OpCompositeExtract %v2float %123 0
%128 = OpFOrdEqual %v2bool %126 %127
%129 = OpAll %bool %128
%130 = OpCompositeExtract %v2float %122 1
%131 = OpCompositeExtract %v2float %123 1
%132 = OpFOrdEqual %v2bool %130 %131
%133 = OpAll %bool %132
%134 = OpLogicalAnd %bool %129 %133
OpBranch %121
%121 = OpLabel
%135 = OpPhi %bool %false %96 %134 %120
OpStore %_0_ok %135
%136 = OpLoad %bool %_0_ok
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpFunctionCall %bool %test_half_b
OpBranch %138
%138 = OpLabel
%140 = OpPhi %bool %false %121 %139 %137
OpSelectionMerge %145 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%150 = OpLoad %v4float %146
OpStore %141 %150
OpBranch %145
%144 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%153 = OpLoad %v4float %151
OpStore %141 %153
OpBranch %145
%145 = OpLabel
%154 = OpLoad %v4float %141
OpReturnValue %154
OpFunctionEnd
