### Compilation failed:

error: SPIR-V validation error: OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %19 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%8 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_25 = OpConstant %float 0.25
%float_0 = OpConstant %float 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%main = OpFunction %v4float None %8
%9 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %13
OpBranch %14
%14 = OpLabel
OpLoopMerge %18 %17 None
OpBranch %15
%15 = OpLabel
%19 = OpLoad %v4float %x
%20 = OpCompositeExtract %float %19 3
%21 = OpFOrdEqual %bool %20 %float_1
OpBranchConditional %21 %16 %18
%16 = OpLabel
%22 = OpAccessChain %_ptr_Function_float %x %int_0
%26 = OpLoad %float %22
%28 = OpFSub %float %26 %float_0_25
OpStore %22 %28
%29 = OpLoad %v4float %x
%30 = OpCompositeExtract %float %29 0
%32 = OpFOrdLessThanEqual %bool %30 %float_0
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
OpBranch %18
%34 = OpLabel
OpBranch %17
%17 = OpLabel
OpBranch %14
%18 = OpLabel
OpBranch %35
%35 = OpLabel
OpLoopMerge %39 %38 None
OpBranch %36
%36 = OpLabel
%40 = OpLoad %v4float %x
%41 = OpCompositeExtract %float %40 2
%42 = OpFOrdGreaterThan %bool %41 %float_0
OpBranchConditional %42 %37 %39
%37 = OpLabel
%43 = OpAccessChain %_ptr_Function_float %x %int_2
%45 = OpLoad %float %43
%46 = OpFSub %float %45 %float_0_25
OpStore %43 %46
%47 = OpLoad %v4float %x
%48 = OpCompositeExtract %float %47 3
%49 = OpFOrdEqual %bool %48 %float_1
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
OpBranch %38
%51 = OpLabel
%52 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %52 %float_0
OpBranch %38
%38 = OpLabel
OpBranch %35
%39 = OpLabel
%54 = OpLoad %v4float %x
OpReturnValue %54
OpFunctionEnd

1 error
