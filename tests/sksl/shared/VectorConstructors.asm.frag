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
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
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
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%120 = OpConstantComposite %v2int %int_3 %int_4
%129 = OpConstantComposite %v2float %float_1 %float_1
%131 = OpConstantComposite %v2float %float_1 %float_2
%134 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%137 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%146 = OpConstantComposite %v2int %int_1 %int_2
%161 = OpConstantComposite %v2int %int_3 %int_1
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%165 = OpConstantComposite %v4bool %true %false %true %false
%167 = OpConstantComposite %v2float %float_1 %float_0
%169 = OpConstantComposite %v2float %float_0 %float_0
%171 = OpConstantComposite %v2bool %false %false
%178 = OpConstantComposite %v2bool %true %true
%186 = OpConstantComposite %v3bool %true %true %true
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
%130 = OpVariable %_ptr_Function_v2float Function
%132 = OpVariable %_ptr_Function_v2float Function
%133 = OpVariable %_ptr_Function_v2float Function
%135 = OpVariable %_ptr_Function_v3float Function
%138 = OpVariable %_ptr_Function_v2int Function
%144 = OpVariable %_ptr_Function_v2int Function
%152 = OpVariable %_ptr_Function_v2float Function
%158 = OpVariable %_ptr_Function_v2float Function
%160 = OpVariable %_ptr_Function_v4float Function
%162 = OpVariable %_ptr_Function_v2int Function
%166 = OpVariable %_ptr_Function_v4bool Function
%168 = OpVariable %_ptr_Function_v2float Function
%170 = OpVariable %_ptr_Function_v2float Function
%177 = OpVariable %_ptr_Function_v2float Function
%179 = OpVariable %_ptr_Function_v2bool Function
%185 = OpVariable %_ptr_Function_v2bool Function
%187 = OpVariable %_ptr_Function_v3bool Function
%189 = OpVariable %_ptr_Function_v4float Function
%116 = OpExtInst %float %1 Sqrt %float_2
%121 = OpCompositeExtract %int %120 0
%122 = OpConvertSToF %float %121
%123 = OpCompositeExtract %int %120 1
%124 = OpConvertSToF %float %123
%125 = OpCompositeConstruct %v2float %122 %124
%126 = OpCompositeExtract %float %125 0
%127 = OpCompositeExtract %float %125 1
%128 = OpCompositeConstruct %v4float %float_1 %116 %126 %127
OpStore %v9 %128
OpStore %130 %129
OpStore %132 %131
OpStore %133 %129
OpStore %135 %134
OpStore %138 %137
%139 = OpCompositeExtract %float %131 0
%140 = OpConvertFToS %int %139
%141 = OpCompositeExtract %float %131 1
%142 = OpConvertFToS %int %141
%143 = OpCompositeConstruct %v2int %140 %142
OpStore %144 %143
%147 = OpCompositeExtract %int %146 0
%148 = OpConvertSToF %float %147
%149 = OpCompositeExtract %int %146 1
%150 = OpConvertSToF %float %149
%151 = OpCompositeConstruct %v2float %148 %150
OpStore %152 %151
%153 = OpCompositeExtract %int %137 0
%154 = OpConvertSToF %float %153
%155 = OpCompositeExtract %int %137 1
%156 = OpConvertSToF %float %155
%157 = OpCompositeConstruct %v2float %154 %156
OpStore %158 %157
%159 = OpLoad %v4float %v9
OpStore %160 %159
OpStore %162 %161
OpStore %166 %165
OpStore %168 %167
OpStore %170 %169
%172 = OpCompositeExtract %bool %171 0
%173 = OpSelect %float %172 %float_1 %float_0
%174 = OpCompositeExtract %bool %171 1
%175 = OpSelect %float %174 %float_1 %float_0
%176 = OpCompositeConstruct %v2float %173 %175
OpStore %177 %176
OpStore %179 %178
%180 = OpCompositeExtract %float %129 0
%181 = OpFUnordNotEqual %bool %180 %float_0
%182 = OpCompositeExtract %float %129 1
%183 = OpFUnordNotEqual %bool %182 %float_0
%184 = OpCompositeConstruct %v2bool %181 %183
OpStore %185 %184
OpStore %187 %186
%188 = OpFunctionCall %bool %check %130 %132 %133 %135 %138 %144 %152 %158 %160 %162 %166 %168 %170 %177 %179 %185 %187
OpSelectionMerge %192 None
OpBranchConditional %188 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%196 = OpLoad %v4float %193
OpStore %189 %196
OpBranch %192
%191 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%198 = OpLoad %v4float %197
OpStore %189 %198
OpBranch %192
%192 = OpLabel
%199 = OpLoad %v4float %189
OpReturnValue %199
OpFunctionEnd
