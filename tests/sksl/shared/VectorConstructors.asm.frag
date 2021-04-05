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
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%26 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_17 = OpConstant %float 17
%113 = OpTypeFunction %v4float
%116 = OpConstantComposite %v2float %float_1 %float_1
%float_2 = OpConstant %float 2
%119 = OpConstantComposite %v2float %float_1 %float_2
%122 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%125 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%128 = OpConstantComposite %v2int %int_1 %int_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%154 = OpConstantComposite %v4bool %true %false %true %false
%156 = OpConstantComposite %v2float %float_1 %float_0
%158 = OpConstantComposite %v2float %float_0 %float_0
%161 = OpConstantComposite %v2bool %true %true
%164 = OpConstantComposite %v3bool %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 = OpFunction %bool None %26
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpFunctionParameter %_ptr_Function_v3float
%38 = OpFunctionParameter %_ptr_Function_v2int
%39 = OpFunctionParameter %_ptr_Function_v2int
%40 = OpFunctionParameter %_ptr_Function_v2float
%41 = OpFunctionParameter %_ptr_Function_v2float
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpFunctionParameter %_ptr_Function_v2int
%44 = OpFunctionParameter %_ptr_Function_v4bool
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpFunctionParameter %_ptr_Function_v2float
%47 = OpFunctionParameter %_ptr_Function_v2float
%48 = OpFunctionParameter %_ptr_Function_v2bool
%49 = OpFunctionParameter %_ptr_Function_v2bool
%50 = OpFunctionParameter %_ptr_Function_v3bool
%51 = OpLabel
%52 = OpLoad %v2float %34
%53 = OpCompositeExtract %float %52 0
%54 = OpLoad %v2float %35
%55 = OpCompositeExtract %float %54 0
%56 = OpFAdd %float %53 %55
%57 = OpLoad %v2float %36
%58 = OpCompositeExtract %float %57 0
%59 = OpFAdd %float %56 %58
%60 = OpLoad %v3float %37
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %59 %61
%63 = OpLoad %v2int %38
%64 = OpCompositeExtract %int %63 0
%65 = OpConvertSToF %float %64
%66 = OpFAdd %float %62 %65
%67 = OpLoad %v2int %39
%68 = OpCompositeExtract %int %67 0
%69 = OpConvertSToF %float %68
%70 = OpFAdd %float %66 %69
%71 = OpLoad %v2float %40
%72 = OpCompositeExtract %float %71 0
%73 = OpFAdd %float %70 %72
%74 = OpLoad %v2float %41
%75 = OpCompositeExtract %float %74 0
%76 = OpFAdd %float %73 %75
%77 = OpLoad %v4float %42
%78 = OpCompositeExtract %float %77 0
%79 = OpFAdd %float %76 %78
%80 = OpLoad %v2int %43
%81 = OpCompositeExtract %int %80 0
%82 = OpConvertSToF %float %81
%83 = OpFAdd %float %79 %82
%84 = OpLoad %v4bool %44
%85 = OpCompositeExtract %bool %84 0
%86 = OpSelect %float %85 %float_1 %float_0
%89 = OpFAdd %float %83 %86
%90 = OpLoad %v2float %45
%91 = OpCompositeExtract %float %90 0
%92 = OpFAdd %float %89 %91
%93 = OpLoad %v2float %46
%94 = OpCompositeExtract %float %93 0
%95 = OpFAdd %float %92 %94
%96 = OpLoad %v2float %47
%97 = OpCompositeExtract %float %96 0
%98 = OpFAdd %float %95 %97
%99 = OpLoad %v2bool %48
%100 = OpCompositeExtract %bool %99 0
%101 = OpSelect %float %100 %float_1 %float_0
%102 = OpFAdd %float %98 %101
%103 = OpLoad %v2bool %49
%104 = OpCompositeExtract %bool %103 0
%105 = OpSelect %float %104 %float_1 %float_0
%106 = OpFAdd %float %102 %105
%107 = OpLoad %v3bool %50
%108 = OpCompositeExtract %bool %107 0
%109 = OpSelect %float %108 %float_1 %float_0
%110 = OpFAdd %float %106 %109
%112 = OpFOrdEqual %bool %110 %float_17
OpReturnValue %112
OpFunctionEnd
%main = OpFunction %v4float None %113
%114 = OpLabel
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
%166 = OpVariable %_ptr_Function_v2float Function
%168 = OpVariable %_ptr_Function_v2float Function
%170 = OpVariable %_ptr_Function_v2float Function
%172 = OpVariable %_ptr_Function_v3float Function
%174 = OpVariable %_ptr_Function_v2int Function
%176 = OpVariable %_ptr_Function_v2int Function
%178 = OpVariable %_ptr_Function_v2float Function
%180 = OpVariable %_ptr_Function_v2float Function
%182 = OpVariable %_ptr_Function_v4float Function
%184 = OpVariable %_ptr_Function_v2int Function
%186 = OpVariable %_ptr_Function_v4bool Function
%188 = OpVariable %_ptr_Function_v2float Function
%190 = OpVariable %_ptr_Function_v2float Function
%192 = OpVariable %_ptr_Function_v2float Function
%194 = OpVariable %_ptr_Function_v2bool Function
%196 = OpVariable %_ptr_Function_v2bool Function
%198 = OpVariable %_ptr_Function_v3bool Function
%200 = OpVariable %_ptr_Function_v4float Function
OpStore %v1 %116
OpStore %v2 %119
OpStore %v3 %116
OpStore %v4 %122
OpStore %v5 %125
OpStore %v6 %128
OpStore %v7 %119
%131 = OpLoad %v2int %v5
%132 = OpCompositeExtract %int %131 0
%133 = OpConvertSToF %float %132
%134 = OpCompositeExtract %int %131 1
%135 = OpConvertSToF %float %134
%136 = OpCompositeConstruct %v2float %133 %135
OpStore %v8 %136
%138 = OpLoad %v2int %v6
%139 = OpCompositeExtract %int %138 0
%140 = OpConvertSToF %float %139
%141 = OpExtInst %float %1 Sqrt %float_2
%144 = OpCompositeConstruct %v4float %140 %141 %float_3 %float_4
OpStore %v9 %144
%147 = OpLoad %v2float %v1
%148 = OpCompositeExtract %float %147 0
%149 = OpConvertFToS %int %148
%150 = OpCompositeConstruct %v2int %int_3 %149
OpStore %v10 %150
OpStore %v11 %154
OpStore %v12 %156
OpStore %v13 %158
OpStore %v14 %158
OpStore %v15 %161
OpStore %v16 %161
OpStore %v17 %164
%165 = OpLoad %v2float %v1
OpStore %166 %165
%167 = OpLoad %v2float %v2
OpStore %168 %167
%169 = OpLoad %v2float %v3
OpStore %170 %169
%171 = OpLoad %v3float %v4
OpStore %172 %171
%173 = OpLoad %v2int %v5
OpStore %174 %173
%175 = OpLoad %v2int %v6
OpStore %176 %175
%177 = OpLoad %v2float %v7
OpStore %178 %177
%179 = OpLoad %v2float %v8
OpStore %180 %179
%181 = OpLoad %v4float %v9
OpStore %182 %181
%183 = OpLoad %v2int %v10
OpStore %184 %183
%185 = OpLoad %v4bool %v11
OpStore %186 %185
%187 = OpLoad %v2float %v12
OpStore %188 %187
%189 = OpLoad %v2float %v13
OpStore %190 %189
%191 = OpLoad %v2float %v14
OpStore %192 %191
%193 = OpLoad %v2bool %v15
OpStore %194 %193
%195 = OpLoad %v2bool %v16
OpStore %196 %195
%197 = OpLoad %v3bool %v17
OpStore %198 %197
%199 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 %166 %168 %170 %172 %174 %176 %178 %180 %182 %184 %186 %188 %190 %192 %194 %196 %198
OpSelectionMerge %203 None
OpBranchConditional %199 %201 %202
%201 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%207 = OpLoad %v4float %204
OpStore %200 %207
OpBranch %203
%202 = OpLabel
%208 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%209 = OpLoad %v4float %208
OpStore %200 %209
OpBranch %203
%203 = OpLabel
%210 = OpLoad %v4float %200
OpReturnValue %210
OpFunctionEnd
