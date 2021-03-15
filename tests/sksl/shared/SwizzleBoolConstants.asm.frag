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
OpDecorate %50 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
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
%152 = OpVariable %_ptr_Function_v4float Function
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
%49 = OpCompositeConstruct %v4bool %false %48 %true %true
OpStore %result %49
%50 = OpLoad %v4bool %v
%51 = OpVectorShuffle %v3bool %50 %50 0 1 2
%53 = OpCompositeExtract %bool %51 0
%54 = OpCompositeExtract %bool %51 1
%55 = OpCompositeExtract %bool %51 2
%56 = OpCompositeConstruct %v4bool %53 %54 %55 %true
OpStore %result %56
%57 = OpLoad %v4bool %v
%58 = OpVectorShuffle %v2bool %57 %57 0 1
%59 = OpCompositeExtract %bool %58 0
%60 = OpCompositeExtract %bool %58 1
%61 = OpCompositeConstruct %v4bool %59 %60 %true %true
OpStore %result %61
%62 = OpLoad %v4bool %v
%63 = OpCompositeExtract %bool %62 0
%64 = OpLoad %v4bool %v
%65 = OpCompositeExtract %bool %64 2
%66 = OpCompositeConstruct %v4bool %63 %false %65 %true
OpStore %result %66
%67 = OpLoad %v4bool %v
%68 = OpCompositeExtract %bool %67 0
%69 = OpCompositeConstruct %v4bool %68 %true %false %false
OpStore %result %69
%70 = OpLoad %v4bool %v
%71 = OpVectorShuffle %v2bool %70 %70 1 2
%72 = OpCompositeExtract %bool %71 0
%73 = OpCompositeExtract %bool %71 1
%74 = OpCompositeConstruct %v4bool %true %72 %73 %false
OpStore %result %74
%75 = OpLoad %v4bool %v
%76 = OpCompositeExtract %bool %75 1
%77 = OpCompositeConstruct %v4bool %false %76 %true %false
OpStore %result %77
%78 = OpLoad %v4bool %v
%79 = OpCompositeExtract %bool %78 2
%80 = OpCompositeConstruct %v4bool %true %true %79 %false
OpStore %result %80
%81 = OpLoad %v4bool %v
OpStore %result %81
%82 = OpLoad %v4bool %v
%83 = OpVectorShuffle %v3bool %82 %82 0 1 2
%84 = OpCompositeExtract %bool %83 0
%85 = OpCompositeExtract %bool %83 1
%86 = OpCompositeExtract %bool %83 2
%87 = OpCompositeConstruct %v4bool %84 %85 %86 %true
OpStore %result %87
%88 = OpLoad %v4bool %v
%89 = OpVectorShuffle %v2bool %88 %88 0 1
%90 = OpCompositeExtract %bool %89 0
%91 = OpCompositeExtract %bool %89 1
%92 = OpLoad %v4bool %v
%93 = OpCompositeExtract %bool %92 3
%94 = OpCompositeConstruct %v4bool %90 %91 %false %93
OpStore %result %94
%95 = OpLoad %v4bool %v
%96 = OpVectorShuffle %v2bool %95 %95 0 1
%97 = OpCompositeExtract %bool %96 0
%98 = OpCompositeExtract %bool %96 1
%99 = OpCompositeConstruct %v4bool %97 %98 %true %false
OpStore %result %99
%100 = OpLoad %v4bool %v
%101 = OpCompositeExtract %bool %100 0
%102 = OpLoad %v4bool %v
%103 = OpVectorShuffle %v2bool %102 %102 2 3
%104 = OpCompositeExtract %bool %103 0
%105 = OpCompositeExtract %bool %103 1
%106 = OpCompositeConstruct %v4bool %101 %true %104 %105
OpStore %result %106
%107 = OpLoad %v4bool %v
%108 = OpCompositeExtract %bool %107 0
%109 = OpLoad %v4bool %v
%110 = OpCompositeExtract %bool %109 2
%111 = OpCompositeConstruct %v4bool %108 %false %110 %true
OpStore %result %111
%112 = OpLoad %v4bool %v
%113 = OpCompositeExtract %bool %112 0
%114 = OpLoad %v4bool %v
%115 = OpCompositeExtract %bool %114 3
%116 = OpCompositeConstruct %v4bool %113 %true %true %115
OpStore %result %116
%117 = OpLoad %v4bool %v
%118 = OpCompositeExtract %bool %117 0
%119 = OpCompositeConstruct %v4bool %118 %true %false %true
OpStore %result %119
%120 = OpLoad %v4bool %v
%121 = OpVectorShuffle %v3bool %120 %120 1 2 3
%122 = OpCompositeExtract %bool %121 0
%123 = OpCompositeExtract %bool %121 1
%124 = OpCompositeExtract %bool %121 2
%125 = OpCompositeConstruct %v4bool %true %122 %123 %124
OpStore %result %125
%126 = OpLoad %v4bool %v
%127 = OpVectorShuffle %v2bool %126 %126 1 2
%128 = OpCompositeExtract %bool %127 0
%129 = OpCompositeExtract %bool %127 1
%130 = OpCompositeConstruct %v4bool %false %128 %129 %true
OpStore %result %130
%131 = OpLoad %v4bool %v
%132 = OpCompositeExtract %bool %131 1
%133 = OpLoad %v4bool %v
%134 = OpCompositeExtract %bool %133 3
%135 = OpCompositeConstruct %v4bool %false %132 %true %134
OpStore %result %135
%136 = OpLoad %v4bool %v
%137 = OpCompositeExtract %bool %136 1
%138 = OpCompositeConstruct %v4bool %true %137 %true %true
OpStore %result %138
%139 = OpLoad %v4bool %v
%140 = OpVectorShuffle %v2bool %139 %139 2 3
%141 = OpCompositeExtract %bool %140 0
%142 = OpCompositeExtract %bool %140 1
%143 = OpCompositeConstruct %v4bool %false %false %141 %142
OpStore %result %143
%144 = OpLoad %v4bool %v
%145 = OpCompositeExtract %bool %144 2
%146 = OpCompositeConstruct %v4bool %false %false %145 %true
OpStore %result %146
%147 = OpLoad %v4bool %v
%148 = OpCompositeExtract %bool %147 3
%149 = OpCompositeConstruct %v4bool %false %true %true %148
OpStore %result %149
%151 = OpLoad %v4bool %result
%150 = OpAny %bool %151
OpSelectionMerge %156 None
OpBranchConditional %150 %154 %155
%154 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%158 = OpLoad %v4float %157
OpStore %152 %158
OpBranch %156
%155 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%161 = OpLoad %v4float %159
OpStore %152 %161
OpBranch %156
%156 = OpLabel
%162 = OpLoad %v4float %152
OpReturnValue %162
OpFunctionEnd
