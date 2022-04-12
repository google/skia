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
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
%36 = OpConstantComposite %mat2v3float %33 %34
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%48 = OpConstantComposite %v2float %float_1 %float_0
%49 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
%51 = OpConstantComposite %mat3v2float %48 %49 %19
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%65 = OpConstantComposite %mat4v2float %48 %49 %19 %19
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%88 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%89 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%mat2v4float = OpTypeMatrix %v4float 2
%91 = OpConstantComposite %mat2v4float %88 %89
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
%130 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%37 = OpCompositeConstruct %mat3v3float %33 %34 %38
OpStore %g %37
%39 = OpLoad %float %result
%42 = OpAccessChain %_ptr_Function_v3float %g %int_0
%44 = OpLoad %v3float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpFAdd %float %39 %45
OpStore %result %46
%53 = OpCompositeConstruct %v3float %48 %float_0
%54 = OpCompositeConstruct %v3float %49 %float_0
%55 = OpCompositeConstruct %v3float %19 %float_1
%52 = OpCompositeConstruct %mat3v3float %53 %54 %55
OpStore %h %52
%56 = OpLoad %float %result
%57 = OpAccessChain %_ptr_Function_v3float %h %int_0
%58 = OpLoad %v3float %57
%59 = OpCompositeExtract %float %58 0
%60 = OpFAdd %float %56 %59
OpStore %result %60
%67 = OpCompositeConstruct %v3float %48 %float_0
%68 = OpCompositeConstruct %v3float %49 %float_0
%69 = OpCompositeConstruct %v3float %19 %float_1
%70 = OpCompositeConstruct %v3float %19 %float_0
%66 = OpCompositeConstruct %mat4v3float %67 %68 %69 %70
%73 = OpCompositeExtract %v3float %66 0
%74 = OpCompositeConstruct %v4float %73 %float_0
%75 = OpCompositeExtract %v3float %66 1
%76 = OpCompositeConstruct %v4float %75 %float_0
%77 = OpCompositeExtract %v3float %66 2
%78 = OpCompositeConstruct %v4float %77 %float_0
%79 = OpCompositeExtract %v3float %66 3
%80 = OpCompositeConstruct %v4float %79 %float_1
%72 = OpCompositeConstruct %mat4v4float %74 %76 %78 %80
OpStore %i %72
%81 = OpLoad %float %result
%82 = OpAccessChain %_ptr_Function_v4float %i %int_0
%84 = OpLoad %v4float %82
%85 = OpCompositeExtract %float %84 0
%86 = OpFAdd %float %81 %85
OpStore %result %86
%93 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%92 = OpCompositeConstruct %mat3v4float %88 %89 %93
%96 = OpCompositeExtract %v4float %92 0
%97 = OpCompositeExtract %v4float %92 1
%98 = OpCompositeExtract %v4float %92 2
%99 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%95 = OpCompositeConstruct %mat4v4float %96 %97 %98 %99
OpStore %j %95
%100 = OpLoad %float %result
%101 = OpAccessChain %_ptr_Function_v4float %j %int_0
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 0
%104 = OpFAdd %float %100 %103
OpStore %result %104
%108 = OpCompositeConstruct %v4float %48 %float_0 %float_0
%109 = OpCompositeConstruct %v4float %49 %float_0 %float_0
%107 = OpCompositeConstruct %mat2v4float %108 %109
OpStore %k %107
%110 = OpLoad %float %result
%111 = OpAccessChain %_ptr_Function_v4float %k %int_0
%112 = OpLoad %v4float %111
%113 = OpCompositeExtract %float %112 0
%114 = OpFAdd %float %110 %113
OpStore %result %114
%118 = OpVectorShuffle %v2float %88 %88 0 1
%119 = OpVectorShuffle %v2float %89 %89 0 1
%120 = OpCompositeConstruct %v2float %float_0 %float_0
%121 = OpCompositeConstruct %v2float %float_0 %float_0
%117 = OpCompositeConstruct %mat4v2float %118 %119 %120 %121
OpStore %l %117
%122 = OpLoad %float %result
%123 = OpAccessChain %_ptr_Function_v2float %l %int_0
%124 = OpLoad %v2float %123
%125 = OpCompositeExtract %float %124 0
%126 = OpFAdd %float %122 %125
OpStore %result %126
%127 = OpLoad %float %result
%129 = OpFOrdEqual %bool %127 %float_6
OpSelectionMerge %133 None
OpBranchConditional %129 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%136 = OpLoad %v4float %134
OpStore %130 %136
OpBranch %133
%132 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%139 = OpLoad %v4float %137
OpStore %130 %139
OpBranch %133
%133 = OpLabel
%140 = OpLoad %v4float %130
OpReturnValue %140
OpFunctionEnd
