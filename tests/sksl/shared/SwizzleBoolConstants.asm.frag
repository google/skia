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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%157 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 1
%35 = OpFUnordNotEqual %bool %34 %float_0
%36 = OpCompositeConstruct %v4bool %35 %35 %35 %35
OpStore %v %36
%38 = OpLoad %v4bool %v
%39 = OpCompositeExtract %bool %38 0
%41 = OpCompositeConstruct %v4bool %39 %true %true %true
OpStore %result %41
%42 = OpLoad %v4bool %v
%43 = OpVectorShuffle %v2bool %42 %42 0 1
%45 = OpCompositeExtract %bool %43 0
%46 = OpCompositeExtract %bool %43 1
%48 = OpCompositeConstruct %v4bool %45 %46 %false %true
OpStore %result %48
%49 = OpLoad %v4bool %v
%50 = OpCompositeExtract %bool %49 0
%51 = OpCompositeConstruct %v4bool %50 %true %true %false
OpStore %result %51
%52 = OpLoad %v4bool %v
%53 = OpCompositeExtract %bool %52 1
%54 = OpCompositeConstruct %v4bool %false %53 %true %true
OpStore %result %54
%55 = OpLoad %v4bool %v
%56 = OpVectorShuffle %v3bool %55 %55 0 1 2
%58 = OpCompositeExtract %bool %56 0
%59 = OpCompositeExtract %bool %56 1
%60 = OpCompositeExtract %bool %56 2
%61 = OpCompositeConstruct %v4bool %58 %59 %60 %true
OpStore %result %61
%62 = OpLoad %v4bool %v
%63 = OpVectorShuffle %v2bool %62 %62 0 1
%64 = OpCompositeExtract %bool %63 0
%65 = OpCompositeExtract %bool %63 1
%66 = OpCompositeConstruct %v4bool %64 %65 %true %true
OpStore %result %66
%67 = OpLoad %v4bool %v
%68 = OpCompositeExtract %bool %67 0
%69 = OpLoad %v4bool %v
%70 = OpCompositeExtract %bool %69 2
%71 = OpCompositeConstruct %v4bool %68 %false %70 %true
OpStore %result %71
%72 = OpLoad %v4bool %v
%73 = OpCompositeExtract %bool %72 0
%74 = OpCompositeConstruct %v4bool %73 %true %false %false
OpStore %result %74
%75 = OpLoad %v4bool %v
%76 = OpVectorShuffle %v2bool %75 %75 1 2
%77 = OpCompositeExtract %bool %76 0
%78 = OpCompositeExtract %bool %76 1
%79 = OpCompositeConstruct %v4bool %true %77 %78 %false
OpStore %result %79
%80 = OpLoad %v4bool %v
%81 = OpCompositeExtract %bool %80 1
%82 = OpCompositeConstruct %v4bool %false %81 %true %false
OpStore %result %82
%83 = OpLoad %v4bool %v
%84 = OpCompositeExtract %bool %83 2
%85 = OpCompositeConstruct %v4bool %true %true %84 %false
OpStore %result %85
%86 = OpLoad %v4bool %v
OpStore %result %86
%87 = OpLoad %v4bool %v
%88 = OpVectorShuffle %v3bool %87 %87 0 1 2
%89 = OpCompositeExtract %bool %88 0
%90 = OpCompositeExtract %bool %88 1
%91 = OpCompositeExtract %bool %88 2
%92 = OpCompositeConstruct %v4bool %89 %90 %91 %true
OpStore %result %92
%93 = OpLoad %v4bool %v
%94 = OpVectorShuffle %v2bool %93 %93 0 1
%95 = OpCompositeExtract %bool %94 0
%96 = OpCompositeExtract %bool %94 1
%97 = OpLoad %v4bool %v
%98 = OpCompositeExtract %bool %97 3
%99 = OpCompositeConstruct %v4bool %95 %96 %false %98
OpStore %result %99
%100 = OpLoad %v4bool %v
%101 = OpVectorShuffle %v2bool %100 %100 0 1
%102 = OpCompositeExtract %bool %101 0
%103 = OpCompositeExtract %bool %101 1
%104 = OpCompositeConstruct %v4bool %102 %103 %true %false
OpStore %result %104
%105 = OpLoad %v4bool %v
%106 = OpCompositeExtract %bool %105 0
%107 = OpLoad %v4bool %v
%108 = OpVectorShuffle %v2bool %107 %107 2 3
%109 = OpCompositeExtract %bool %108 0
%110 = OpCompositeExtract %bool %108 1
%111 = OpCompositeConstruct %v4bool %106 %true %109 %110
OpStore %result %111
%112 = OpLoad %v4bool %v
%113 = OpCompositeExtract %bool %112 0
%114 = OpLoad %v4bool %v
%115 = OpCompositeExtract %bool %114 2
%116 = OpCompositeConstruct %v4bool %113 %false %115 %true
OpStore %result %116
%117 = OpLoad %v4bool %v
%118 = OpCompositeExtract %bool %117 0
%119 = OpLoad %v4bool %v
%120 = OpCompositeExtract %bool %119 3
%121 = OpCompositeConstruct %v4bool %118 %true %true %120
OpStore %result %121
%122 = OpLoad %v4bool %v
%123 = OpCompositeExtract %bool %122 0
%124 = OpCompositeConstruct %v4bool %123 %true %false %true
OpStore %result %124
%125 = OpLoad %v4bool %v
%126 = OpVectorShuffle %v3bool %125 %125 1 2 3
%127 = OpCompositeExtract %bool %126 0
%128 = OpCompositeExtract %bool %126 1
%129 = OpCompositeExtract %bool %126 2
%130 = OpCompositeConstruct %v4bool %true %127 %128 %129
OpStore %result %130
%131 = OpLoad %v4bool %v
%132 = OpVectorShuffle %v2bool %131 %131 1 2
%133 = OpCompositeExtract %bool %132 0
%134 = OpCompositeExtract %bool %132 1
%135 = OpCompositeConstruct %v4bool %false %133 %134 %true
OpStore %result %135
%136 = OpLoad %v4bool %v
%137 = OpCompositeExtract %bool %136 1
%138 = OpLoad %v4bool %v
%139 = OpCompositeExtract %bool %138 3
%140 = OpCompositeConstruct %v4bool %false %137 %true %139
OpStore %result %140
%141 = OpLoad %v4bool %v
%142 = OpCompositeExtract %bool %141 1
%143 = OpCompositeConstruct %v4bool %true %142 %true %true
OpStore %result %143
%144 = OpLoad %v4bool %v
%145 = OpVectorShuffle %v2bool %144 %144 2 3
%146 = OpCompositeExtract %bool %145 0
%147 = OpCompositeExtract %bool %145 1
%148 = OpCompositeConstruct %v4bool %false %false %146 %147
OpStore %result %148
%149 = OpLoad %v4bool %v
%150 = OpCompositeExtract %bool %149 2
%151 = OpCompositeConstruct %v4bool %false %false %150 %true
OpStore %result %151
%152 = OpLoad %v4bool %v
%153 = OpCompositeExtract %bool %152 3
%154 = OpCompositeConstruct %v4bool %false %true %true %153
OpStore %result %154
%156 = OpLoad %v4bool %result
%155 = OpAny %bool %156
OpSelectionMerge %161 None
OpBranchConditional %155 %159 %160
%159 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%163 = OpLoad %v4float %162
OpStore %157 %163
OpBranch %161
%160 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%166 = OpLoad %v4float %164
OpStore %157 %166
OpBranch %161
%161 = OpLabel
%167 = OpLoad %v4float %157
OpReturnValue %167
OpFunctionEnd
