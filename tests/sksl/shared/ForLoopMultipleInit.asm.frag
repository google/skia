OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %result RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%int_10 = OpConstant %int 10
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%a = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_int Function
OpStore %result %20
OpStore %a %int_0
OpStore %b %int_0
OpBranch %26
%26 = OpLabel
OpLoopMerge %30 %29 None
OpBranch %27
%27 = OpLabel
%32 = OpLoad %int %a
%34 = OpSLessThan %bool %32 %int_10
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%37 = OpLoad %int %b
%38 = OpSLessThan %bool %37 %int_10
OpBranch %36
%36 = OpLabel
%39 = OpPhi %bool %false %27 %38 %35
OpBranchConditional %39 %28 %30
%28 = OpLabel
%40 = OpAccessChain %_ptr_Function_float %result %int_0
%42 = OpLoad %float %40
%44 = OpFAdd %float %42 %float_1
OpStore %40 %44
%45 = OpAccessChain %_ptr_Function_float %result %int_1
%47 = OpLoad %float %45
%49 = OpFAdd %float %47 %float_2
OpStore %45 %49
OpBranch %29
%29 = OpLabel
%50 = OpLoad %int %a
%51 = OpIAdd %int %50 %int_1
OpStore %a %51
%52 = OpLoad %int %b
%53 = OpIAdd %int %52 %int_1
OpStore %b %53
OpBranch %26
%30 = OpLabel
%54 = OpLoad %v4float %result
OpReturnValue %54
OpFunctionEnd
