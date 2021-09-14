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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
OpName %ok "ok"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %68 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%float_4 = OpConstant %float 4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%174 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%45 = OpCompositeExtract %float %42 0
%46 = OpCompositeExtract %float %42 1
%47 = OpCompositeConstruct %v2float %45 %46
%48 = OpCompositeExtract %float %42 2
%49 = OpCompositeConstruct %v2float %48 %float_4
%50 = OpCompositeConstruct %mat2v2float %47 %49
%54 = OpCompositeConstruct %v2float %float_1 %float_2
%55 = OpCompositeConstruct %v2float %float_3 %float_4
%56 = OpCompositeConstruct %mat2v2float %54 %55
%58 = OpCompositeExtract %v2float %50 0
%59 = OpCompositeExtract %v2float %56 0
%60 = OpFOrdEqual %v2bool %58 %59
%61 = OpAll %bool %60
%62 = OpCompositeExtract %v2float %50 1
%63 = OpCompositeExtract %v2float %56 1
%64 = OpFOrdEqual %v2bool %62 %63
%65 = OpAll %bool %64
%66 = OpLogicalAnd %bool %61 %65
OpStore %ok %66
%68 = OpLoad %bool %ok
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v4float %f4
%72 = OpVectorShuffle %v2float %71 %71 0 1
%73 = OpLoad %v4float %f4
%74 = OpVectorShuffle %v2float %73 %73 2 3
%75 = OpLoad %v4float %f4
%76 = OpLoad %v4float %f4
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeExtract %float %72 0
%79 = OpCompositeExtract %float %72 1
%80 = OpCompositeExtract %float %74 0
%81 = OpCompositeConstruct %v3float %78 %79 %80
%82 = OpCompositeExtract %float %74 1
%83 = OpCompositeExtract %float %75 0
%84 = OpCompositeExtract %float %75 1
%85 = OpCompositeConstruct %v3float %82 %83 %84
%86 = OpCompositeExtract %float %75 2
%87 = OpCompositeExtract %float %75 3
%88 = OpCompositeConstruct %v3float %86 %87 %77
%89 = OpCompositeConstruct %mat3v3float %81 %85 %88
%91 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%92 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%93 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%94 = OpCompositeConstruct %mat3v3float %91 %92 %93
%96 = OpCompositeExtract %v3float %89 0
%97 = OpCompositeExtract %v3float %94 0
%98 = OpFOrdEqual %v3bool %96 %97
%99 = OpAll %bool %98
%100 = OpCompositeExtract %v3float %89 1
%101 = OpCompositeExtract %v3float %94 1
%102 = OpFOrdEqual %v3bool %100 %101
%103 = OpAll %bool %102
%104 = OpLogicalAnd %bool %99 %103
%105 = OpCompositeExtract %v3float %89 2
%106 = OpCompositeExtract %v3float %94 2
%107 = OpFOrdEqual %v3bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
OpBranch %70
%70 = OpLabel
%110 = OpPhi %bool %false %26 %109 %69
OpStore %ok %110
%111 = OpLoad %bool %ok
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %v4float %f4
%115 = OpVectorShuffle %v2float %114 %114 0 1
%116 = OpLoad %v4float %f4
%117 = OpVectorShuffle %v4float %116 %116 2 3 0 1
%118 = OpLoad %v4float %f4
%119 = OpVectorShuffle %v3float %118 %118 2 3 0
%120 = OpLoad %v4float %f4
%121 = OpVectorShuffle %v2float %120 %120 1 2
%122 = OpLoad %v4float %f4
%123 = OpVectorShuffle %v4float %122 %122 3 0 1 2
%124 = OpLoad %v4float %f4
%125 = OpCompositeExtract %float %124 3
%126 = OpCompositeExtract %float %115 0
%127 = OpCompositeExtract %float %115 1
%128 = OpCompositeExtract %float %117 0
%129 = OpCompositeExtract %float %117 1
%130 = OpCompositeConstruct %v4float %126 %127 %128 %129
%131 = OpCompositeExtract %float %117 2
%132 = OpCompositeExtract %float %117 3
%133 = OpCompositeExtract %float %119 0
%134 = OpCompositeExtract %float %119 1
%135 = OpCompositeConstruct %v4float %131 %132 %133 %134
%136 = OpCompositeExtract %float %119 2
%137 = OpCompositeExtract %float %121 0
%138 = OpCompositeExtract %float %121 1
%139 = OpCompositeExtract %float %123 0
%140 = OpCompositeConstruct %v4float %136 %137 %138 %139
%141 = OpCompositeExtract %float %123 1
%142 = OpCompositeExtract %float %123 2
%143 = OpCompositeExtract %float %123 3
%144 = OpCompositeConstruct %v4float %141 %142 %143 %125
%145 = OpCompositeConstruct %mat4v4float %130 %135 %140 %144
%147 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%148 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%149 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%150 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%151 = OpCompositeConstruct %mat4v4float %147 %148 %149 %150
%153 = OpCompositeExtract %v4float %145 0
%154 = OpCompositeExtract %v4float %151 0
%155 = OpFOrdEqual %v4bool %153 %154
%156 = OpAll %bool %155
%157 = OpCompositeExtract %v4float %145 1
%158 = OpCompositeExtract %v4float %151 1
%159 = OpFOrdEqual %v4bool %157 %158
%160 = OpAll %bool %159
%161 = OpLogicalAnd %bool %156 %160
%162 = OpCompositeExtract %v4float %145 2
%163 = OpCompositeExtract %v4float %151 2
%164 = OpFOrdEqual %v4bool %162 %163
%165 = OpAll %bool %164
%166 = OpLogicalAnd %bool %161 %165
%167 = OpCompositeExtract %v4float %145 3
%168 = OpCompositeExtract %v4float %151 3
%169 = OpFOrdEqual %v4bool %167 %168
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %166 %170
OpBranch %113
%113 = OpLabel
%172 = OpPhi %bool %false %70 %171 %112
OpStore %ok %172
%173 = OpLoad %bool %ok
OpSelectionMerge %177 None
OpBranchConditional %173 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%181 = OpLoad %v4float %178
OpStore %174 %181
OpBranch %177
%176 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%184 = OpLoad %v4float %182
OpStore %174 %184
OpBranch %177
%177 = OpLabel
%185 = OpLoad %v4float %174
OpReturnValue %185
OpFunctionEnd
