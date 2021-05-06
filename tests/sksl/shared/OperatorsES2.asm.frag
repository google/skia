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
OpName %x "x"
OpName %y "y"
OpName %z "z"
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
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%float_6 = OpConstant %float 6
%int_1 = OpConstant %int 1
%int_6 = OpConstant %int 6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
%134 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpLoad %float %x
%36 = OpLoad %float %x
%37 = OpFSub %float %35 %36
%38 = OpLoad %float %y
%39 = OpLoad %float %x
%40 = OpFMul %float %38 %39
%41 = OpLoad %float %x
%42 = OpFMul %float %40 %41
%43 = OpLoad %float %y
%44 = OpLoad %float %x
%45 = OpFSub %float %43 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %37 %46
OpStore %x %47
%48 = OpLoad %float %x
%49 = OpLoad %float %y
%50 = OpFDiv %float %48 %49
%51 = OpLoad %float %x
%52 = OpFDiv %float %50 %51
OpStore %y %52
%53 = OpLoad %int %z
%55 = OpSDiv %int %53 %int_2
%56 = OpIMul %int %55 %int_3
%58 = OpIAdd %int %56 %int_4
%59 = OpISub %int %58 %int_2
OpStore %z %59
%63 = OpLoad %float %x
%65 = OpFOrdGreaterThan %bool %63 %float_4
%66 = OpLoad %float %x
%67 = OpFOrdLessThan %bool %66 %float_2
%68 = OpLogicalEqual %bool %65 %67
OpSelectionMerge %70 None
OpBranchConditional %68 %70 %69
%69 = OpLabel
%71 = OpLoad %float %y
%72 = OpLoad %float %x
%73 = OpFOrdLessThanEqual %bool %71 %72
OpBranch %70
%70 = OpLabel
%74 = OpPhi %bool %true %25 %73 %69
OpStore %b %74
OpStore %c %false
%78 = OpLoad %bool %b
%79 = OpLoad %bool %c
%80 = OpLogicalNotEqual %bool %78 %79
OpStore %d %80
%82 = OpLoad %bool %b
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %bool %c
OpBranch %84
%84 = OpLabel
%86 = OpPhi %bool %false %70 %85 %83
OpStore %e %86
%88 = OpLoad %bool %b
OpSelectionMerge %90 None
OpBranchConditional %88 %90 %89
%89 = OpLabel
%91 = OpLoad %bool %c
OpBranch %90
%90 = OpLabel
%92 = OpPhi %bool %true %84 %91 %89
OpStore %f %92
%93 = OpLoad %float %x
%95 = OpFAdd %float %93 %float_12
OpStore %x %95
%96 = OpLoad %float %x
%97 = OpFSub %float %96 %float_12
OpStore %x %97
%98 = OpLoad %float %x
%99 = OpLoad %float %y
%101 = OpFDiv %float %99 %float_10
OpStore %y %101
%102 = OpFMul %float %98 %101
OpStore %x %102
OpStore %x %float_6
%104 = OpLoad %bool %b
%105 = OpSelect %float %104 %float_1 %float_0
%106 = OpLoad %bool %c
%107 = OpSelect %float %106 %float_1 %float_0
%108 = OpFMul %float %105 %107
%109 = OpLoad %bool %d
%110 = OpSelect %float %109 %float_1 %float_0
%111 = OpFMul %float %108 %110
%112 = OpLoad %bool %e
%113 = OpSelect %float %112 %float_1 %float_0
%114 = OpFMul %float %111 %113
%115 = OpLoad %bool %f
%116 = OpSelect %float %115 %float_1 %float_0
%117 = OpFMul %float %114 %116
OpStore %y %117
OpStore %y %float_6
%118 = OpLoad %int %z
%120 = OpISub %int %118 %int_1
OpStore %z %120
OpStore %z %int_6
%122 = OpLoad %float %x
%123 = OpFOrdEqual %bool %122 %float_6
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %float %y
%127 = OpFOrdEqual %bool %126 %float_6
OpBranch %125
%125 = OpLabel
%128 = OpPhi %bool %false %90 %127 %124
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpLoad %int %z
%132 = OpIEqual %bool %131 %int_6
OpBranch %130
%130 = OpLabel
%133 = OpPhi %bool %false %125 %132 %129
OpSelectionMerge %138 None
OpBranchConditional %133 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%142 = OpLoad %v4float %139
OpStore %134 %142
OpBranch %138
%137 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
OpStore %134 %144
OpBranch %138
%138 = OpLabel
%145 = OpLoad %v4float %134
OpReturnValue %145
OpFunctionEnd
