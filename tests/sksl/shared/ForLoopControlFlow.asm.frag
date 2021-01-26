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
OpName %r "r"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %29 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
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
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
OpStore %x %20
OpStore %r %float_n5
OpBranch %24
%24 = OpLabel
OpLoopMerge %28 %27 None
OpBranch %25
%25 = OpLabel
%29 = OpLoad %float %r
%31 = OpFOrdLessThan %bool %29 %float_5
OpBranchConditional %31 %26 %28
%26 = OpLabel
%33 = OpLoad %float %r
%32 = OpExtInst %float %1 FAbs %33
%34 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %34 %32
%37 = OpLoad %v4float %x
%38 = OpCompositeExtract %float %37 0
%40 = OpFOrdEqual %bool %38 %float_0
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpBranch %28
%42 = OpLabel
OpBranch %27
%27 = OpLabel
%43 = OpLoad %float %r
%44 = OpFAdd %float %43 %float_1
OpStore %r %44
OpBranch %24
%28 = OpLabel
OpStore %b %float_5
OpBranch %46
%46 = OpLabel
OpLoopMerge %50 %49 None
OpBranch %47
%47 = OpLabel
%51 = OpLoad %float %b
%52 = OpFOrdGreaterThanEqual %bool %51 %float_0
OpBranchConditional %52 %48 %50
%48 = OpLabel
%53 = OpLoad %float %b
%54 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %54 %53
%56 = OpLoad %v4float %x
%57 = OpCompositeExtract %float %56 3
%58 = OpFOrdEqual %bool %57 %float_1
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
OpBranch %49
%60 = OpLabel
%61 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %61 %float_0
OpBranch %49
%49 = OpLabel
%63 = OpLoad %float %b
%64 = OpFSub %float %63 %float_1
OpStore %b %64
OpBranch %46
%50 = OpLabel
%65 = OpLoad %v4float %x
OpReturnValue %65
OpFunctionEnd
