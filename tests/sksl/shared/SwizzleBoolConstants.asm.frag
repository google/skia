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
OpDecorate %67 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
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
%171 = OpVariable %_ptr_Function_v4float Function
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
%66 = OpCompositeExtract %bool %65 0
%67 = OpLoad %v4bool %v
%68 = OpCompositeExtract %bool %67 2
%69 = OpCompositeConstruct %v3bool %66 %false %68
%70 = OpCompositeExtract %bool %69 0
%71 = OpCompositeExtract %bool %69 1
%72 = OpCompositeExtract %bool %69 2
%73 = OpCompositeConstruct %v4bool %70 %71 %72 %true
OpStore %result %73
%74 = OpLoad %v4bool %v
%75 = OpCompositeExtract %bool %74 0
%76 = OpCompositeConstruct %v4bool %75 %true %false %false
OpStore %result %76
%77 = OpLoad %v4bool %v
%78 = OpVectorShuffle %v2bool %77 %77 1 2
%79 = OpCompositeExtract %bool %78 0
%80 = OpCompositeExtract %bool %78 1
%81 = OpCompositeConstruct %v3bool %true %79 %80
%82 = OpCompositeExtract %bool %81 0
%83 = OpCompositeExtract %bool %81 1
%84 = OpCompositeExtract %bool %81 2
%85 = OpCompositeConstruct %v4bool %82 %83 %84 %false
OpStore %result %85
%86 = OpLoad %v4bool %v
%87 = OpCompositeExtract %bool %86 1
%88 = OpCompositeConstruct %v3bool %false %87 %true
%89 = OpCompositeExtract %bool %88 0
%90 = OpCompositeExtract %bool %88 1
%91 = OpCompositeExtract %bool %88 2
%92 = OpCompositeConstruct %v4bool %89 %90 %91 %false
OpStore %result %92
%93 = OpLoad %v4bool %v
%94 = OpCompositeExtract %bool %93 2
%95 = OpCompositeConstruct %v3bool %true %true %94
%96 = OpCompositeExtract %bool %95 0
%97 = OpCompositeExtract %bool %95 1
%98 = OpCompositeExtract %bool %95 2
%99 = OpCompositeConstruct %v4bool %96 %97 %98 %false
OpStore %result %99
%100 = OpLoad %v4bool %v
OpStore %result %100
%101 = OpLoad %v4bool %v
%102 = OpVectorShuffle %v3bool %101 %101 0 1 2
%103 = OpCompositeExtract %bool %102 0
%104 = OpCompositeExtract %bool %102 1
%105 = OpCompositeExtract %bool %102 2
%106 = OpCompositeConstruct %v4bool %103 %104 %105 %true
OpStore %result %106
%107 = OpLoad %v4bool %v
%108 = OpVectorShuffle %v2bool %107 %107 0 1
%109 = OpCompositeExtract %bool %108 0
%110 = OpCompositeExtract %bool %108 1
%111 = OpLoad %v4bool %v
%112 = OpCompositeExtract %bool %111 3
%113 = OpCompositeConstruct %v4bool %109 %110 %false %112
OpStore %result %113
%114 = OpLoad %v4bool %v
%115 = OpVectorShuffle %v2bool %114 %114 0 1
%116 = OpCompositeExtract %bool %115 0
%117 = OpCompositeExtract %bool %115 1
%118 = OpCompositeConstruct %v4bool %116 %117 %true %false
OpStore %result %118
%119 = OpLoad %v4bool %v
%120 = OpCompositeExtract %bool %119 0
%121 = OpLoad %v4bool %v
%122 = OpVectorShuffle %v2bool %121 %121 2 3
%123 = OpCompositeExtract %bool %122 0
%124 = OpCompositeExtract %bool %122 1
%125 = OpCompositeConstruct %v4bool %120 %true %123 %124
OpStore %result %125
%126 = OpLoad %v4bool %v
%127 = OpCompositeExtract %bool %126 0
%128 = OpLoad %v4bool %v
%129 = OpCompositeExtract %bool %128 2
%130 = OpCompositeConstruct %v4bool %127 %false %129 %true
OpStore %result %130
%131 = OpLoad %v4bool %v
%132 = OpCompositeExtract %bool %131 0
%133 = OpLoad %v4bool %v
%134 = OpCompositeExtract %bool %133 3
%135 = OpCompositeConstruct %v4bool %132 %true %true %134
OpStore %result %135
%136 = OpLoad %v4bool %v
%137 = OpCompositeExtract %bool %136 0
%138 = OpCompositeConstruct %v4bool %137 %true %false %true
OpStore %result %138
%139 = OpLoad %v4bool %v
%140 = OpVectorShuffle %v3bool %139 %139 1 2 3
%141 = OpCompositeExtract %bool %140 0
%142 = OpCompositeExtract %bool %140 1
%143 = OpCompositeExtract %bool %140 2
%144 = OpCompositeConstruct %v4bool %true %141 %142 %143
OpStore %result %144
%145 = OpLoad %v4bool %v
%146 = OpVectorShuffle %v2bool %145 %145 1 2
%147 = OpCompositeExtract %bool %146 0
%148 = OpCompositeExtract %bool %146 1
%149 = OpCompositeConstruct %v4bool %false %147 %148 %true
OpStore %result %149
%150 = OpLoad %v4bool %v
%151 = OpCompositeExtract %bool %150 1
%152 = OpLoad %v4bool %v
%153 = OpCompositeExtract %bool %152 3
%154 = OpCompositeConstruct %v4bool %false %151 %true %153
OpStore %result %154
%155 = OpLoad %v4bool %v
%156 = OpCompositeExtract %bool %155 1
%157 = OpCompositeConstruct %v4bool %true %156 %true %true
OpStore %result %157
%158 = OpLoad %v4bool %v
%159 = OpVectorShuffle %v2bool %158 %158 2 3
%160 = OpCompositeExtract %bool %159 0
%161 = OpCompositeExtract %bool %159 1
%162 = OpCompositeConstruct %v4bool %false %false %160 %161
OpStore %result %162
%163 = OpLoad %v4bool %v
%164 = OpCompositeExtract %bool %163 2
%165 = OpCompositeConstruct %v4bool %false %false %164 %true
OpStore %result %165
%166 = OpLoad %v4bool %v
%167 = OpCompositeExtract %bool %166 3
%168 = OpCompositeConstruct %v4bool %false %true %true %167
OpStore %result %168
%170 = OpLoad %v4bool %result
%169 = OpAny %bool %170
OpSelectionMerge %175 None
OpBranchConditional %169 %173 %174
%173 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%177 = OpLoad %v4float %176
OpStore %171 %177
OpBranch %175
%174 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%180 = OpLoad %v4float %178
OpStore %171 %180
OpBranch %175
%175 = OpLabel
%181 = OpLoad %v4float %171
OpReturnValue %181
OpFunctionEnd
