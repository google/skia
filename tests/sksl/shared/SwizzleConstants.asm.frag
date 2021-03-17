OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%144 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%148 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %v %26
%27 = OpLoad %v4float %v
%28 = OpCompositeExtract %float %27 0
%30 = OpCompositeConstruct %v4float %28 %float_1 %float_1 %float_1
OpStore %v %30
%31 = OpLoad %v4float %v
%32 = OpVectorShuffle %v2float %31 %31 0 1
%34 = OpCompositeExtract %float %32 0
%35 = OpCompositeExtract %float %32 1
%36 = OpCompositeConstruct %v4float %34 %35 %float_1 %float_1
OpStore %v %36
%37 = OpLoad %v4float %v
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeConstruct %v4float %38 %float_1 %float_1 %float_1
OpStore %v %39
%41 = OpLoad %v4float %v
%42 = OpCompositeExtract %float %41 1
%43 = OpCompositeConstruct %v4float %float_0 %42 %float_1 %float_1
OpStore %v %43
%44 = OpLoad %v4float %v
%45 = OpVectorShuffle %v3float %44 %44 0 1 2
%47 = OpCompositeExtract %float %45 0
%48 = OpCompositeExtract %float %45 1
%49 = OpCompositeExtract %float %45 2
%50 = OpCompositeConstruct %v4float %47 %48 %49 %float_1
OpStore %v %50
%51 = OpLoad %v4float %v
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpCompositeExtract %float %52 0
%54 = OpCompositeExtract %float %52 1
%55 = OpCompositeConstruct %v4float %53 %54 %float_1 %float_1
OpStore %v %55
%56 = OpLoad %v4float %v
%57 = OpCompositeExtract %float %56 0
%58 = OpLoad %v4float %v
%59 = OpCompositeExtract %float %58 2
%60 = OpCompositeConstruct %v4float %57 %float_0 %59 %float_1
OpStore %v %60
%61 = OpLoad %v4float %v
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeConstruct %v4float %62 %float_1 %float_0 %float_1
OpStore %v %63
%64 = OpLoad %v4float %v
%65 = OpVectorShuffle %v2float %64 %64 1 2
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeConstruct %v4float %float_1 %66 %67 %float_1
OpStore %v %68
%69 = OpLoad %v4float %v
%70 = OpCompositeExtract %float %69 1
%71 = OpCompositeConstruct %v4float %float_0 %70 %float_1 %float_1
OpStore %v %71
%72 = OpLoad %v4float %v
%73 = OpCompositeExtract %float %72 2
%74 = OpCompositeConstruct %v4float %float_1 %float_1 %73 %float_1
OpStore %v %74
%75 = OpLoad %v4float %v
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeExtract %float %76 1
%79 = OpCompositeExtract %float %76 2
%80 = OpCompositeConstruct %v4float %77 %78 %79 %float_1
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpVectorShuffle %v2float %81 %81 0 1
%83 = OpCompositeExtract %float %82 0
%84 = OpCompositeExtract %float %82 1
%85 = OpLoad %v4float %v
%86 = OpCompositeExtract %float %85 3
%87 = OpCompositeConstruct %v4float %83 %84 %float_0 %86
OpStore %v %87
%88 = OpLoad %v4float %v
%89 = OpVectorShuffle %v2float %88 %88 0 1
%90 = OpCompositeExtract %float %89 0
%91 = OpCompositeExtract %float %89 1
%92 = OpCompositeConstruct %v4float %90 %91 %float_1 %float_0
OpStore %v %92
%93 = OpLoad %v4float %v
%94 = OpCompositeExtract %float %93 0
%95 = OpLoad %v4float %v
%96 = OpVectorShuffle %v2float %95 %95 2 3
%97 = OpCompositeExtract %float %96 0
%98 = OpCompositeExtract %float %96 1
%99 = OpCompositeConstruct %v4float %94 %float_1 %97 %98
OpStore %v %99
%100 = OpLoad %v4float %v
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v4float %v
%103 = OpCompositeExtract %float %102 2
%104 = OpCompositeConstruct %v4float %101 %float_0 %103 %float_1
OpStore %v %104
%105 = OpLoad %v4float %v
%106 = OpCompositeExtract %float %105 0
%107 = OpLoad %v4float %v
%108 = OpCompositeExtract %float %107 3
%109 = OpCompositeConstruct %v4float %106 %float_1 %float_1 %108
OpStore %v %109
%110 = OpLoad %v4float %v
%111 = OpCompositeExtract %float %110 0
%112 = OpCompositeConstruct %v4float %111 %float_1 %float_0 %float_1
OpStore %v %112
%113 = OpLoad %v4float %v
%114 = OpVectorShuffle %v3float %113 %113 1 2 3
%115 = OpCompositeExtract %float %114 0
%116 = OpCompositeExtract %float %114 1
%117 = OpCompositeExtract %float %114 2
%118 = OpCompositeConstruct %v4float %float_1 %115 %116 %117
OpStore %v %118
%119 = OpLoad %v4float %v
%120 = OpVectorShuffle %v2float %119 %119 1 2
%121 = OpCompositeExtract %float %120 0
%122 = OpCompositeExtract %float %120 1
%123 = OpCompositeConstruct %v4float %float_0 %121 %122 %float_1
OpStore %v %123
%124 = OpLoad %v4float %v
%125 = OpCompositeExtract %float %124 1
%126 = OpLoad %v4float %v
%127 = OpCompositeExtract %float %126 3
%128 = OpCompositeConstruct %v4float %float_0 %125 %float_1 %127
OpStore %v %128
%129 = OpLoad %v4float %v
%130 = OpCompositeExtract %float %129 1
%131 = OpCompositeConstruct %v4float %float_1 %130 %float_1 %float_1
OpStore %v %131
%132 = OpLoad %v4float %v
%133 = OpVectorShuffle %v2float %132 %132 2 3
%134 = OpCompositeExtract %float %133 0
%135 = OpCompositeExtract %float %133 1
%136 = OpCompositeConstruct %v4float %float_0 %float_0 %134 %135
OpStore %v %136
%137 = OpLoad %v4float %v
%138 = OpCompositeExtract %float %137 2
%139 = OpCompositeConstruct %v4float %float_0 %float_0 %138 %float_1
OpStore %v %139
%140 = OpLoad %v4float %v
%141 = OpCompositeExtract %float %140 3
%142 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %141
OpStore %v %142
%143 = OpLoad %v4float %v
%145 = OpFOrdEqual %v4bool %143 %144
%147 = OpAll %bool %145
OpSelectionMerge %151 None
OpBranchConditional %147 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %152
OpStore %148 %154
OpBranch %151
%150 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%157 = OpLoad %v4float %155
OpStore %148 %157
OpBranch %151
%151 = OpLabel
%158 = OpLoad %v4float %148
OpReturnValue %158
OpFunctionEnd
