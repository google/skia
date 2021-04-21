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
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%mat2v3float = OpTypeMatrix %v3float 2
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
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
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%158 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%34 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%35 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%33 = OpCompositeConstruct %mat2v3float %34 %35
%38 = OpCompositeExtract %v3float %33 0
%39 = OpCompositeExtract %v3float %33 1
%40 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%37 = OpCompositeConstruct %mat3v3float %38 %39 %40
OpStore %g %37
%41 = OpLoad %float %result
%44 = OpAccessChain %_ptr_Function_v3float %g %int_0
%46 = OpLoad %v3float %44
%47 = OpCompositeExtract %float %46 0
%48 = OpFAdd %float %41 %47
OpStore %result %48
%51 = OpCompositeConstruct %v2float %float_1 %float_0
%52 = OpCompositeConstruct %v2float %float_0 %float_1
%53 = OpCompositeConstruct %v2float %float_0 %float_0
%50 = OpCompositeConstruct %mat3v2float %51 %52 %53
%56 = OpCompositeExtract %v2float %50 0
%57 = OpCompositeConstruct %v3float %56 %float_0
%58 = OpCompositeExtract %v2float %50 1
%59 = OpCompositeConstruct %v3float %58 %float_0
%60 = OpCompositeExtract %v2float %50 2
%61 = OpCompositeConstruct %v3float %60 %float_1
%55 = OpCompositeConstruct %mat3v3float %57 %59 %61
OpStore %h %55
%62 = OpLoad %float %result
%63 = OpAccessChain %_ptr_Function_v3float %h %int_0
%64 = OpLoad %v3float %63
%65 = OpCompositeExtract %float %64 0
%66 = OpFAdd %float %62 %65
OpStore %result %66
%71 = OpCompositeConstruct %v2float %float_1 %float_0
%72 = OpCompositeConstruct %v2float %float_0 %float_1
%73 = OpCompositeConstruct %v2float %float_0 %float_0
%74 = OpCompositeConstruct %v2float %float_0 %float_0
%70 = OpCompositeConstruct %mat4v2float %71 %72 %73 %74
%77 = OpCompositeExtract %v2float %70 0
%78 = OpCompositeConstruct %v3float %77 %float_0
%79 = OpCompositeExtract %v2float %70 1
%80 = OpCompositeConstruct %v3float %79 %float_0
%81 = OpCompositeExtract %v2float %70 2
%82 = OpCompositeConstruct %v3float %81 %float_1
%83 = OpCompositeExtract %v2float %70 3
%84 = OpCompositeConstruct %v3float %83 %float_0
%76 = OpCompositeConstruct %mat4v3float %78 %80 %82 %84
%87 = OpCompositeExtract %v3float %76 0
%88 = OpCompositeConstruct %v4float %87 %float_0
%89 = OpCompositeExtract %v3float %76 1
%90 = OpCompositeConstruct %v4float %89 %float_0
%91 = OpCompositeExtract %v3float %76 2
%92 = OpCompositeConstruct %v4float %91 %float_0
%93 = OpCompositeExtract %v3float %76 3
%94 = OpCompositeConstruct %v4float %93 %float_1
%86 = OpCompositeConstruct %mat4v4float %88 %90 %92 %94
OpStore %i %86
%95 = OpLoad %float %result
%96 = OpAccessChain %_ptr_Function_v4float %i %int_0
%98 = OpLoad %v4float %96
%99 = OpCompositeExtract %float %98 0
%100 = OpFAdd %float %95 %99
OpStore %result %100
%103 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%104 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%102 = OpCompositeConstruct %mat2v4float %103 %104
%107 = OpCompositeExtract %v4float %102 0
%108 = OpCompositeExtract %v4float %102 1
%109 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%106 = OpCompositeConstruct %mat3v4float %107 %108 %109
%112 = OpCompositeExtract %v4float %106 0
%113 = OpCompositeExtract %v4float %106 1
%114 = OpCompositeExtract %v4float %106 2
%115 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%111 = OpCompositeConstruct %mat4v4float %112 %113 %114 %115
OpStore %j %111
%116 = OpLoad %float %result
%117 = OpAccessChain %_ptr_Function_v4float %j %int_0
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpFAdd %float %116 %119
OpStore %result %120
%124 = OpCompositeConstruct %v2float %float_1 %float_0
%125 = OpCompositeConstruct %v2float %float_0 %float_1
%126 = OpCompositeConstruct %v2float %float_0 %float_0
%127 = OpCompositeConstruct %v2float %float_0 %float_0
%123 = OpCompositeConstruct %mat4v2float %124 %125 %126 %127
%129 = OpCompositeExtract %v2float %123 0
%130 = OpCompositeConstruct %v4float %129 %float_0 %float_0
%131 = OpCompositeExtract %v2float %123 1
%132 = OpCompositeConstruct %v4float %131 %float_0 %float_0
%128 = OpCompositeConstruct %mat2v4float %130 %132
OpStore %k %128
%133 = OpLoad %float %result
%134 = OpAccessChain %_ptr_Function_v4float %k %int_0
%135 = OpLoad %v4float %134
%136 = OpCompositeExtract %float %135 0
%137 = OpFAdd %float %133 %136
OpStore %result %137
%141 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%142 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%140 = OpCompositeConstruct %mat2v4float %141 %142
%144 = OpCompositeExtract %v4float %140 0
%145 = OpVectorShuffle %v2float %144 %144 0 1
%146 = OpCompositeExtract %v4float %140 1
%147 = OpVectorShuffle %v2float %146 %146 0 1
%148 = OpCompositeConstruct %v2float %float_0 %float_0
%149 = OpCompositeConstruct %v2float %float_0 %float_0
%143 = OpCompositeConstruct %mat4v2float %145 %147 %148 %149
OpStore %l %143
%150 = OpLoad %float %result
%151 = OpAccessChain %_ptr_Function_v2float %l %int_0
%152 = OpLoad %v2float %151
%153 = OpCompositeExtract %float %152 0
%154 = OpFAdd %float %150 %153
OpStore %result %154
%155 = OpLoad %float %result
%157 = OpFOrdEqual %bool %155 %float_6
OpSelectionMerge %161 None
OpBranchConditional %157 %159 %160
%159 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%164 = OpLoad %v4float %162
OpStore %158 %164
OpBranch %161
%160 = OpLabel
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%167 = OpLoad %v4float %165
OpStore %158 %167
OpBranch %161
%161 = OpLabel
%168 = OpLoad %v4float %158
OpReturnValue %168
OpFunctionEnd
