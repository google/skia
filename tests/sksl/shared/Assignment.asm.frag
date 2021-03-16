OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %globalVar "globalVar"
OpName %S "S"
OpMemberName %S 0 "f"
OpMemberName %S 1 "af"
OpMemberName %S 2 "h4"
OpMemberName %S 3 "ah4"
OpName %globalStruct "globalStruct"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
OpName %s "s"
OpName %l "l"
OpName %r "r"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %globalVar RelaxedPrecision
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpDecorate %globalStruct RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %65 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %98 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_v4float = OpTypePointer Private %v4float
%globalVar = OpVariable %_ptr_Private_v4float Private
%int = OpTypeInt 32 1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Private_S = OpTypePointer Private %S
%globalStruct = OpVariable %_ptr_Private_S Private
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%27 = OpTypeFunction %v4float
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%39 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%v2float = OpTypeVector %float 2
%63 = OpConstantComposite %v2float %float_0 %float_0
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%87 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
%95 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%99 = OpConstantComposite %v2float %float_5 %float_5
%103 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%117 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %24
%25 = OpLabel
%26 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %27
%28 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %39
%54 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%55 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%56 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%53 = OpCompositeConstruct %mat3v3float %54 %55 %56
OpStore %f3x3 %53
%60 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %60 %float_0
%64 = OpLoad %v4float %x
%65 = OpVectorShuffle %v4float %64 %63 5 4 2 3
OpStore %x %65
%69 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %69 %int_0
%73 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %73 %39
%78 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%79 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%80 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%77 = OpCompositeConstruct %mat3v3float %78 %79 %80
%81 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %81 %77
%85 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%86 = OpAccessChain %_ptr_Function_float %85 %int_0
OpStore %86 %float_0
%88 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v4float %89 %87 6 4 7 5
OpStore %88 %90
%93 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %93 %float_0
%94 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %94 %float_0
%96 = OpAccessChain %_ptr_Function_v4float %s %int_2
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v4float %97 %95 5 6 4 3
OpStore %96 %98
%100 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v4float %101 %99 0 4 2 5
OpStore %100 %102
OpStore %globalVar %103
%104 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
OpStore %104 %float_0
OpStore %l %float_0
%108 = OpAccessChain %_ptr_Function_int %ai %int_0
%109 = OpLoad %int %108
%110 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%111 = OpLoad %v4int %110
%112 = OpCompositeExtract %int %111 0
%113 = OpIAdd %int %109 %112
OpStore %108 %113
%114 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %114 %float_1
%115 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %115 %float_2
%116 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %116 %87
%118 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %118 %117
%119 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%120 = OpLoad %v4float %119
%121 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%123 = OpLoad %v3float %121
%124 = OpCompositeExtract %float %123 0
%125 = OpVectorTimesScalar %v4float %120 %124
OpStore %119 %125
%126 = OpAccessChain %_ptr_Function_int %i4 %int_1
%127 = OpLoad %int %126
%128 = OpLoad %int %i
%129 = OpIMul %int %127 %128
OpStore %126 %129
%130 = OpAccessChain %_ptr_Function_float %x %int_1
%131 = OpLoad %float %130
%132 = OpLoad %float %l
%133 = OpFMul %float %131 %132
OpStore %130 %133
%134 = OpAccessChain %_ptr_Function_float %s %int_0
%135 = OpLoad %float %134
%136 = OpLoad %float %l
%137 = OpFMul %float %135 %136
OpStore %134 %137
%138 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%140 = OpLoad %v4float %138
OpReturnValue %140
OpFunctionEnd
