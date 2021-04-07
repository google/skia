OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct_S "returns_a_struct_S"
OpName %s "s"
OpName %accepts_a_struct_fS "accepts_a_struct_fS"
OpName %modifies_a_struct_vS "modifies_a_struct_vS"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %expected "expected"
OpName %valid "valid"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %35 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%23 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%36 = OpTypeFunction %float %_ptr_Function_S
%45 = OpTypeFunction %void %_ptr_Function_S
%54 = OpTypeFunction %v4float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%20 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %23
%24 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%29 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %29 %float_1
%33 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %33 %int_2
%35 = OpLoad %S %s
OpReturnValue %35
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_S
%38 = OpLabel
%39 = OpAccessChain %_ptr_Function_float %37 %int_0
%40 = OpLoad %float %39
%41 = OpAccessChain %_ptr_Function_int %37 %int_1
%42 = OpLoad %int %41
%43 = OpConvertSToF %float %42
%44 = OpFAdd %float %40 %43
OpReturnValue %44
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %45
%46 = OpFunctionParameter %_ptr_Function_S
%47 = OpLabel
%48 = OpAccessChain %_ptr_Function_float %46 %int_0
%49 = OpLoad %float %48
%50 = OpFAdd %float %49 %float_1
OpStore %48 %50
%51 = OpAccessChain %_ptr_Function_int %46 %int_1
%52 = OpLoad %int %51
%53 = OpIAdd %int %52 %int_1
OpStore %51 %53
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%60 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%122 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %57
%59 = OpLoad %S %s_0
OpStore %60 %59
%61 = OpFunctionCall %float %accepts_a_struct_fS %60
OpStore %x %61
%62 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%66 = OpCompositeConstruct %S %float_2 %int_3
OpStore %expected %66
%70 = OpLoad %float %x
%72 = OpFOrdEqual %bool %70 %float_3
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%76 = OpLoad %float %75
%77 = OpFOrdEqual %bool %76 %float_2
OpBranch %74
%74 = OpLabel
%78 = OpPhi %bool %false %55 %77 %73
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%82 = OpLoad %int %81
%83 = OpIEqual %bool %82 %int_3
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %74 %83 %79
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %S %s_0
%88 = OpLoad %S %expected
%89 = OpCompositeExtract %float %87 0
%90 = OpCompositeExtract %float %88 0
%91 = OpFOrdEqual %bool %89 %90
%92 = OpCompositeExtract %int %87 1
%93 = OpCompositeExtract %int %88 1
%94 = OpIEqual %bool %92 %93
%95 = OpLogicalAnd %bool %94 %91
OpBranch %86
%86 = OpLabel
%96 = OpPhi %bool %false %80 %95 %85
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %S %s_0
%100 = OpCompositeConstruct %S %float_2 %int_3
%101 = OpCompositeExtract %float %99 0
%102 = OpCompositeExtract %float %100 0
%103 = OpFOrdEqual %bool %101 %102
%104 = OpCompositeExtract %int %99 1
%105 = OpCompositeExtract %int %100 1
%106 = OpIEqual %bool %104 %105
%107 = OpLogicalAnd %bool %106 %103
OpBranch %98
%98 = OpLabel
%108 = OpPhi %bool %false %86 %107 %97
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %S %s_0
%112 = OpFunctionCall %S %returns_a_struct_S
%113 = OpCompositeExtract %float %111 0
%114 = OpCompositeExtract %float %112 0
%115 = OpFOrdNotEqual %bool %113 %114
%116 = OpCompositeExtract %int %111 1
%117 = OpCompositeExtract %int %112 1
%118 = OpINotEqual %bool %116 %117
%119 = OpLogicalOr %bool %118 %115
OpBranch %110
%110 = OpLabel
%120 = OpPhi %bool %false %98 %119 %109
OpStore %valid %120
%121 = OpLoad %bool %valid
OpSelectionMerge %126 None
OpBranchConditional %121 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%129 = OpLoad %v4float %127
OpStore %122 %129
OpBranch %126
%125 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%131 = OpLoad %v4float %130
OpStore %122 %131
OpBranch %126
%126 = OpLabel
%132 = OpLoad %v4float %122
OpReturnValue %132
OpFunctionEnd
