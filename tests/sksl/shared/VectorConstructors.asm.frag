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
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
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
%130 = OpConstantComposite %v2float %float_1 %float_1
%135 = OpConstantComposite %v2float %float_1 %float_2
%138 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%141 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%150 = OpConstantComposite %v2int %int_1 %int_2
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%169 = OpConstantComposite %v4bool %true %false %true %false
%171 = OpConstantComposite %v2float %float_1 %float_0
%173 = OpConstantComposite %v2float %float_0 %float_0
%175 = OpConstantComposite %v2bool %false %false
%182 = OpConstantComposite %v2bool %true %true
%190 = OpConstantComposite %v3bool %true %true %true
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
%134 = OpVariable %_ptr_Function_v2float Function
%136 = OpVariable %_ptr_Function_v2float Function
%137 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v3float Function
%142 = OpVariable %_ptr_Function_v2int Function
%148 = OpVariable %_ptr_Function_v2int Function
%156 = OpVariable %_ptr_Function_v2float Function
%162 = OpVariable %_ptr_Function_v2float Function
%164 = OpVariable %_ptr_Function_v4float Function
%166 = OpVariable %_ptr_Function_v2int Function
%170 = OpVariable %_ptr_Function_v4bool Function
%172 = OpVariable %_ptr_Function_v2float Function
%174 = OpVariable %_ptr_Function_v2float Function
%181 = OpVariable %_ptr_Function_v2float Function
%183 = OpVariable %_ptr_Function_v2bool Function
%189 = OpVariable %_ptr_Function_v2bool Function
%191 = OpVariable %_ptr_Function_v3bool Function
%193 = OpVariable %_ptr_Function_v4float Function
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
%131 = OpCompositeExtract %float %130 0
%132 = OpConvertFToS %int %131
%133 = OpCompositeConstruct %v2int %int_3 %132
OpStore %v10 %133
OpStore %134 %130
OpStore %136 %135
OpStore %137 %130
OpStore %139 %138
OpStore %142 %141
%143 = OpCompositeExtract %float %135 0
%144 = OpConvertFToS %int %143
%145 = OpCompositeExtract %float %135 1
%146 = OpConvertFToS %int %145
%147 = OpCompositeConstruct %v2int %144 %146
OpStore %148 %147
%151 = OpCompositeExtract %int %150 0
%152 = OpConvertSToF %float %151
%153 = OpCompositeExtract %int %150 1
%154 = OpConvertSToF %float %153
%155 = OpCompositeConstruct %v2float %152 %154
OpStore %156 %155
%157 = OpCompositeExtract %int %141 0
%158 = OpConvertSToF %float %157
%159 = OpCompositeExtract %int %141 1
%160 = OpConvertSToF %float %159
%161 = OpCompositeConstruct %v2float %158 %160
OpStore %162 %161
%163 = OpLoad %v4float %v9
OpStore %164 %163
%165 = OpLoad %v2int %v10
OpStore %166 %165
OpStore %170 %169
OpStore %172 %171
OpStore %174 %173
%176 = OpCompositeExtract %bool %175 0
%177 = OpSelect %float %176 %float_1 %float_0
%178 = OpCompositeExtract %bool %175 1
%179 = OpSelect %float %178 %float_1 %float_0
%180 = OpCompositeConstruct %v2float %177 %179
OpStore %181 %180
OpStore %183 %182
%184 = OpCompositeExtract %float %130 0
%185 = OpFUnordNotEqual %bool %184 %float_0
%186 = OpCompositeExtract %float %130 1
%187 = OpFUnordNotEqual %bool %186 %float_0
%188 = OpCompositeConstruct %v2bool %185 %187
OpStore %189 %188
OpStore %191 %190
%192 = OpFunctionCall %bool %check %134 %136 %137 %139 %142 %148 %156 %162 %164 %166 %170 %172 %174 %181 %183 %189 %191
OpSelectionMerge %196 None
OpBranchConditional %192 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%200 = OpLoad %v4float %197
OpStore %193 %200
OpBranch %196
%195 = OpLabel
%201 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%202 = OpLoad %v4float %201
OpStore %193 %202
OpBranch %196
%196 = OpLabel
%203 = OpLoad %v4float %193
OpReturnValue %203
OpFunctionEnd
