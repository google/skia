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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpName %i "i"
OpName %f "f"
OpName %iv "iv"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_5 = OpConstant %int 5
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%_ptr_Function_float = OpTypePointer Function %float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_2_5 = OpConstant %float 2.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_n1 = OpConstant %float -1
%109 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_n1
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%122 = OpConstantComposite %v2float %float_n1 %float_n2
%123 = OpConstantComposite %v2float %float_n3 %float_n4
%124 = OpConstantComposite %mat2v2float %122 %123
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%int_n5 = OpConstant %int -5
%155 = OpConstantComposite %v2int %int_n5 %int_5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%i = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%iv = OpVariable %_ptr_Function_v2int Function
%159 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpStore %i %int_5
%35 = OpIAdd %int %int_5 %int_1
OpStore %i %35
OpSelectionMerge %38 None
OpBranchConditional %true %37 %38
%37 = OpLabel
%40 = OpIEqual %bool %35 %int_6
OpBranch %38
%38 = OpLabel
%41 = OpPhi %bool %false %26 %40 %37
OpStore %ok %41
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpIAdd %int %35 %int_1
OpStore %i %44
%46 = OpIEqual %bool %44 %int_7
OpBranch %43
%43 = OpLabel
%47 = OpPhi %bool %false %38 %46 %42
OpStore %ok %47
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%50 = OpLoad %int %i
%51 = OpISub %int %50 %int_1
OpStore %i %51
%52 = OpIEqual %bool %51 %int_6
OpBranch %49
%49 = OpLabel
%53 = OpPhi %bool %false %43 %52 %48
OpStore %ok %53
%54 = OpLoad %int %i
%55 = OpISub %int %54 %int_1
OpStore %i %55
OpSelectionMerge %57 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
%58 = OpIEqual %bool %55 %int_5
OpBranch %57
%57 = OpLabel
%59 = OpPhi %bool %false %49 %58 %56
OpStore %ok %59
OpStore %f %float_0_5
%64 = OpFAdd %float %float_0_5 %float_1
OpStore %f %64
OpSelectionMerge %66 None
OpBranchConditional %59 %65 %66
%65 = OpLabel
%68 = OpFOrdEqual %bool %64 %float_1_5
OpBranch %66
%66 = OpLabel
%69 = OpPhi %bool %false %57 %68 %65
OpStore %ok %69
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpFAdd %float %64 %float_1
OpStore %f %72
%74 = OpFOrdEqual %bool %72 %float_2_5
OpBranch %71
%71 = OpLabel
%75 = OpPhi %bool %false %66 %74 %70
OpStore %ok %75
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %float %f
%79 = OpFSub %float %78 %float_1
OpStore %f %79
%80 = OpFOrdEqual %bool %79 %float_1_5
OpBranch %77
%77 = OpLabel
%81 = OpPhi %bool %false %71 %80 %76
OpStore %ok %81
%82 = OpLoad %float %f
%83 = OpFSub %float %82 %float_1
OpStore %f %83
OpSelectionMerge %85 None
OpBranchConditional %81 %84 %85
%84 = OpLabel
%86 = OpFOrdEqual %bool %83 %float_0_5
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %77 %86 %84
OpStore %ok %87
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%94 = OpLoad %v4float %91
%95 = OpCompositeExtract %float %94 0
%96 = OpFOrdEqual %bool %95 %float_1
%90 = OpLogicalNot %bool %96
OpBranch %89
%89 = OpLabel
%97 = OpPhi %bool %false %85 %90 %88
OpStore %ok %97
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 1
%104 = OpFNegate %float %103
%105 = OpFOrdEqual %bool %float_n1 %104
OpBranch %99
%99 = OpLabel
%106 = OpPhi %bool %false %89 %105 %98
OpStore %ok %106
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%111 = OpLoad %v4float %110
%112 = OpFNegate %v4float %111
%113 = OpFOrdEqual %v4bool %109 %112
%115 = OpAll %bool %113
OpBranch %108
%108 = OpLabel
%116 = OpPhi %bool %false %99 %115 %107
OpStore %ok %116
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%128 = OpLoad %mat2v2float %125
%129 = OpCompositeExtract %v2float %128 0
%130 = OpFNegate %v2float %129
%131 = OpCompositeExtract %v2float %128 1
%132 = OpFNegate %v2float %131
%133 = OpCompositeConstruct %mat2v2float %130 %132
%135 = OpFOrdEqual %v2bool %122 %130
%136 = OpAll %bool %135
%137 = OpFOrdEqual %v2bool %123 %132
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %136 %138
OpBranch %118
%118 = OpLabel
%140 = OpPhi %bool %false %108 %139 %117
OpStore %ok %140
%144 = OpSNegate %int %55
%145 = OpCompositeConstruct %v2int %55 %144
OpStore %iv %145
OpSelectionMerge %147 None
OpBranchConditional %140 %146 %147
%146 = OpLabel
%148 = OpSNegate %int %55
%150 = OpIEqual %bool %148 %int_n5
OpBranch %147
%147 = OpLabel
%151 = OpPhi %bool %false %118 %150 %146
OpStore %ok %151
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpSNegate %v2int %145
%156 = OpIEqual %v2bool %154 %155
%157 = OpAll %bool %156
OpBranch %153
%153 = OpLabel
%158 = OpPhi %bool %false %147 %157 %152
OpStore %ok %158
OpSelectionMerge %163 None
OpBranchConditional %158 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%165 = OpLoad %v4float %164
OpStore %159 %165
OpBranch %163
%162 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%167 = OpLoad %v4float %166
OpStore %159 %167
OpBranch %163
%163 = OpLabel
%168 = OpLoad %v4float %159
OpReturnValue %168
OpFunctionEnd
