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
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
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
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_5 = OpConstant %int 5
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%a = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_int Function
%c = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_int Function
%50 = OpVariable %_ptr_Function_v4float Function
OpStore %a %int_0
OpStore %b %int_0
OpStore %c %int_0
OpStore %d %int_0
OpStore %a %int_1
OpStore %b %int_2
OpStore %c %int_5
%37 = OpIEqual %bool %int_1 %int_1
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpIEqual %bool %int_2 %int_2
OpBranch %39
%39 = OpLabel
%41 = OpPhi %bool %false %25 %40 %38
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpIEqual %bool %int_5 %int_5
OpBranch %43
%43 = OpLabel
%45 = OpPhi %bool %false %39 %44 %42
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%48 = OpIEqual %bool %int_0 %int_0
OpBranch %47
%47 = OpLabel
%49 = OpPhi %bool %false %43 %48 %46
OpSelectionMerge %54 None
OpBranchConditional %49 %52 %53
%52 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%57 = OpLoad %v4float %55
OpStore %50 %57
OpBranch %54
%53 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%59 = OpLoad %v4float %58
OpStore %50 %59
OpBranch %54
%54 = OpLabel
%60 = OpLoad %v4float %50
OpReturnValue %60
OpFunctionEnd
