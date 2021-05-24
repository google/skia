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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 "check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3"
OpName %main "main"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %v4 "v4"
OpName %v5 "v5"
OpName %v6 "v6"
OpName %v7 "v7"
OpName %v8 "v8"
OpName %v9 "v9"
OpName %v10 "v10"
OpName %v11 "v11"
OpName %v12 "v12"
OpName %v13 "v13"
OpName %v14 "v14"
OpName %v15 "v15"
OpName %v16 "v16"
OpName %v17 "v17"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%30 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool
%float_1 = OpConstant %float 1
%float_17 = OpConstant %float 17
%115 = OpTypeFunction %v4float %_ptr_Function_v2float
%119 = OpConstantComposite %v2float %float_1 %float_1
%float_2 = OpConstant %float 2
%122 = OpConstantComposite %v2float %float_1 %float_2
%125 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%128 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%131 = OpConstantComposite %v2int %int_1 %int_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%159 = OpConstantComposite %v4bool %true %false %true %false
%161 = OpConstantComposite %v2float %float_1 %float_0
%165 = OpConstantComposite %v2bool %true %true
%168 = OpConstantComposite %v3bool %true %true %true
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
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 = OpFunction %bool None %30
%37 = OpFunctionParameter %_ptr_Function_v2float
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpFunctionParameter %_ptr_Function_v2float
%40 = OpFunctionParameter %_ptr_Function_v3float
%41 = OpFunctionParameter %_ptr_Function_v2int
%42 = OpFunctionParameter %_ptr_Function_v2int
%43 = OpFunctionParameter %_ptr_Function_v2float
%44 = OpFunctionParameter %_ptr_Function_v2float
%45 = OpFunctionParameter %_ptr_Function_v4float
%46 = OpFunctionParameter %_ptr_Function_v2int
%47 = OpFunctionParameter %_ptr_Function_v4bool
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpFunctionParameter %_ptr_Function_v2float
%50 = OpFunctionParameter %_ptr_Function_v2float
%51 = OpFunctionParameter %_ptr_Function_v2bool
%52 = OpFunctionParameter %_ptr_Function_v2bool
%53 = OpFunctionParameter %_ptr_Function_v3bool
%54 = OpLabel
%55 = OpLoad %v2float %37
%56 = OpCompositeExtract %float %55 0
%57 = OpLoad %v2float %38
%58 = OpCompositeExtract %float %57 0
%59 = OpFAdd %float %56 %58
%60 = OpLoad %v2float %39
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %59 %61
%63 = OpLoad %v3float %40
%64 = OpCompositeExtract %float %63 0
%65 = OpFAdd %float %62 %64
%66 = OpLoad %v2int %41
%67 = OpCompositeExtract %int %66 0
%68 = OpConvertSToF %float %67
%69 = OpFAdd %float %65 %68
%70 = OpLoad %v2int %42
%71 = OpCompositeExtract %int %70 0
%72 = OpConvertSToF %float %71
%73 = OpFAdd %float %69 %72
%74 = OpLoad %v2float %43
%75 = OpCompositeExtract %float %74 0
%76 = OpFAdd %float %73 %75
%77 = OpLoad %v2float %44
%78 = OpCompositeExtract %float %77 0
%79 = OpFAdd %float %76 %78
%80 = OpLoad %v4float %45
%81 = OpCompositeExtract %float %80 0
%82 = OpFAdd %float %79 %81
%83 = OpLoad %v2int %46
%84 = OpCompositeExtract %int %83 0
%85 = OpConvertSToF %float %84
%86 = OpFAdd %float %82 %85
%87 = OpLoad %v4bool %47
%88 = OpCompositeExtract %bool %87 0
%89 = OpSelect %float %88 %float_1 %float_0
%91 = OpFAdd %float %86 %89
%92 = OpLoad %v2float %48
%93 = OpCompositeExtract %float %92 0
%94 = OpFAdd %float %91 %93
%95 = OpLoad %v2float %49
%96 = OpCompositeExtract %float %95 0
%97 = OpFAdd %float %94 %96
%98 = OpLoad %v2float %50
%99 = OpCompositeExtract %float %98 0
%100 = OpFAdd %float %97 %99
%101 = OpLoad %v2bool %51
%102 = OpCompositeExtract %bool %101 0
%103 = OpSelect %float %102 %float_1 %float_0
%104 = OpFAdd %float %100 %103
%105 = OpLoad %v2bool %52
%106 = OpCompositeExtract %bool %105 0
%107 = OpSelect %float %106 %float_1 %float_0
%108 = OpFAdd %float %104 %107
%109 = OpLoad %v3bool %53
%110 = OpCompositeExtract %bool %109 0
%111 = OpSelect %float %110 %float_1 %float_0
%112 = OpFAdd %float %108 %111
%114 = OpFOrdEqual %bool %112 %float_17
OpReturnValue %114
OpFunctionEnd
%main = OpFunction %v4float None %115
%116 = OpFunctionParameter %_ptr_Function_v2float
%117 = OpLabel
%v1 = OpVariable %_ptr_Function_v2float Function
%v2 = OpVariable %_ptr_Function_v2float Function
%v3 = OpVariable %_ptr_Function_v2float Function
%v4 = OpVariable %_ptr_Function_v3float Function
%v5 = OpVariable %_ptr_Function_v2int Function
%v6 = OpVariable %_ptr_Function_v2int Function
%v7 = OpVariable %_ptr_Function_v2float Function
%v8 = OpVariable %_ptr_Function_v2float Function
%v9 = OpVariable %_ptr_Function_v4float Function
%v10 = OpVariable %_ptr_Function_v2int Function
%v11 = OpVariable %_ptr_Function_v4bool Function
%v12 = OpVariable %_ptr_Function_v2float Function
%v13 = OpVariable %_ptr_Function_v2float Function
%v14 = OpVariable %_ptr_Function_v2float Function
%v15 = OpVariable %_ptr_Function_v2bool Function
%v16 = OpVariable %_ptr_Function_v2bool Function
%v17 = OpVariable %_ptr_Function_v3bool Function
%170 = OpVariable %_ptr_Function_v2float Function
%172 = OpVariable %_ptr_Function_v2float Function
%174 = OpVariable %_ptr_Function_v2float Function
%176 = OpVariable %_ptr_Function_v3float Function
%178 = OpVariable %_ptr_Function_v2int Function
%180 = OpVariable %_ptr_Function_v2int Function
%182 = OpVariable %_ptr_Function_v2float Function
%184 = OpVariable %_ptr_Function_v2float Function
%186 = OpVariable %_ptr_Function_v4float Function
%188 = OpVariable %_ptr_Function_v2int Function
%190 = OpVariable %_ptr_Function_v4bool Function
%192 = OpVariable %_ptr_Function_v2float Function
%194 = OpVariable %_ptr_Function_v2float Function
%196 = OpVariable %_ptr_Function_v2float Function
%198 = OpVariable %_ptr_Function_v2bool Function
%200 = OpVariable %_ptr_Function_v2bool Function
%202 = OpVariable %_ptr_Function_v3bool Function
%204 = OpVariable %_ptr_Function_v4float Function
OpStore %v1 %119
OpStore %v2 %122
OpStore %v3 %119
OpStore %v4 %125
OpStore %v5 %128
OpStore %v6 %131
OpStore %v7 %122
%134 = OpLoad %v2int %v5
%135 = OpCompositeExtract %int %134 0
%136 = OpConvertSToF %float %135
%137 = OpCompositeExtract %int %134 1
%138 = OpConvertSToF %float %137
%139 = OpCompositeConstruct %v2float %136 %138
OpStore %v8 %139
%141 = OpLoad %v2int %v6
%142 = OpCompositeExtract %int %141 0
%143 = OpConvertSToF %float %142
%144 = OpAccessChain %_ptr_Uniform_float %11 %int_2
%146 = OpLoad %float %144
%149 = OpCompositeConstruct %v4float %143 %146 %float_3 %float_4
OpStore %v9 %149
%152 = OpLoad %v2float %v1
%153 = OpCompositeExtract %float %152 0
%154 = OpConvertFToS %int %153
%155 = OpCompositeConstruct %v2int %int_3 %154
OpStore %v10 %155
OpStore %v11 %159
OpStore %v12 %161
OpStore %v13 %20
OpStore %v14 %20
OpStore %v15 %165
OpStore %v16 %165
OpStore %v17 %168
%169 = OpLoad %v2float %v1
OpStore %170 %169
%171 = OpLoad %v2float %v2
OpStore %172 %171
%173 = OpLoad %v2float %v3
OpStore %174 %173
%175 = OpLoad %v3float %v4
OpStore %176 %175
%177 = OpLoad %v2int %v5
OpStore %178 %177
%179 = OpLoad %v2int %v6
OpStore %180 %179
%181 = OpLoad %v2float %v7
OpStore %182 %181
%183 = OpLoad %v2float %v8
OpStore %184 %183
%185 = OpLoad %v4float %v9
OpStore %186 %185
%187 = OpLoad %v2int %v10
OpStore %188 %187
%189 = OpLoad %v4bool %v11
OpStore %190 %189
%191 = OpLoad %v2float %v12
OpStore %192 %191
%193 = OpLoad %v2float %v13
OpStore %194 %193
%195 = OpLoad %v2float %v14
OpStore %196 %195
%197 = OpLoad %v2bool %v15
OpStore %198 %197
%199 = OpLoad %v2bool %v16
OpStore %200 %199
%201 = OpLoad %v3bool %v17
OpStore %202 %201
%203 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 %170 %172 %174 %176 %178 %180 %182 %184 %186 %188 %190 %192 %194 %196 %198 %200 %202
OpSelectionMerge %207 None
OpBranchConditional %203 %205 %206
%205 = OpLabel
%208 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%211 = OpLoad %v4float %208
OpStore %204 %211
OpBranch %207
%206 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%213 = OpLoad %v4float %212
OpStore %204 %213
OpBranch %207
%207 = OpLabel
%214 = OpLoad %v4float %204
OpReturnValue %214
OpFunctionEnd
