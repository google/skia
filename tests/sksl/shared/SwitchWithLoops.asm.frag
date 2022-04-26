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
OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"
OpName %val "val"
OpName %i "i"
OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"
OpName %val_0 "val"
OpName %i_0 "i"
OpName %main "main"
OpName %x "x"
OpName %_0_val "_0_val"
OpName %_1_i "_1_i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%27 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%int_11 = OpConstant %int 11
%false = OpConstantFalse %bool
%int_20 = OpConstant %int 20
%82 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %27
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%val = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%32 = OpLoad %int %28
OpSelectionMerge %33 None
OpSwitch %32 %35 1 %34
%34 = OpLabel
OpStore %i %int_0
OpBranch %37
%37 = OpLabel
OpLoopMerge %41 %40 None
OpBranch %38
%38 = OpLabel
%42 = OpLoad %int %i
%44 = OpSLessThan %bool %42 %int_10
OpBranchConditional %44 %39 %41
%39 = OpLabel
%46 = OpLoad %int %val
%47 = OpIAdd %int %46 %int_1
OpStore %val %47
OpBranch %40
%49 = OpLabel
%48 = OpIAdd %int %47 %int_1
OpStore %val %48
OpBranch %40
%40 = OpLabel
%50 = OpLoad %int %i
%51 = OpIAdd %int %50 %int_1
OpStore %i %51
OpBranch %37
%41 = OpLabel
OpBranch %35
%35 = OpLabel
%52 = OpLoad %int %val
%53 = OpIAdd %int %52 %int_1
OpStore %val %53
OpBranch %33
%33 = OpLabel
%54 = OpLoad %int %val
%56 = OpIEqual %bool %54 %int_11
OpReturnValue %56
OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %27
%57 = OpFunctionParameter %_ptr_Function_int
%58 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
OpStore %i_0 %int_0
OpBranch %61
%61 = OpLabel
OpLoopMerge %65 %64 None
OpBranch %62
%62 = OpLabel
%66 = OpLoad %int %i_0
%67 = OpSLessThan %bool %66 %int_10
OpBranchConditional %67 %63 %65
%63 = OpLabel
%68 = OpLoad %int %57
OpSelectionMerge %69 None
OpSwitch %68 %71 1 %70
%70 = OpLabel
%72 = OpLoad %int %val_0
%73 = OpIAdd %int %72 %int_1
OpStore %val_0 %73
OpBranch %69
%71 = OpLabel
OpReturnValue %false
%69 = OpLabel
%75 = OpLoad %int %val_0
%76 = OpIAdd %int %75 %int_1
OpStore %val_0 %76
OpBranch %64
%64 = OpLabel
%77 = OpLoad %int %i_0
%78 = OpIAdd %int %77 %int_1
OpStore %i_0 %78
OpBranch %61
%65 = OpLabel
%79 = OpLoad %int %val_0
%81 = OpIEqual %bool %79 %int_20
OpReturnValue %81
OpFunctionEnd
%main = OpFunction %v4float None %82
%83 = OpFunctionParameter %_ptr_Function_v2float
%84 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%_0_val = OpVariable %_ptr_Function_int Function
%_1_i = OpVariable %_ptr_Function_int Function
%117 = OpVariable %_ptr_Function_int Function
%123 = OpVariable %_ptr_Function_int Function
%126 = OpVariable %_ptr_Function_v4float Function
%86 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%88 = OpLoad %v4float %86
%89 = OpCompositeExtract %float %88 1
%90 = OpConvertFToS %int %89
OpStore %x %90
OpStore %_0_val %int_0
OpSelectionMerge %92 None
OpSwitch %90 %94 1 %93
%93 = OpLabel
OpStore %_1_i %int_0
OpBranch %96
%96 = OpLabel
OpLoopMerge %100 %99 None
OpBranch %97
%97 = OpLabel
%101 = OpLoad %int %_1_i
%102 = OpSLessThan %bool %101 %int_10
OpBranchConditional %102 %98 %100
%98 = OpLabel
%103 = OpLoad %int %_0_val
%104 = OpIAdd %int %103 %int_1
OpStore %_0_val %104
OpBranch %100
%106 = OpLabel
%105 = OpIAdd %int %104 %int_1
OpStore %_0_val %105
OpBranch %99
%99 = OpLabel
%107 = OpLoad %int %_1_i
%108 = OpIAdd %int %107 %int_1
OpStore %_1_i %108
OpBranch %96
%100 = OpLabel
OpBranch %94
%94 = OpLabel
%109 = OpLoad %int %_0_val
%110 = OpIAdd %int %109 %int_1
OpStore %_0_val %110
OpBranch %92
%92 = OpLabel
%111 = OpLoad %int %_0_val
%113 = OpIEqual %bool %111 %int_2
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %int %x
OpStore %117 %116
%118 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %117
OpBranch %115
%115 = OpLabel
%119 = OpPhi %bool %false %92 %118 %114
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %int %x
OpStore %123 %122
%124 = OpFunctionCall %bool %loop_with_break_in_switch_bi %123
OpBranch %121
%121 = OpLabel
%125 = OpPhi %bool %false %115 %124 %120
OpSelectionMerge %130 None
OpBranchConditional %125 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%132 = OpLoad %v4float %131
OpStore %126 %132
OpBranch %130
%129 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%134 = OpLoad %v4float %133
OpStore %126 %134
OpBranch %130
%130 = OpLabel
%135 = OpLoad %v4float %126
OpReturnValue %135
OpFunctionEnd
