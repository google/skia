OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %globalVar "globalVar"
OpName %S "S"
OpMemberName %S 0 "f"
OpMemberName %S 1 "af"
OpMemberName %S 2 "h4"
OpMemberName %S 3 "ah4"
OpName %globalStruct "globalStruct"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %keepAlive_vh "keepAlive_vh"
OpName %keepAlive_vf "keepAlive_vf"
OpName %keepAlive_vi "keepAlive_vi"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah3x3 "ah3x3"
OpName %af4 "af4"
OpName %s "s"
OpName %l "l"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %globalVar RelaxedPrecision
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %22 Binding 0
OpDecorate %22 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%22 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%27 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%36 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Function_int = OpTypePointer Function %int
%42 = OpTypeFunction %void %_ptr_Function_int
%45 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%57 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
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
%71 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%72 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%73 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%74 = OpConstantComposite %mat3v3float %71 %72 %73
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%97 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
%105 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%109 = OpConstantComposite %v2float %float_5 %float_5
%113 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%126 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %27
%28 = OpLabel
%32 = OpVariable %_ptr_Function_v2float Function
OpStore %32 %31
%34 = OpFunctionCall %v4float %main %32
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
%keepAlive_vh = OpFunction %void None %36
%37 = OpFunctionParameter %_ptr_Function_float
%38 = OpLabel
OpReturn
OpFunctionEnd
%keepAlive_vf = OpFunction %void None %36
%39 = OpFunctionParameter %_ptr_Function_float
%40 = OpLabel
OpReturn
OpFunctionEnd
%keepAlive_vi = OpFunction %void None %42
%43 = OpFunctionParameter %_ptr_Function_int
%44 = OpLabel
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %45
%46 = OpFunctionParameter %_ptr_Function_v2float
%47 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah3x3 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
%131 = OpVariable %_ptr_Function_float Function
%138 = OpVariable %_ptr_Function_float Function
%142 = OpVariable %_ptr_Function_int Function
%147 = OpVariable %_ptr_Function_int Function
%152 = OpVariable %_ptr_Function_int Function
%158 = OpVariable %_ptr_Function_int Function
%163 = OpVariable %_ptr_Function_float Function
%168 = OpVariable %_ptr_Function_float Function
%172 = OpVariable %_ptr_Function_float Function
%178 = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %57
OpStore %f3x3 %74
%77 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %77 %float_0
%78 = OpLoad %v4float %x
%79 = OpVectorShuffle %v4float %78 %31 5 4 2 3
OpStore %x %79
%83 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %83 %int_0
%87 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %87 %57
%91 = OpAccessChain %_ptr_Function_mat3v3float %ah3x3 %int_0
OpStore %91 %74
%95 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%96 = OpAccessChain %_ptr_Function_float %95 %int_0
OpStore %96 %float_0
%98 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%99 = OpLoad %v4float %98
%100 = OpVectorShuffle %v4float %99 %97 6 4 7 5
OpStore %98 %100
%103 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %103 %float_0
%104 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %104 %float_0
%106 = OpAccessChain %_ptr_Function_v4float %s %int_2
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v4float %107 %105 5 6 4 3
OpStore %106 %108
%110 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v4float %111 %109 0 4 2 5
OpStore %110 %112
OpStore %globalVar %113
%114 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
OpStore %114 %float_0
OpStore %l %float_0
%117 = OpAccessChain %_ptr_Function_int %ai %int_0
%118 = OpLoad %int %117
%119 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%120 = OpLoad %v4int %119
%121 = OpCompositeExtract %int %120 0
%122 = OpIAdd %int %118 %121
OpStore %117 %122
%123 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %123 %float_1
%124 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %124 %float_2
%125 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %125 %97
%127 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %127 %126
%128 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%129 = OpAccessChain %_ptr_Function_float %128 %int_0
%130 = OpLoad %float %129
OpStore %131 %130
%132 = OpFunctionCall %void %keepAlive_vf %131
%133 = OpLoad %float %131
OpStore %129 %133
%134 = OpAccessChain %_ptr_Function_v3float %ah3x3 %int_0 %int_0
%136 = OpAccessChain %_ptr_Function_float %134 %int_0
%137 = OpLoad %float %136
OpStore %138 %137
%139 = OpFunctionCall %void %keepAlive_vh %138
%140 = OpLoad %float %138
OpStore %136 %140
%141 = OpLoad %int %i
OpStore %142 %141
%143 = OpFunctionCall %void %keepAlive_vi %142
%144 = OpLoad %int %142
OpStore %i %144
%145 = OpAccessChain %_ptr_Function_int %i4 %int_1
%146 = OpLoad %int %145
OpStore %147 %146
%148 = OpFunctionCall %void %keepAlive_vi %147
%149 = OpLoad %int %147
OpStore %145 %149
%150 = OpAccessChain %_ptr_Function_int %ai %int_0
%151 = OpLoad %int %150
OpStore %152 %151
%153 = OpFunctionCall %void %keepAlive_vi %152
%154 = OpLoad %int %152
OpStore %150 %154
%155 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%156 = OpAccessChain %_ptr_Function_int %155 %int_0
%157 = OpLoad %int %156
OpStore %158 %157
%159 = OpFunctionCall %void %keepAlive_vi %158
%160 = OpLoad %int %158
OpStore %156 %160
%161 = OpAccessChain %_ptr_Function_float %x %int_1
%162 = OpLoad %float %161
OpStore %163 %162
%164 = OpFunctionCall %void %keepAlive_vh %163
%165 = OpLoad %float %163
OpStore %161 %165
%166 = OpAccessChain %_ptr_Function_float %s %int_0
%167 = OpLoad %float %166
OpStore %168 %167
%169 = OpFunctionCall %void %keepAlive_vf %168
%170 = OpLoad %float %168
OpStore %166 %170
%171 = OpLoad %float %l
OpStore %172 %171
%173 = OpFunctionCall %void %keepAlive_vh %172
%174 = OpLoad %float %172
OpStore %l %174
%175 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%176 = OpAccessChain %_ptr_Function_float %175 %int_0
%177 = OpLoad %float %176
OpStore %178 %177
%179 = OpFunctionCall %void %keepAlive_vf %178
%180 = OpLoad %float %178
OpStore %176 %180
%181 = OpAccessChain %_ptr_Uniform_v4float %22 %int_0
%183 = OpLoad %v4float %181
OpReturnValue %183
OpFunctionEnd
