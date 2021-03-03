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
OpDecorate %50 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
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
%197 = OpVariable %_ptr_Function_v4float Function
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
%46 = OpCompositeConstruct %v2bool %45 %true
%47 = OpCompositeExtract %bool %46 0
%48 = OpCompositeExtract %bool %46 1
%49 = OpCompositeConstruct %v4bool %47 %48 %true %false
OpStore %result %49
%50 = OpLoad %v4bool %v
%51 = OpCompositeExtract %bool %50 1
%52 = OpCompositeConstruct %v2bool %51 %false
%53 = OpVectorShuffle %v2bool %52 %52 1 0
%54 = OpCompositeExtract %bool %53 0
%55 = OpCompositeExtract %bool %53 1
%56 = OpCompositeConstruct %v4bool %54 %55 %true %true
OpStore %result %56
%57 = OpLoad %v4bool %v
%58 = OpVectorShuffle %v3bool %57 %57 0 1 2
%60 = OpCompositeExtract %bool %58 0
%61 = OpCompositeExtract %bool %58 1
%62 = OpCompositeExtract %bool %58 2
%63 = OpCompositeConstruct %v4bool %60 %61 %62 %true
OpStore %result %63
%64 = OpLoad %v4bool %v
%65 = OpVectorShuffle %v2bool %64 %64 0 1
%66 = OpCompositeExtract %bool %65 0
%67 = OpCompositeExtract %bool %65 1
%68 = OpCompositeConstruct %v3bool %66 %67 %true
%69 = OpCompositeExtract %bool %68 0
%70 = OpCompositeExtract %bool %68 1
%71 = OpCompositeExtract %bool %68 2
%72 = OpCompositeConstruct %v4bool %69 %70 %71 %true
OpStore %result %72
%73 = OpLoad %v4bool %v
%74 = OpVectorShuffle %v2bool %73 %73 0 2
%75 = OpCompositeExtract %bool %74 0
%76 = OpCompositeExtract %bool %74 1
%77 = OpCompositeConstruct %v3bool %75 %76 %false
%78 = OpVectorShuffle %v3bool %77 %77 0 2 1
%79 = OpCompositeExtract %bool %78 0
%80 = OpCompositeExtract %bool %78 1
%81 = OpCompositeExtract %bool %78 2
%82 = OpCompositeConstruct %v4bool %79 %80 %81 %true
OpStore %result %82
%83 = OpLoad %v4bool %v
%84 = OpCompositeExtract %bool %83 0
%85 = OpCompositeConstruct %v3bool %84 %true %false
%86 = OpCompositeExtract %bool %85 0
%87 = OpCompositeExtract %bool %85 1
%88 = OpCompositeExtract %bool %85 2
%89 = OpCompositeConstruct %v4bool %86 %87 %88 %false
OpStore %result %89
%90 = OpLoad %v4bool %v
%91 = OpVectorShuffle %v2bool %90 %90 1 2
%92 = OpCompositeExtract %bool %91 0
%93 = OpCompositeExtract %bool %91 1
%94 = OpCompositeConstruct %v3bool %92 %93 %true
%95 = OpVectorShuffle %v3bool %94 %94 2 0 1
%96 = OpCompositeExtract %bool %95 0
%97 = OpCompositeExtract %bool %95 1
%98 = OpCompositeExtract %bool %95 2
%99 = OpCompositeConstruct %v4bool %96 %97 %98 %false
OpStore %result %99
%100 = OpLoad %v4bool %v
%101 = OpCompositeExtract %bool %100 1
%102 = OpCompositeConstruct %v3bool %101 %false %true
%103 = OpVectorShuffle %v3bool %102 %102 1 0 2
%104 = OpCompositeExtract %bool %103 0
%105 = OpCompositeExtract %bool %103 1
%106 = OpCompositeExtract %bool %103 2
%107 = OpCompositeConstruct %v4bool %104 %105 %106 %false
OpStore %result %107
%108 = OpLoad %v4bool %v
%109 = OpCompositeExtract %bool %108 2
%110 = OpCompositeConstruct %v2bool %109 %true
%111 = OpVectorShuffle %v3bool %110 %110 1 1 0
%112 = OpCompositeExtract %bool %111 0
%113 = OpCompositeExtract %bool %111 1
%114 = OpCompositeExtract %bool %111 2
%115 = OpCompositeConstruct %v4bool %112 %113 %114 %false
OpStore %result %115
%116 = OpLoad %v4bool %v
OpStore %result %116
%117 = OpLoad %v4bool %v
%118 = OpVectorShuffle %v3bool %117 %117 0 1 2
%119 = OpCompositeExtract %bool %118 0
%120 = OpCompositeExtract %bool %118 1
%121 = OpCompositeExtract %bool %118 2
%122 = OpCompositeConstruct %v4bool %119 %120 %121 %true
OpStore %result %122
%123 = OpLoad %v4bool %v
%124 = OpVectorShuffle %v3bool %123 %123 0 1 3
%125 = OpCompositeExtract %bool %124 0
%126 = OpCompositeExtract %bool %124 1
%127 = OpCompositeExtract %bool %124 2
%128 = OpCompositeConstruct %v4bool %125 %126 %127 %false
%129 = OpVectorShuffle %v4bool %128 %128 0 1 3 2
OpStore %result %129
%130 = OpLoad %v4bool %v
%131 = OpVectorShuffle %v2bool %130 %130 0 1
%132 = OpCompositeExtract %bool %131 0
%133 = OpCompositeExtract %bool %131 1
%134 = OpCompositeConstruct %v4bool %132 %133 %true %false
OpStore %result %134
%135 = OpLoad %v4bool %v
%136 = OpVectorShuffle %v3bool %135 %135 0 2 3
%137 = OpCompositeExtract %bool %136 0
%138 = OpCompositeExtract %bool %136 1
%139 = OpCompositeExtract %bool %136 2
%140 = OpCompositeConstruct %v4bool %137 %138 %139 %true
%141 = OpVectorShuffle %v4bool %140 %140 0 3 1 2
OpStore %result %141
%142 = OpLoad %v4bool %v
%143 = OpVectorShuffle %v2bool %142 %142 0 2
%144 = OpCompositeExtract %bool %143 0
%145 = OpCompositeExtract %bool %143 1
%146 = OpCompositeConstruct %v4bool %144 %145 %false %true
%147 = OpVectorShuffle %v4bool %146 %146 0 2 1 3
OpStore %result %147
%148 = OpLoad %v4bool %v
%149 = OpVectorShuffle %v2bool %148 %148 0 3
%150 = OpCompositeExtract %bool %149 0
%151 = OpCompositeExtract %bool %149 1
%152 = OpCompositeConstruct %v3bool %150 %151 %true
%153 = OpVectorShuffle %v4bool %152 %152 0 2 2 1
OpStore %result %153
%154 = OpLoad %v4bool %v
%155 = OpCompositeExtract %bool %154 0
%156 = OpCompositeConstruct %v3bool %155 %true %false
%157 = OpVectorShuffle %v4bool %156 %156 0 1 2 1
OpStore %result %157
%158 = OpLoad %v4bool %v
%159 = OpVectorShuffle %v3bool %158 %158 1 2 3
%160 = OpCompositeExtract %bool %159 0
%161 = OpCompositeExtract %bool %159 1
%162 = OpCompositeExtract %bool %159 2
%163 = OpCompositeConstruct %v4bool %160 %161 %162 %true
%164 = OpVectorShuffle %v4bool %163 %163 3 0 1 2
OpStore %result %164
%165 = OpLoad %v4bool %v
%166 = OpVectorShuffle %v2bool %165 %165 1 2
%167 = OpCompositeExtract %bool %166 0
%168 = OpCompositeExtract %bool %166 1
%169 = OpCompositeConstruct %v4bool %167 %168 %false %true
%170 = OpVectorShuffle %v4bool %169 %169 2 0 1 3
OpStore %result %170
%171 = OpLoad %v4bool %v
%172 = OpVectorShuffle %v2bool %171 %171 1 3
%173 = OpCompositeExtract %bool %172 0
%174 = OpCompositeExtract %bool %172 1
%175 = OpCompositeConstruct %v4bool %173 %174 %false %true
%176 = OpVectorShuffle %v4bool %175 %175 2 0 3 1
OpStore %result %176
%177 = OpLoad %v4bool %v
%178 = OpCompositeExtract %bool %177 1
%179 = OpCompositeConstruct %v2bool %178 %true
%180 = OpVectorShuffle %v4bool %179 %179 1 0 1 1
OpStore %result %180
%181 = OpLoad %v4bool %v
%182 = OpVectorShuffle %v2bool %181 %181 2 3
%183 = OpCompositeExtract %bool %182 0
%184 = OpCompositeExtract %bool %182 1
%185 = OpCompositeConstruct %v3bool %183 %184 %false
%186 = OpVectorShuffle %v4bool %185 %185 2 2 0 1
OpStore %result %186
%187 = OpLoad %v4bool %v
%188 = OpCompositeExtract %bool %187 2
%189 = OpCompositeConstruct %v3bool %188 %false %true
%190 = OpVectorShuffle %v4bool %189 %189 1 1 0 2
OpStore %result %190
%191 = OpLoad %v4bool %v
%192 = OpCompositeExtract %bool %191 3
%193 = OpCompositeConstruct %v3bool %192 %false %true
%194 = OpVectorShuffle %v4bool %193 %193 1 2 2 0
OpStore %result %194
%196 = OpLoad %v4bool %result
%195 = OpAny %bool %196
OpSelectionMerge %201 None
OpBranchConditional %195 %199 %200
%199 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%203 = OpLoad %v4float %202
OpStore %197 %203
OpBranch %201
%200 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%206 = OpLoad %v4float %204
OpStore %197 %206
OpBranch %201
%201 = OpLabel
%207 = OpLoad %v4float %197
OpReturnValue %207
OpFunctionEnd
