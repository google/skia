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
OpName %_entrypoint_v "_entrypoint_v"
OpName %switch_with_break_in_loop_bi "switch_with_break_in_loop_bi"
OpName %val "val"
OpName %i "i"
OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"
OpName %val_0 "val"
OpName %i_0 "i"
OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"
OpName %val_1 "val"
OpName %i_1 "i"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%28 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_11 = OpConstant %int 11
%false = OpConstantFalse %bool
%int_20 = OpConstant %int 20
%105 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%switch_with_break_in_loop_bi = OpFunction %bool None %28
%29 = OpFunctionParameter %_ptr_Function_int
%30 = OpLabel
%val = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%33 = OpLoad %int %29
OpSelectionMerge %34 None
OpSwitch %33 %36 1 %35
%35 = OpLabel
OpStore %i %int_0
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %int %i
%45 = OpSLessThan %bool %43 %int_10
OpBranchConditional %45 %40 %42
%40 = OpLabel
%47 = OpLoad %int %val
%48 = OpIAdd %int %47 %int_1
OpStore %val %48
OpBranch %42
%41 = OpLabel
%49 = OpLoad %int %i
%50 = OpIAdd %int %49 %int_1
OpStore %i %50
OpBranch %38
%42 = OpLabel
OpBranch %36
%36 = OpLabel
%51 = OpLoad %int %val
%52 = OpIAdd %int %51 %int_1
OpStore %val %52
OpBranch %34
%34 = OpLabel
%53 = OpLoad %int %val
%55 = OpIEqual %bool %53 %int_2
OpReturnValue %55
OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %28
%56 = OpFunctionParameter %_ptr_Function_int
%57 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
%59 = OpLoad %int %56
OpSelectionMerge %60 None
OpSwitch %59 %62 1 %61
%61 = OpLabel
OpStore %i_0 %int_0
OpBranch %64
%64 = OpLabel
OpLoopMerge %68 %67 None
OpBranch %65
%65 = OpLabel
%69 = OpLoad %int %i_0
%70 = OpSLessThan %bool %69 %int_10
OpBranchConditional %70 %66 %68
%66 = OpLabel
%71 = OpLoad %int %val_0
%72 = OpIAdd %int %71 %int_1
OpStore %val_0 %72
OpBranch %67
%67 = OpLabel
%73 = OpLoad %int %i_0
%74 = OpIAdd %int %73 %int_1
OpStore %i_0 %74
OpBranch %64
%68 = OpLabel
OpBranch %62
%62 = OpLabel
%75 = OpLoad %int %val_0
%76 = OpIAdd %int %75 %int_1
OpStore %val_0 %76
OpBranch %60
%60 = OpLabel
%77 = OpLoad %int %val_0
%79 = OpIEqual %bool %77 %int_11
OpReturnValue %79
OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %28
%80 = OpFunctionParameter %_ptr_Function_int
%81 = OpLabel
%val_1 = OpVariable %_ptr_Function_int Function
%i_1 = OpVariable %_ptr_Function_int Function
OpStore %val_1 %int_0
OpStore %i_1 %int_0
OpBranch %84
%84 = OpLabel
OpLoopMerge %88 %87 None
OpBranch %85
%85 = OpLabel
%89 = OpLoad %int %i_1
%90 = OpSLessThan %bool %89 %int_10
OpBranchConditional %90 %86 %88
%86 = OpLabel
%91 = OpLoad %int %80
OpSelectionMerge %92 None
OpSwitch %91 %94 1 %93
%93 = OpLabel
%95 = OpLoad %int %val_1
%96 = OpIAdd %int %95 %int_1
OpStore %val_1 %96
OpBranch %92
%94 = OpLabel
OpReturnValue %false
%92 = OpLabel
%98 = OpLoad %int %val_1
%99 = OpIAdd %int %98 %int_1
OpStore %val_1 %99
OpBranch %87
%87 = OpLabel
%100 = OpLoad %int %i_1
%101 = OpIAdd %int %100 %int_1
OpStore %i_1 %101
OpBranch %84
%88 = OpLabel
%102 = OpLoad %int %val_1
%104 = OpIEqual %bool %102 %int_20
OpReturnValue %104
OpFunctionEnd
%main = OpFunction %v4float None %105
%106 = OpFunctionParameter %_ptr_Function_v2float
%107 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%114 = OpVariable %_ptr_Function_int Function
%118 = OpVariable %_ptr_Function_int Function
%123 = OpVariable %_ptr_Function_int Function
%126 = OpVariable %_ptr_Function_v4float Function
%109 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%111 = OpLoad %v4float %109
%112 = OpCompositeExtract %float %111 1
%113 = OpConvertFToS %int %112
OpStore %x %113
OpStore %114 %113
%115 = OpFunctionCall %bool %switch_with_break_in_loop_bi %114
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
OpStore %118 %113
%119 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %118
OpBranch %117
%117 = OpLabel
%120 = OpPhi %bool %false %107 %119 %116
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
OpStore %123 %113
%124 = OpFunctionCall %bool %loop_with_break_in_switch_bi %123
OpBranch %122
%122 = OpLabel
%125 = OpPhi %bool %false %117 %124 %121
OpSelectionMerge %130 None
OpBranchConditional %125 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%132 = OpLoad %v4float %131
OpStore %126 %132
OpBranch %130
%129 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%134 = OpLoad %v4float %133
OpStore %126 %134
OpBranch %130
%130 = OpLabel
%135 = OpLoad %v4float %126
OpReturnValue %135
OpFunctionEnd
