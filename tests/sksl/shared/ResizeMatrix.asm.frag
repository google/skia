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
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
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
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%150 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%30 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%31 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%32 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%28 = OpCompositeConstruct %mat3v3float %30 %31 %32
%35 = OpCompositeExtract %v3float %28 0
%36 = OpVectorShuffle %v2float %35 %35 0 1
%37 = OpCompositeExtract %v3float %28 1
%38 = OpVectorShuffle %v2float %37 %37 0 1
%34 = OpCompositeConstruct %mat2v2float %36 %38
OpStore %a %34
%39 = OpLoad %float %result
%42 = OpAccessChain %_ptr_Function_v2float %a %int_0
%44 = OpLoad %v2float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpFAdd %float %39 %45
OpStore %result %46
%49 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%50 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%52 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%48 = OpCompositeConstruct %mat4v4float %49 %50 %51 %52
%55 = OpCompositeExtract %v4float %48 0
%56 = OpVectorShuffle %v2float %55 %55 0 1
%57 = OpCompositeExtract %v4float %48 1
%58 = OpVectorShuffle %v2float %57 %57 0 1
%54 = OpCompositeConstruct %mat2v2float %56 %58
OpStore %b %54
%59 = OpLoad %float %result
%60 = OpAccessChain %_ptr_Function_v2float %b %int_0
%61 = OpLoad %v2float %60
%62 = OpCompositeExtract %float %61 0
%63 = OpFAdd %float %59 %62
OpStore %result %63
%67 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%68 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%66 = OpCompositeConstruct %mat4v4float %67 %68 %69 %70
%72 = OpCompositeExtract %v4float %66 0
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%74 = OpCompositeExtract %v4float %66 1
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%76 = OpCompositeExtract %v4float %66 2
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%71 = OpCompositeConstruct %mat3v3float %73 %75 %77
OpStore %c %71
%78 = OpLoad %float %result
%79 = OpAccessChain %_ptr_Function_v3float %c %int_0
%81 = OpLoad %v3float %79
%82 = OpCompositeExtract %float %81 0
%83 = OpFAdd %float %78 %82
OpStore %result %83
%86 = OpCompositeConstruct %v2float %float_1 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_1
%85 = OpCompositeConstruct %mat2v2float %86 %87
%89 = OpCompositeExtract %v2float %85 0
%90 = OpCompositeConstruct %v3float %89 %float_0
%91 = OpCompositeExtract %v2float %85 1
%92 = OpCompositeConstruct %v3float %91 %float_0
%93 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%88 = OpCompositeConstruct %mat3v3float %90 %92 %93
OpStore %d %88
%94 = OpLoad %float %result
%95 = OpAccessChain %_ptr_Function_v3float %d %int_0
%96 = OpLoad %v3float %95
%97 = OpCompositeExtract %float %96 0
%98 = OpFAdd %float %94 %97
OpStore %result %98
%102 = OpCompositeConstruct %v2float %float_1 %float_0
%103 = OpCompositeConstruct %v2float %float_0 %float_1
%101 = OpCompositeConstruct %mat2v2float %102 %103
%105 = OpCompositeExtract %v2float %101 0
%106 = OpCompositeConstruct %v3float %105 %float_0
%107 = OpCompositeExtract %v2float %101 1
%108 = OpCompositeConstruct %v3float %107 %float_0
%109 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%104 = OpCompositeConstruct %mat3v3float %106 %108 %109
%111 = OpCompositeExtract %v3float %104 0
%112 = OpCompositeConstruct %v4float %111 %float_0
%113 = OpCompositeExtract %v3float %104 1
%114 = OpCompositeConstruct %v4float %113 %float_0
%115 = OpCompositeExtract %v3float %104 2
%116 = OpCompositeConstruct %v4float %115 %float_0
%117 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%110 = OpCompositeConstruct %mat4v4float %112 %114 %116 %117
OpStore %e %110
%118 = OpLoad %float %result
%119 = OpAccessChain %_ptr_Function_v4float %e %int_0
%121 = OpLoad %v4float %119
%122 = OpCompositeExtract %float %121 0
%123 = OpFAdd %float %118 %122
OpStore %result %123
%126 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%125 = OpCompositeConstruct %mat4v4float %126 %127 %128 %129
%131 = OpCompositeExtract %v4float %125 0
%132 = OpVectorShuffle %v3float %131 %131 0 1 2
%133 = OpCompositeExtract %v4float %125 1
%134 = OpVectorShuffle %v3float %133 %133 0 1 2
%135 = OpCompositeExtract %v4float %125 2
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%130 = OpCompositeConstruct %mat3v3float %132 %134 %136
%138 = OpCompositeExtract %v3float %130 0
%139 = OpVectorShuffle %v2float %138 %138 0 1
%140 = OpCompositeExtract %v3float %130 1
%141 = OpVectorShuffle %v2float %140 %140 0 1
%137 = OpCompositeConstruct %mat2v2float %139 %141
OpStore %f %137
%142 = OpLoad %float %result
%143 = OpAccessChain %_ptr_Function_v2float %f %int_0
%144 = OpLoad %v2float %143
%145 = OpCompositeExtract %float %144 0
%146 = OpFAdd %float %142 %145
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
