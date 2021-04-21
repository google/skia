OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %x RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%26 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_25 = OpConstant %float 0.25
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %26
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpLoad %v4float %x
%33 = OpCompositeExtract %float %32 3
%34 = OpFOrdEqual %bool %33 %float_1
OpBranchConditional %34 %29 %31
%29 = OpLabel
%35 = OpAccessChain %_ptr_Function_float %x %int_0
%39 = OpLoad %float %35
%41 = OpFSub %float %39 %float_0_25
OpStore %35 %41
%42 = OpLoad %v4float %x
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdLessThanEqual %bool %43 %float_0
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
OpBranch %31
%46 = OpLabel
OpBranch %30
%30 = OpLabel
OpBranch %27
%31 = OpLabel
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpLoad %v4float %x
%53 = OpCompositeExtract %float %52 2
%54 = OpFOrdGreaterThan %bool %53 %float_0
OpBranchConditional %54 %49 %51
%49 = OpLabel
%55 = OpAccessChain %_ptr_Function_float %x %int_2
%57 = OpLoad %float %55
%58 = OpFSub %float %57 %float_0_25
OpStore %55 %58
%59 = OpLoad %v4float %x
%60 = OpCompositeExtract %float %59 3
%61 = OpFOrdEqual %bool %60 %float_1
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
OpBranch %50
%63 = OpLabel
%64 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %64 %float_0
OpBranch %50
%50 = OpLabel
OpBranch %47
%51 = OpLabel
%66 = OpLoad %v4float %x
OpReturnValue %66
OpFunctionEnd
