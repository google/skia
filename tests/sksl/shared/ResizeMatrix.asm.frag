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
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_float Function
%24 = OpVariable %_ptr_Function_mat2v2float Function
%48 = OpVariable %_ptr_Function_mat2v2float Function
%65 = OpVariable %_ptr_Function_mat3v3float Function
%85 = OpVariable %_ptr_Function_mat3v3float Function
%100 = OpVariable %_ptr_Function_mat4v4float Function
%125 = OpVariable %_ptr_Function_mat2v2float Function
%150 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%23 = OpLoad %float %result
%31 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%32 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%33 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%29 = OpCompositeConstruct %mat3v3float %31 %32 %33
%36 = OpCompositeExtract %v3float %29 0
%37 = OpVectorShuffle %v2float %36 %36 0 1
%38 = OpCompositeExtract %v3float %29 1
%39 = OpVectorShuffle %v2float %38 %38 0 1
%35 = OpCompositeConstruct %mat2v2float %37 %39
OpStore %24 %35
%42 = OpAccessChain %_ptr_Function_v2float %24 %int_0
%44 = OpLoad %v2float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpFAdd %float %23 %45
OpStore %result %46
%47 = OpLoad %float %result
%50 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%52 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%53 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%49 = OpCompositeConstruct %mat4v4float %50 %51 %52 %53
%56 = OpCompositeExtract %v4float %49 0
%57 = OpVectorShuffle %v2float %56 %56 0 1
%58 = OpCompositeExtract %v4float %49 1
%59 = OpVectorShuffle %v2float %58 %58 0 1
%55 = OpCompositeConstruct %mat2v2float %57 %59
OpStore %48 %55
%60 = OpAccessChain %_ptr_Function_v2float %48 %int_0
%61 = OpLoad %v2float %60
%62 = OpCompositeExtract %float %61 0
%63 = OpFAdd %float %47 %62
OpStore %result %63
%64 = OpLoad %float %result
%68 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%71 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%67 = OpCompositeConstruct %mat4v4float %68 %69 %70 %71
%73 = OpCompositeExtract %v4float %67 0
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%75 = OpCompositeExtract %v4float %67 1
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%77 = OpCompositeExtract %v4float %67 2
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%72 = OpCompositeConstruct %mat3v3float %74 %76 %78
OpStore %65 %72
%79 = OpAccessChain %_ptr_Function_v3float %65 %int_0
%81 = OpLoad %v3float %79
%82 = OpCompositeExtract %float %81 0
%83 = OpFAdd %float %64 %82
OpStore %result %83
%84 = OpLoad %float %result
%87 = OpCompositeConstruct %v2float %float_1 %float_0
%88 = OpCompositeConstruct %v2float %float_0 %float_1
%86 = OpCompositeConstruct %mat2v2float %87 %88
%90 = OpCompositeExtract %v2float %86 0
%91 = OpCompositeConstruct %v3float %90 %float_0
%92 = OpCompositeExtract %v2float %86 1
%93 = OpCompositeConstruct %v3float %92 %float_0
%94 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%89 = OpCompositeConstruct %mat3v3float %91 %93 %94
OpStore %85 %89
%95 = OpAccessChain %_ptr_Function_v3float %85 %int_0
%96 = OpLoad %v3float %95
%97 = OpCompositeExtract %float %96 0
%98 = OpFAdd %float %84 %97
OpStore %result %98
%99 = OpLoad %float %result
%103 = OpCompositeConstruct %v2float %float_1 %float_0
%104 = OpCompositeConstruct %v2float %float_0 %float_1
%102 = OpCompositeConstruct %mat2v2float %103 %104
%106 = OpCompositeExtract %v2float %102 0
%107 = OpCompositeConstruct %v3float %106 %float_0
%108 = OpCompositeExtract %v2float %102 1
%109 = OpCompositeConstruct %v3float %108 %float_0
%110 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%105 = OpCompositeConstruct %mat3v3float %107 %109 %110
%112 = OpCompositeExtract %v3float %105 0
%113 = OpCompositeConstruct %v4float %112 %float_0
%114 = OpCompositeExtract %v3float %105 1
%115 = OpCompositeConstruct %v4float %114 %float_0
%116 = OpCompositeExtract %v3float %105 2
%117 = OpCompositeConstruct %v4float %116 %float_0
%118 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%111 = OpCompositeConstruct %mat4v4float %113 %115 %117 %118
OpStore %100 %111
%119 = OpAccessChain %_ptr_Function_v4float %100 %int_0
%121 = OpLoad %v4float %119
%122 = OpCompositeExtract %float %121 0
%123 = OpFAdd %float %99 %122
OpStore %result %123
%124 = OpLoad %float %result
%127 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%130 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%126 = OpCompositeConstruct %mat4v4float %127 %128 %129 %130
%132 = OpCompositeExtract %v4float %126 0
%133 = OpVectorShuffle %v3float %132 %132 0 1 2
%134 = OpCompositeExtract %v4float %126 1
%135 = OpVectorShuffle %v3float %134 %134 0 1 2
%136 = OpCompositeExtract %v4float %126 2
%137 = OpVectorShuffle %v3float %136 %136 0 1 2
%131 = OpCompositeConstruct %mat3v3float %133 %135 %137
%139 = OpCompositeExtract %v3float %131 0
%140 = OpVectorShuffle %v2float %139 %139 0 1
%141 = OpCompositeExtract %v3float %131 1
%142 = OpVectorShuffle %v2float %141 %141 0 1
%138 = OpCompositeConstruct %mat2v2float %140 %142
OpStore %125 %138
%143 = OpAccessChain %_ptr_Function_v2float %125 %int_0
%144 = OpLoad %v2float %143
%145 = OpCompositeExtract %float %144 0
%146 = OpFAdd %float %124 %145
OpStore %result %146
%147 = OpLoad %float %result
%149 = OpFOrdEqual %bool %147 %float_6
OpSelectionMerge %153 None
OpBranchConditional %149 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%156 = OpLoad %v4float %154
OpStore %150 %156
OpBranch %153
%152 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %157
OpStore %150 %159
OpBranch %153
%153 = OpLabel
%160 = OpLoad %v4float %150
OpReturnValue %160
OpFunctionEnd
