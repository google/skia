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
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%result = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%153 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%34 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%35 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%32 = OpCompositeConstruct %mat3v3float %34 %35 %36
%39 = OpCompositeExtract %v3float %32 0
%40 = OpVectorShuffle %v2float %39 %39 0 1
%41 = OpCompositeExtract %v3float %32 1
%42 = OpVectorShuffle %v2float %41 %41 0 1
%38 = OpCompositeConstruct %mat2v2float %40 %42
OpStore %a %38
%43 = OpLoad %float %result
%46 = OpAccessChain %_ptr_Function_v2float %a %int_0
%47 = OpLoad %v2float %46
%48 = OpCompositeExtract %float %47 0
%49 = OpFAdd %float %43 %48
OpStore %result %49
%52 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%53 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%54 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%51 = OpCompositeConstruct %mat4v4float %52 %53 %54 %55
%58 = OpCompositeExtract %v4float %51 0
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpCompositeExtract %v4float %51 1
%61 = OpVectorShuffle %v2float %60 %60 0 1
%57 = OpCompositeConstruct %mat2v2float %59 %61
OpStore %b %57
%62 = OpLoad %float %result
%63 = OpAccessChain %_ptr_Function_v2float %b %int_0
%64 = OpLoad %v2float %63
%65 = OpCompositeExtract %float %64 0
%66 = OpFAdd %float %62 %65
OpStore %result %66
%70 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%71 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%69 = OpCompositeConstruct %mat4v4float %70 %71 %72 %73
%75 = OpCompositeExtract %v4float %69 0
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%77 = OpCompositeExtract %v4float %69 1
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpCompositeExtract %v4float %69 2
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%74 = OpCompositeConstruct %mat3v3float %76 %78 %80
OpStore %c %74
%81 = OpLoad %float %result
%82 = OpAccessChain %_ptr_Function_v3float %c %int_0
%84 = OpLoad %v3float %82
%85 = OpCompositeExtract %float %84 0
%86 = OpFAdd %float %81 %85
OpStore %result %86
%89 = OpCompositeConstruct %v2float %float_1 %float_0
%90 = OpCompositeConstruct %v2float %float_0 %float_1
%88 = OpCompositeConstruct %mat2v2float %89 %90
%92 = OpCompositeExtract %v2float %88 0
%93 = OpCompositeConstruct %v3float %92 %float_0
%94 = OpCompositeExtract %v2float %88 1
%95 = OpCompositeConstruct %v3float %94 %float_0
%96 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%91 = OpCompositeConstruct %mat3v3float %93 %95 %96
OpStore %d %91
%97 = OpLoad %float %result
%98 = OpAccessChain %_ptr_Function_v3float %d %int_0
%99 = OpLoad %v3float %98
%100 = OpCompositeExtract %float %99 0
%101 = OpFAdd %float %97 %100
OpStore %result %101
%105 = OpCompositeConstruct %v2float %float_1 %float_0
%106 = OpCompositeConstruct %v2float %float_0 %float_1
%104 = OpCompositeConstruct %mat2v2float %105 %106
%108 = OpCompositeExtract %v2float %104 0
%109 = OpCompositeConstruct %v3float %108 %float_0
%110 = OpCompositeExtract %v2float %104 1
%111 = OpCompositeConstruct %v3float %110 %float_0
%112 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%107 = OpCompositeConstruct %mat3v3float %109 %111 %112
%114 = OpCompositeExtract %v3float %107 0
%115 = OpCompositeConstruct %v4float %114 %float_0
%116 = OpCompositeExtract %v3float %107 1
%117 = OpCompositeConstruct %v4float %116 %float_0
%118 = OpCompositeExtract %v3float %107 2
%119 = OpCompositeConstruct %v4float %118 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%113 = OpCompositeConstruct %mat4v4float %115 %117 %119 %120
OpStore %e %113
%121 = OpLoad %float %result
%122 = OpAccessChain %_ptr_Function_v4float %e %int_0
%124 = OpLoad %v4float %122
%125 = OpCompositeExtract %float %124 0
%126 = OpFAdd %float %121 %125
OpStore %result %126
%129 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%130 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%131 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%132 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%128 = OpCompositeConstruct %mat4v4float %129 %130 %131 %132
%134 = OpCompositeExtract %v4float %128 0
%135 = OpVectorShuffle %v3float %134 %134 0 1 2
%136 = OpCompositeExtract %v4float %128 1
%137 = OpVectorShuffle %v3float %136 %136 0 1 2
%138 = OpCompositeExtract %v4float %128 2
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%133 = OpCompositeConstruct %mat3v3float %135 %137 %139
%141 = OpCompositeExtract %v3float %133 0
%142 = OpVectorShuffle %v2float %141 %141 0 1
%143 = OpCompositeExtract %v3float %133 1
%144 = OpVectorShuffle %v2float %143 %143 0 1
%140 = OpCompositeConstruct %mat2v2float %142 %144
OpStore %f %140
%145 = OpLoad %float %result
%146 = OpAccessChain %_ptr_Function_v2float %f %int_0
%147 = OpLoad %v2float %146
%148 = OpCompositeExtract %float %147 0
%149 = OpFAdd %float %145 %148
OpStore %result %149
%150 = OpLoad %float %result
%152 = OpFOrdEqual %bool %150 %float_6
OpSelectionMerge %156 None
OpBranchConditional %152 %154 %155
%154 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%159 = OpLoad %v4float %157
OpStore %153 %159
OpBranch %156
%155 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%162 = OpLoad %v4float %160
OpStore %153 %162
OpBranch %156
%156 = OpLabel
%163 = OpLoad %v4float %153
OpReturnValue %163
OpFunctionEnd
