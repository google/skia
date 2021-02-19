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
OpName %check "check"
OpName %main "main"
OpName %v9 "v9"
OpName %v10 "v10"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
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
%float_2 = OpConstant %float 2
%117 = OpConstantComposite %v2float %float_1 %float_2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%128 = OpConstantComposite %v2int %int_3 %int_4
%138 = OpConstantComposite %v2float %float_1 %float_1
%145 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%148 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%157 = OpConstantComposite %v2int %int_1 %int_2
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%176 = OpConstantComposite %v4bool %true %false %true %false
%178 = OpConstantComposite %v2float %float_1 %float_0
%180 = OpConstantComposite %v2float %float_0 %float_0
%182 = OpConstantComposite %v2bool %false %false
%189 = OpConstantComposite %v2bool %true %true
%197 = OpConstantComposite %v3bool %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%check = OpFunction %bool None %26
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
%v9 = OpVariable %_ptr_Function_v4float Function
%v10 = OpVariable %_ptr_Function_v2int Function
%142 = OpVariable %_ptr_Function_v2float Function
%143 = OpVariable %_ptr_Function_v2float Function
%144 = OpVariable %_ptr_Function_v2float Function
%146 = OpVariable %_ptr_Function_v3float Function
%149 = OpVariable %_ptr_Function_v2int Function
%155 = OpVariable %_ptr_Function_v2int Function
%163 = OpVariable %_ptr_Function_v2float Function
%169 = OpVariable %_ptr_Function_v2float Function
%171 = OpVariable %_ptr_Function_v4float Function
%173 = OpVariable %_ptr_Function_v2int Function
%177 = OpVariable %_ptr_Function_v4bool Function
%179 = OpVariable %_ptr_Function_v2float Function
%181 = OpVariable %_ptr_Function_v2float Function
%188 = OpVariable %_ptr_Function_v2float Function
%190 = OpVariable %_ptr_Function_v2bool Function
%196 = OpVariable %_ptr_Function_v2bool Function
%198 = OpVariable %_ptr_Function_v3bool Function
%200 = OpVariable %_ptr_Function_v4float Function
%118 = OpCompositeExtract %float %117 0
%119 = OpConvertFToS %int %118
%120 = OpCompositeExtract %float %117 1
%121 = OpConvertFToS %int %120
%122 = OpCompositeConstruct %v2int %119 %121
%123 = OpCompositeExtract %int %122 0
%124 = OpConvertSToF %float %123
%125 = OpExtInst %float %1 Sqrt %float_2
%129 = OpCompositeExtract %int %128 0
%130 = OpConvertSToF %float %129
%131 = OpCompositeExtract %int %128 1
%132 = OpConvertSToF %float %131
%133 = OpCompositeConstruct %v2float %130 %132
%134 = OpCompositeExtract %float %133 0
%135 = OpCompositeExtract %float %133 1
%136 = OpCompositeConstruct %v4float %124 %125 %134 %135
OpStore %v9 %136
%139 = OpCompositeExtract %float %138 0
%140 = OpConvertFToS %int %139
%141 = OpCompositeConstruct %v2int %int_3 %140
OpStore %v10 %141
OpStore %142 %138
OpStore %143 %117
OpStore %144 %138
OpStore %146 %145
OpStore %149 %148
%150 = OpCompositeExtract %float %117 0
%151 = OpConvertFToS %int %150
%152 = OpCompositeExtract %float %117 1
%153 = OpConvertFToS %int %152
%154 = OpCompositeConstruct %v2int %151 %153
OpStore %155 %154
%158 = OpCompositeExtract %int %157 0
%159 = OpConvertSToF %float %158
%160 = OpCompositeExtract %int %157 1
%161 = OpConvertSToF %float %160
%162 = OpCompositeConstruct %v2float %159 %161
OpStore %163 %162
%164 = OpCompositeExtract %int %148 0
%165 = OpConvertSToF %float %164
%166 = OpCompositeExtract %int %148 1
%167 = OpConvertSToF %float %166
%168 = OpCompositeConstruct %v2float %165 %167
OpStore %169 %168
%170 = OpLoad %v4float %v9
OpStore %171 %170
%172 = OpLoad %v2int %v10
OpStore %173 %172
OpStore %177 %176
OpStore %179 %178
OpStore %181 %180
%183 = OpCompositeExtract %bool %182 0
%184 = OpSelect %float %183 %float_1 %float_0
%185 = OpCompositeExtract %bool %182 1
%186 = OpSelect %float %185 %float_1 %float_0
%187 = OpCompositeConstruct %v2float %184 %186
OpStore %188 %187
OpStore %190 %189
%191 = OpCompositeExtract %float %138 0
%192 = OpFUnordNotEqual %bool %191 %float_0
%193 = OpCompositeExtract %float %138 1
%194 = OpFUnordNotEqual %bool %193 %float_0
%195 = OpCompositeConstruct %v2bool %192 %194
OpStore %196 %195
OpStore %198 %197
%199 = OpFunctionCall %bool %check %142 %143 %144 %146 %149 %155 %163 %169 %171 %173 %177 %179 %181 %188 %190 %196 %198
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
