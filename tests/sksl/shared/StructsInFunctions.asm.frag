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
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
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
%111 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %57
%59 = OpLoad %S %s_0
OpStore %60 %59
%61 = OpFunctionCall %float %accepts_a_struct_fS %60
OpStore %x %61
%62 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%65 = OpAccessChain %_ptr_Function_float %expected %int_0
OpStore %65 %float_2
%67 = OpAccessChain %_ptr_Function_int %expected %int_1
OpStore %67 %int_3
%71 = OpLoad %float %x
%73 = OpFOrdEqual %bool %71 %float_3
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%77 = OpLoad %float %76
%78 = OpFOrdEqual %bool %77 %float_2
OpBranch %75
%75 = OpLabel
%79 = OpPhi %bool %false %55 %78 %74
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%83 = OpLoad %int %82
%84 = OpIEqual %bool %83 %int_3
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %75 %84 %80
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %S %s_0
%89 = OpLoad %S %expected
%90 = OpCompositeExtract %float %88 0
%91 = OpCompositeExtract %float %89 0
%92 = OpFOrdEqual %bool %90 %91
%93 = OpCompositeExtract %int %88 1
%94 = OpCompositeExtract %int %89 1
%95 = OpIEqual %bool %93 %94
%96 = OpLogicalAnd %bool %95 %92
OpBranch %87
%87 = OpLabel
%97 = OpPhi %bool %false %81 %96 %86
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %S %s_0
%101 = OpFunctionCall %S %returns_a_struct_S
%102 = OpCompositeExtract %float %100 0
%103 = OpCompositeExtract %float %101 0
%104 = OpFOrdNotEqual %bool %102 %103
%105 = OpCompositeExtract %int %100 1
%106 = OpCompositeExtract %int %101 1
%107 = OpINotEqual %bool %105 %106
%108 = OpLogicalOr %bool %107 %104
OpBranch %99
%99 = OpLabel
%109 = OpPhi %bool %false %87 %108 %98
OpStore %valid %109
%110 = OpLoad %bool %valid
OpSelectionMerge %115 None
OpBranchConditional %110 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%118 = OpLoad %v4float %116
OpStore %111 %118
OpBranch %115
%114 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%120 = OpLoad %v4float %119
OpStore %111 %120
OpBranch %115
%115 = OpLabel
%121 = OpLoad %v4float %111
OpReturnValue %121
OpFunctionEnd
