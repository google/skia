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
OpName %main "main"
OpName %v "v"
OpName %result "result"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%187 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 1
%29 = OpFUnordNotEqual %bool %28 %float_0
%31 = OpCompositeConstruct %v4bool %29 %29 %29 %29
OpStore %v %31
%33 = OpLoad %v4bool %v
%34 = OpCompositeExtract %bool %33 0
%36 = OpCompositeConstruct %v4bool %34 %true %true %true
OpStore %result %36
%37 = OpLoad %v4bool %v
%38 = OpVectorShuffle %v2bool %37 %37 0 1
%40 = OpCompositeExtract %bool %38 0
%41 = OpCompositeExtract %bool %38 1
%43 = OpCompositeConstruct %v4bool %40 %41 %false %true
OpStore %result %43
%44 = OpLoad %v4bool %v
%45 = OpCompositeExtract %bool %44 0
%46 = OpCompositeConstruct %v4bool %45 %true %true %false
OpStore %result %46
%47 = OpLoad %v4bool %v
%48 = OpCompositeExtract %bool %47 1
%49 = OpCompositeConstruct %v2bool %false %48
%50 = OpCompositeExtract %bool %49 0
%51 = OpCompositeExtract %bool %49 1
%52 = OpCompositeConstruct %v4bool %50 %51 %true %true
OpStore %result %52
%53 = OpLoad %v4bool %v
%54 = OpVectorShuffle %v3bool %53 %53 0 1 2
%56 = OpCompositeExtract %bool %54 0
%57 = OpCompositeExtract %bool %54 1
%58 = OpCompositeExtract %bool %54 2
%59 = OpCompositeConstruct %v4bool %56 %57 %58 %true
OpStore %result %59
%60 = OpLoad %v4bool %v
%61 = OpVectorShuffle %v2bool %60 %60 0 1
%62 = OpCompositeExtract %bool %61 0
%63 = OpCompositeExtract %bool %61 1
%64 = OpCompositeConstruct %v4bool %62 %63 %true %true
OpStore %result %64
%65 = OpLoad %v4bool %v
%66 = OpVectorShuffle %v2bool %65 %65 0 2
%67 = OpCompositeExtract %bool %66 0
%68 = OpLoad %v4bool %v
%69 = OpVectorShuffle %v2bool %68 %68 0 2
%70 = OpCompositeExtract %bool %69 1
%71 = OpCompositeConstruct %v3bool %67 %false %70
%72 = OpCompositeExtract %bool %71 0
%73 = OpCompositeExtract %bool %71 1
%74 = OpCompositeExtract %bool %71 2
%75 = OpCompositeConstruct %v4bool %72 %73 %74 %true
OpStore %result %75
%76 = OpLoad %v4bool %v
%77 = OpCompositeExtract %bool %76 0
%78 = OpCompositeConstruct %v4bool %77 %true %false %false
OpStore %result %78
%79 = OpLoad %v4bool %v
%80 = OpVectorShuffle %v2bool %79 %79 1 2
%81 = OpVectorShuffle %v2bool %80 %80 0 1
%82 = OpCompositeExtract %bool %81 0
%83 = OpCompositeExtract %bool %81 1
%84 = OpCompositeConstruct %v3bool %true %82 %83
%85 = OpCompositeExtract %bool %84 0
%86 = OpCompositeExtract %bool %84 1
%87 = OpCompositeExtract %bool %84 2
%88 = OpCompositeConstruct %v4bool %85 %86 %87 %false
OpStore %result %88
%89 = OpLoad %v4bool %v
%90 = OpCompositeExtract %bool %89 1
%91 = OpCompositeConstruct %v3bool %false %90 %true
%92 = OpCompositeExtract %bool %91 0
%93 = OpCompositeExtract %bool %91 1
%94 = OpCompositeExtract %bool %91 2
%95 = OpCompositeConstruct %v4bool %92 %93 %94 %false
OpStore %result %95
%96 = OpLoad %v4bool %v
%97 = OpCompositeExtract %bool %96 2
%98 = OpCompositeConstruct %v3bool %true %true %97
%99 = OpCompositeExtract %bool %98 0
%100 = OpCompositeExtract %bool %98 1
%101 = OpCompositeExtract %bool %98 2
%102 = OpCompositeConstruct %v4bool %99 %100 %101 %false
OpStore %result %102
%103 = OpLoad %v4bool %v
OpStore %result %103
%104 = OpLoad %v4bool %v
%105 = OpVectorShuffle %v3bool %104 %104 0 1 2
%106 = OpCompositeExtract %bool %105 0
%107 = OpCompositeExtract %bool %105 1
%108 = OpCompositeExtract %bool %105 2
%109 = OpCompositeConstruct %v4bool %106 %107 %108 %true
OpStore %result %109
%110 = OpLoad %v4bool %v
%111 = OpVectorShuffle %v3bool %110 %110 0 1 3
%112 = OpVectorShuffle %v2bool %111 %111 0 1
%113 = OpCompositeExtract %bool %112 0
%114 = OpCompositeExtract %bool %112 1
%115 = OpLoad %v4bool %v
%116 = OpVectorShuffle %v3bool %115 %115 0 1 3
%117 = OpCompositeExtract %bool %116 2
%118 = OpCompositeConstruct %v4bool %113 %114 %false %117
OpStore %result %118
%119 = OpLoad %v4bool %v
%120 = OpVectorShuffle %v2bool %119 %119 0 1
%121 = OpCompositeExtract %bool %120 0
%122 = OpCompositeExtract %bool %120 1
%123 = OpCompositeConstruct %v4bool %121 %122 %true %false
OpStore %result %123
%124 = OpLoad %v4bool %v
%125 = OpVectorShuffle %v3bool %124 %124 0 2 3
%126 = OpCompositeExtract %bool %125 0
%127 = OpLoad %v4bool %v
%128 = OpVectorShuffle %v3bool %127 %127 0 2 3
%129 = OpVectorShuffle %v2bool %128 %128 1 2
%130 = OpCompositeExtract %bool %129 0
%131 = OpCompositeExtract %bool %129 1
%132 = OpCompositeConstruct %v4bool %126 %true %130 %131
OpStore %result %132
%133 = OpLoad %v4bool %v
%134 = OpVectorShuffle %v2bool %133 %133 0 2
%135 = OpCompositeExtract %bool %134 0
%136 = OpLoad %v4bool %v
%137 = OpVectorShuffle %v2bool %136 %136 0 2
%138 = OpCompositeExtract %bool %137 1
%139 = OpCompositeConstruct %v4bool %135 %false %138 %true
OpStore %result %139
%140 = OpLoad %v4bool %v
%141 = OpVectorShuffle %v2bool %140 %140 0 3
%142 = OpCompositeExtract %bool %141 0
%143 = OpLoad %v4bool %v
%144 = OpVectorShuffle %v2bool %143 %143 0 3
%145 = OpCompositeExtract %bool %144 1
%146 = OpCompositeConstruct %v4bool %142 %true %true %145
OpStore %result %146
%147 = OpLoad %v4bool %v
%148 = OpCompositeExtract %bool %147 0
%149 = OpCompositeConstruct %v4bool %148 %true %false %true
OpStore %result %149
%150 = OpLoad %v4bool %v
%151 = OpVectorShuffle %v3bool %150 %150 1 2 3
%152 = OpVectorShuffle %v3bool %151 %151 0 1 2
%153 = OpCompositeExtract %bool %152 0
%154 = OpCompositeExtract %bool %152 1
%155 = OpCompositeExtract %bool %152 2
%156 = OpCompositeConstruct %v4bool %true %153 %154 %155
OpStore %result %156
%157 = OpLoad %v4bool %v
%158 = OpVectorShuffle %v2bool %157 %157 1 2
%159 = OpVectorShuffle %v2bool %158 %158 0 1
%160 = OpCompositeExtract %bool %159 0
%161 = OpCompositeExtract %bool %159 1
%162 = OpCompositeConstruct %v4bool %false %160 %161 %true
OpStore %result %162
%163 = OpLoad %v4bool %v
%164 = OpVectorShuffle %v2bool %163 %163 1 3
%165 = OpCompositeExtract %bool %164 0
%166 = OpLoad %v4bool %v
%167 = OpVectorShuffle %v2bool %166 %166 1 3
%168 = OpCompositeExtract %bool %167 1
%169 = OpCompositeConstruct %v4bool %false %165 %true %168
OpStore %result %169
%170 = OpLoad %v4bool %v
%171 = OpCompositeExtract %bool %170 1
%172 = OpCompositeConstruct %v4bool %true %171 %true %true
OpStore %result %172
%173 = OpLoad %v4bool %v
%174 = OpVectorShuffle %v2bool %173 %173 2 3
%175 = OpVectorShuffle %v2bool %174 %174 0 1
%176 = OpCompositeExtract %bool %175 0
%177 = OpCompositeExtract %bool %175 1
%178 = OpCompositeConstruct %v4bool %false %false %176 %177
OpStore %result %178
%179 = OpLoad %v4bool %v
%180 = OpCompositeExtract %bool %179 2
%181 = OpCompositeConstruct %v4bool %false %false %180 %true
OpStore %result %181
%182 = OpLoad %v4bool %v
%183 = OpCompositeExtract %bool %182 3
%184 = OpCompositeConstruct %v4bool %false %true %true %183
OpStore %result %184
%186 = OpLoad %v4bool %result
%185 = OpAny %bool %186
OpSelectionMerge %191 None
OpBranchConditional %185 %189 %190
%189 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%193 = OpLoad %v4float %192
OpStore %187 %193
OpBranch %191
%190 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%196 = OpLoad %v4float %194
OpStore %187 %196
OpBranch %191
%191 = OpLabel
%197 = OpLoad %v4float %187
OpReturnValue %197
OpFunctionEnd
