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
OpName %g "g"
OpName %h "h"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpName %l "l"
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
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%mat2v3float = OpTypeMatrix %v3float 2
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_float Function
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%155 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%29 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%30 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%28 = OpCompositeConstruct %mat2v3float %29 %30
%33 = OpCompositeExtract %v3float %28 0
%34 = OpCompositeExtract %v3float %28 1
%35 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%32 = OpCompositeConstruct %mat3v3float %33 %34 %35
OpStore %g %32
%36 = OpLoad %float %result
%39 = OpAccessChain %_ptr_Function_v3float %g %int_0
%41 = OpLoad %v3float %39
%42 = OpCompositeExtract %float %41 0
%43 = OpFAdd %float %36 %42
OpStore %result %43
%47 = OpCompositeConstruct %v2float %float_1 %float_0
%48 = OpCompositeConstruct %v2float %float_0 %float_1
%49 = OpCompositeConstruct %v2float %float_0 %float_0
%45 = OpCompositeConstruct %mat3v2float %47 %48 %49
%52 = OpCompositeExtract %v2float %45 0
%53 = OpCompositeConstruct %v3float %52 %float_0
%54 = OpCompositeExtract %v2float %45 1
%55 = OpCompositeConstruct %v3float %54 %float_0
%56 = OpCompositeExtract %v2float %45 2
%57 = OpCompositeConstruct %v3float %56 %float_1
%51 = OpCompositeConstruct %mat3v3float %53 %55 %57
OpStore %h %51
%58 = OpLoad %float %result
%59 = OpAccessChain %_ptr_Function_v3float %h %int_0
%60 = OpLoad %v3float %59
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %58 %61
OpStore %result %62
%67 = OpCompositeConstruct %v2float %float_1 %float_0
%68 = OpCompositeConstruct %v2float %float_0 %float_1
%69 = OpCompositeConstruct %v2float %float_0 %float_0
%70 = OpCompositeConstruct %v2float %float_0 %float_0
%66 = OpCompositeConstruct %mat4v2float %67 %68 %69 %70
%73 = OpCompositeExtract %v2float %66 0
%74 = OpCompositeConstruct %v3float %73 %float_0
%75 = OpCompositeExtract %v2float %66 1
%76 = OpCompositeConstruct %v3float %75 %float_0
%77 = OpCompositeExtract %v2float %66 2
%78 = OpCompositeConstruct %v3float %77 %float_1
%79 = OpCompositeExtract %v2float %66 3
%80 = OpCompositeConstruct %v3float %79 %float_0
%72 = OpCompositeConstruct %mat4v3float %74 %76 %78 %80
%83 = OpCompositeExtract %v3float %72 0
%84 = OpCompositeConstruct %v4float %83 %float_0
%85 = OpCompositeExtract %v3float %72 1
%86 = OpCompositeConstruct %v4float %85 %float_0
%87 = OpCompositeExtract %v3float %72 2
%88 = OpCompositeConstruct %v4float %87 %float_0
%89 = OpCompositeExtract %v3float %72 3
%90 = OpCompositeConstruct %v4float %89 %float_1
%82 = OpCompositeConstruct %mat4v4float %84 %86 %88 %90
OpStore %i %82
%91 = OpLoad %float %result
%92 = OpAccessChain %_ptr_Function_v4float %i %int_0
%94 = OpLoad %v4float %92
%95 = OpCompositeExtract %float %94 0
%96 = OpFAdd %float %91 %95
OpStore %result %96
%99 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%100 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%98 = OpCompositeConstruct %mat2v4float %99 %100
%103 = OpCompositeExtract %v4float %98 0
%104 = OpCompositeExtract %v4float %98 1
%105 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%102 = OpCompositeConstruct %mat3v4float %103 %104 %105
%108 = OpCompositeExtract %v4float %102 0
%109 = OpCompositeExtract %v4float %102 1
%110 = OpCompositeExtract %v4float %102 2
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%107 = OpCompositeConstruct %mat4v4float %108 %109 %110 %111
OpStore %j %107
%112 = OpLoad %float %result
%113 = OpAccessChain %_ptr_Function_v4float %j %int_0
%114 = OpLoad %v4float %113
%115 = OpCompositeExtract %float %114 0
%116 = OpFAdd %float %112 %115
OpStore %result %116
%120 = OpCompositeConstruct %v2float %float_1 %float_0
%121 = OpCompositeConstruct %v2float %float_0 %float_1
%122 = OpCompositeConstruct %v2float %float_0 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %float_0
%119 = OpCompositeConstruct %mat4v2float %120 %121 %122 %123
%125 = OpCompositeExtract %v2float %119 0
%126 = OpCompositeConstruct %v4float %125 %float_0 %float_0
%127 = OpCompositeExtract %v2float %119 1
%128 = OpCompositeConstruct %v4float %127 %float_0 %float_0
%124 = OpCompositeConstruct %mat2v4float %126 %128
OpStore %k %124
%129 = OpLoad %float %result
%130 = OpAccessChain %_ptr_Function_v4float %k %int_0
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 0
%133 = OpFAdd %float %129 %132
OpStore %result %133
%137 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%138 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%136 = OpCompositeConstruct %mat2v4float %137 %138
%140 = OpCompositeExtract %v4float %136 0
%141 = OpVectorShuffle %v2float %140 %140 0 1
%142 = OpCompositeExtract %v4float %136 1
%143 = OpVectorShuffle %v2float %142 %142 0 1
%144 = OpCompositeConstruct %v2float %float_0 %float_0
%145 = OpCompositeConstruct %v2float %float_0 %float_0
%139 = OpCompositeConstruct %mat4v2float %141 %143 %144 %145
OpStore %l %139
%146 = OpLoad %float %result
%147 = OpAccessChain %_ptr_Function_v2float %l %int_0
%149 = OpLoad %v2float %147
%150 = OpCompositeExtract %float %149 0
%151 = OpFAdd %float %146 %150
OpStore %result %151
%152 = OpLoad %float %result
%154 = OpFOrdEqual %bool %152 %float_6
OpSelectionMerge %158 None
OpBranchConditional %154 %156 %157
%156 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%161 = OpLoad %v4float %159
OpStore %155 %161
OpBranch %158
%157 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%164 = OpLoad %v4float %162
OpStore %155 %164
OpBranch %158
%158 = OpLabel
%165 = OpLoad %v4float %155
OpReturnValue %165
OpFunctionEnd
