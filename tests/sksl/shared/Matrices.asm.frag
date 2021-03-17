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
OpName %_entrypoint "_entrypoint"
OpName %test_half "test_half"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m6 "m6"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_2_m1 "_2_m1"
OpName %_4_m3 "_4_m3"
OpName %_5_m4 "_5_m4"
OpName %_6_m5 "_6_m5"
OpName %_7_m6 "_7_m6"
OpName %_10_m10 "_10_m10"
OpName %_11_m11 "_11_m11"
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
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
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
%19 = OpTypeFunction %bool
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%95 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_half = OpFunction %bool None %19
%20 = OpLabel
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m6 = OpVariable %_ptr_Function_mat2v2float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%30 = OpCompositeConstruct %v2float %float_1 %float_2
%31 = OpCompositeConstruct %v2float %float_3 %float_4
%29 = OpCompositeConstruct %mat2v2float %30 %31
OpStore %m1 %29
%33 = OpLoad %mat2v2float %m1
OpStore %m3 %33
%37 = OpCompositeConstruct %v2float %float_1 %float_0
%38 = OpCompositeConstruct %v2float %float_0 %float_1
%35 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m4 %35
%39 = OpLoad %mat2v2float %m3
%40 = OpLoad %mat2v2float %m4
%41 = OpMatrixTimesMatrix %mat2v2float %39 %40
OpStore %m3 %41
%45 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%47 = OpLoad %v2float %45
%48 = OpCompositeExtract %float %47 0
%50 = OpCompositeConstruct %v2float %48 %float_0
%51 = OpCompositeConstruct %v2float %float_0 %48
%49 = OpCompositeConstruct %mat2v2float %50 %51
OpStore %m5 %49
%54 = OpCompositeConstruct %v2float %float_1 %float_2
%55 = OpCompositeConstruct %v2float %float_3 %float_4
%53 = OpCompositeConstruct %mat2v2float %54 %55
OpStore %m6 %53
%56 = OpLoad %mat2v2float %m6
%57 = OpLoad %mat2v2float %m5
%58 = OpCompositeExtract %v2float %56 0
%59 = OpCompositeExtract %v2float %57 0
%60 = OpFAdd %v2float %58 %59
%61 = OpCompositeExtract %v2float %56 1
%62 = OpCompositeExtract %v2float %57 1
%63 = OpFAdd %v2float %61 %62
%64 = OpCompositeConstruct %mat2v2float %60 %63
OpStore %m6 %64
%69 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%71 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%68 = OpCompositeConstruct %mat4v4float %69 %70 %71 %72
OpStore %m10 %68
%75 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%76 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%74 = OpCompositeConstruct %mat4v4float %75 %76 %77 %78
OpStore %m11 %74
%79 = OpLoad %mat4v4float %m11
%80 = OpLoad %mat4v4float %m10
%81 = OpCompositeExtract %v4float %79 0
%82 = OpCompositeExtract %v4float %80 0
%83 = OpFSub %v4float %81 %82
%84 = OpCompositeExtract %v4float %79 1
%85 = OpCompositeExtract %v4float %80 1
%86 = OpFSub %v4float %84 %85
%87 = OpCompositeExtract %v4float %79 2
%88 = OpCompositeExtract %v4float %80 2
%89 = OpFSub %v4float %87 %88
%90 = OpCompositeExtract %v4float %79 3
%91 = OpCompositeExtract %v4float %80 3
%92 = OpFSub %v4float %90 %91
%93 = OpCompositeConstruct %mat4v4float %83 %86 %89 %92
OpStore %m11 %93
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %95
%96 = OpLabel
%_2_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_10_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_11_m11 = OpVariable %_ptr_Function_mat4v4float Function
%162 = OpVariable %_ptr_Function_v4float Function
%99 = OpCompositeConstruct %v2float %float_1 %float_2
%100 = OpCompositeConstruct %v2float %float_3 %float_4
%98 = OpCompositeConstruct %mat2v2float %99 %100
OpStore %_2_m1 %98
%102 = OpLoad %mat2v2float %_2_m1
OpStore %_4_m3 %102
%105 = OpCompositeConstruct %v2float %float_1 %float_0
%106 = OpCompositeConstruct %v2float %float_0 %float_1
%104 = OpCompositeConstruct %mat2v2float %105 %106
OpStore %_5_m4 %104
%107 = OpLoad %mat2v2float %_4_m3
%108 = OpLoad %mat2v2float %_5_m4
%109 = OpMatrixTimesMatrix %mat2v2float %107 %108
OpStore %_4_m3 %109
%111 = OpAccessChain %_ptr_Function_v2float %_2_m1 %int_0
%112 = OpLoad %v2float %111
%113 = OpCompositeExtract %float %112 0
%115 = OpCompositeConstruct %v2float %113 %float_0
%116 = OpCompositeConstruct %v2float %float_0 %113
%114 = OpCompositeConstruct %mat2v2float %115 %116
OpStore %_6_m5 %114
%119 = OpCompositeConstruct %v2float %float_1 %float_2
%120 = OpCompositeConstruct %v2float %float_3 %float_4
%118 = OpCompositeConstruct %mat2v2float %119 %120
OpStore %_7_m6 %118
%121 = OpLoad %mat2v2float %_7_m6
%122 = OpLoad %mat2v2float %_6_m5
%123 = OpCompositeExtract %v2float %121 0
%124 = OpCompositeExtract %v2float %122 0
%125 = OpFAdd %v2float %123 %124
%126 = OpCompositeExtract %v2float %121 1
%127 = OpCompositeExtract %v2float %122 1
%128 = OpFAdd %v2float %126 %127
%129 = OpCompositeConstruct %mat2v2float %125 %128
OpStore %_7_m6 %129
%132 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%133 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%134 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%135 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%131 = OpCompositeConstruct %mat4v4float %132 %133 %134 %135
OpStore %_10_m10 %131
%138 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%139 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%141 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%137 = OpCompositeConstruct %mat4v4float %138 %139 %140 %141
OpStore %_11_m11 %137
%142 = OpLoad %mat4v4float %_11_m11
%143 = OpLoad %mat4v4float %_10_m10
%144 = OpCompositeExtract %v4float %142 0
%145 = OpCompositeExtract %v4float %143 0
%146 = OpFSub %v4float %144 %145
%147 = OpCompositeExtract %v4float %142 1
%148 = OpCompositeExtract %v4float %143 1
%149 = OpFSub %v4float %147 %148
%150 = OpCompositeExtract %v4float %142 2
%151 = OpCompositeExtract %v4float %143 2
%152 = OpFSub %v4float %150 %151
%153 = OpCompositeExtract %v4float %142 3
%154 = OpCompositeExtract %v4float %143 3
%155 = OpFSub %v4float %153 %154
%156 = OpCompositeConstruct %mat4v4float %146 %149 %152 %155
OpStore %_11_m11 %156
OpSelectionMerge %159 None
OpBranchConditional %true %158 %159
%158 = OpLabel
%160 = OpFunctionCall %bool %test_half
OpBranch %159
%159 = OpLabel
%161 = OpPhi %bool %false %96 %160 %158
OpSelectionMerge %166 None
OpBranchConditional %161 %164 %165
%164 = OpLabel
%167 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%169 = OpLoad %v4float %167
OpStore %162 %169
OpBranch %166
%165 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%172 = OpLoad %v4float %170
OpStore %162 %172
OpBranch %166
%166 = OpLabel
%173 = OpLoad %v4float %162
OpReturnValue %173
OpFunctionEnd
