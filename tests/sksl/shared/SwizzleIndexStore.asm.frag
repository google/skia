OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix3x3"
OpMemberName %_UniformBuffer 3 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test3x3_b "test3x3_b"
OpName %expected "expected"
OpName %vec "vec"
OpName %c "c"
OpName %r "r"
OpName %test4x4_b "test4x4_b"
OpName %expected_0 "expected"
OpName %vec_0 "vec"
OpName %c_0 "c"
OpName %r_0 "r"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 80
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %bool
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_3 = OpConstant %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_2 = OpConstant %int 2
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%v3uint = OpTypeVector %uint 3
%71 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%88 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_4 = OpConstant %float 4
%97 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
%int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%uint_3 = OpConstant %uint 3
%v4uint = OpTypeVector %uint 4
%126 = OpConstantComposite %v4uint %uint_3 %uint_2 %uint_1 %uint_0
%v4bool = OpTypeVector %bool 4
%140 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%144 = OpTypeFunction %v4float %_ptr_Function_v2float
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%25 = OpVariable %_ptr_Function_v2float Function
OpStore %25 %24
%27 = OpFunctionCall %v4float %main %25
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%test3x3_b = OpFunction %bool None %28
%29 = OpLabel
%expected = OpVariable %_ptr_Function_v3float Function
%vec = OpVariable %_ptr_Function_v3float Function
%c = OpVariable %_ptr_Function_int Function
%r = OpVariable %_ptr_Function_int Function
OpStore %expected %35
OpStore %c %int_0
OpBranch %41
%41 = OpLabel
OpLoopMerge %45 %44 None
OpBranch %42
%42 = OpLabel
%46 = OpLoad %int %c
%48 = OpSLessThan %bool %46 %int_3
OpBranchConditional %48 %43 %45
%43 = OpLabel
OpStore %r %int_0
OpBranch %50
%50 = OpLabel
OpLoopMerge %54 %53 None
OpBranch %51
%51 = OpLabel
%55 = OpLoad %int %r
%56 = OpSLessThan %bool %55 %int_3
OpBranchConditional %56 %52 %54
%52 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
%60 = OpLoad %int %c
%61 = OpAccessChain %_ptr_Uniform_v3float %57 %60
%63 = OpLoad %v3float %61
%64 = OpLoad %int %r
%65 = OpVectorExtractDynamic %float %63 %64
%72 = OpLoad %int %r
%73 = OpVectorExtractDynamic %uint %71 %72
%74 = OpAccessChain %_ptr_Function_float %vec %73
OpStore %74 %65
OpBranch %53
%53 = OpLabel
%77 = OpLoad %int %r
%78 = OpIAdd %int %77 %int_1
OpStore %r %78
OpBranch %50
%54 = OpLabel
%79 = OpLoad %v3float %vec
%80 = OpLoad %v3float %expected
%81 = OpFUnordNotEqual %v3bool %79 %80
%83 = OpAny %bool %81
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
OpReturnValue %false
%85 = OpLabel
%87 = OpLoad %v3float %expected
%89 = OpFAdd %v3float %87 %88
OpStore %expected %89
OpBranch %44
%44 = OpLabel
%90 = OpLoad %int %c
%91 = OpIAdd %int %90 %int_1
OpStore %c %91
OpBranch %41
%45 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test4x4_b = OpFunction %bool None %28
%93 = OpLabel
%expected_0 = OpVariable %_ptr_Function_v4float Function
%vec_0 = OpVariable %_ptr_Function_v4float Function
%c_0 = OpVariable %_ptr_Function_int Function
%r_0 = OpVariable %_ptr_Function_int Function
OpStore %expected_0 %97
OpStore %c_0 %int_0
OpBranch %100
%100 = OpLabel
OpLoopMerge %104 %103 None
OpBranch %101
%101 = OpLabel
%105 = OpLoad %int %c_0
%107 = OpSLessThan %bool %105 %int_4
OpBranchConditional %107 %102 %104
%102 = OpLabel
OpStore %r_0 %int_0
OpBranch %109
%109 = OpLabel
OpLoopMerge %113 %112 None
OpBranch %110
%110 = OpLabel
%114 = OpLoad %int %r_0
%115 = OpSLessThan %bool %114 %int_4
OpBranchConditional %115 %111 %113
%111 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
%118 = OpLoad %int %c_0
%119 = OpAccessChain %_ptr_Uniform_v4float %116 %118
%121 = OpLoad %v4float %119
%122 = OpLoad %int %r_0
%123 = OpVectorExtractDynamic %float %121 %122
%127 = OpLoad %int %r_0
%128 = OpVectorExtractDynamic %uint %126 %127
%129 = OpAccessChain %_ptr_Function_float %vec_0 %128
OpStore %129 %123
OpBranch %112
%112 = OpLabel
%130 = OpLoad %int %r_0
%131 = OpIAdd %int %130 %int_1
OpStore %r_0 %131
OpBranch %109
%113 = OpLabel
%132 = OpLoad %v4float %vec_0
%133 = OpLoad %v4float %expected_0
%134 = OpFUnordNotEqual %v4bool %132 %133
%136 = OpAny %bool %134
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
OpReturnValue %false
%138 = OpLabel
%139 = OpLoad %v4float %expected_0
%141 = OpFAdd %v4float %139 %140
OpStore %expected_0 %141
OpBranch %103
%103 = OpLabel
%142 = OpLoad %int %c_0
%143 = OpIAdd %int %142 %int_1
OpStore %c_0 %143
OpBranch %100
%104 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %144
%145 = OpFunctionParameter %_ptr_Function_v2float
%146 = OpLabel
%152 = OpVariable %_ptr_Function_v4float Function
%147 = OpFunctionCall %bool %test3x3_b
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpFunctionCall %bool %test4x4_b
OpBranch %149
%149 = OpLabel
%151 = OpPhi %bool %false %146 %150 %148
OpSelectionMerge %155 None
OpBranchConditional %151 %153 %154
%153 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%157 = OpLoad %v4float %156
OpStore %152 %157
OpBranch %155
%154 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%159 = OpLoad %v4float %158
OpStore %152 %159
OpBranch %155
%155 = OpLabel
%160 = OpLoad %v4float %152
OpReturnValue %160
OpFunctionEnd
