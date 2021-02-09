OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%20 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0_25 = OpConstant %float 0.25
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %20
OpBranch %21
%21 = OpLabel
OpLoopMerge %25 %24 None
OpBranch %22
%22 = OpLabel
%26 = OpLoad %v4float %x
%27 = OpCompositeExtract %float %26 3
%28 = OpFOrdEqual %bool %27 %float_1
OpBranchConditional %28 %23 %25
%23 = OpLabel
%29 = OpLoad %v4float %x
%30 = OpCompositeExtract %float %29 0
%32 = OpFSub %float %30 %float_0_25
%33 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %33 %32
%37 = OpLoad %v4float %x
%38 = OpCompositeExtract %float %37 0
%40 = OpFOrdLessThanEqual %bool %38 %float_0
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpBranch %25
%42 = OpLabel
OpBranch %24
%24 = OpLabel
OpBranch %21
%25 = OpLabel
OpBranch %43
%43 = OpLabel
OpLoopMerge %47 %46 None
OpBranch %44
%44 = OpLabel
%48 = OpLoad %v4float %x
%49 = OpCompositeExtract %float %48 2
%50 = OpFOrdGreaterThan %bool %49 %float_0
OpBranchConditional %50 %45 %47
%45 = OpLabel
%51 = OpLoad %v4float %x
%52 = OpCompositeExtract %float %51 2
%53 = OpFSub %float %52 %float_0_25
%54 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %54 %53
%56 = OpLoad %v4float %x
%57 = OpCompositeExtract %float %56 3
%58 = OpFOrdEqual %bool %57 %float_1
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
OpBranch %46
%60 = OpLabel
%61 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %61 %float_0
OpBranch %46
%46 = OpLabel
OpBranch %43
%47 = OpLabel
%63 = OpLoad %v4float %x
OpReturnValue %63
OpFunctionEnd
