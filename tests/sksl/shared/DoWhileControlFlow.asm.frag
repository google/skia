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
OpDecorate %23 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
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
%19 = OpAccessChain %_ptr_Function_float %x %int_0
%23 = OpLoad %float %19
%25 = OpFSub %float %23 %float_0_25
OpStore %19 %25
%26 = OpLoad %v4float %x
%27 = OpCompositeExtract %float %26 0
%29 = OpFOrdLessThanEqual %bool %27 %float_0
OpSelectionMerge %31 None
OpBranchConditional %29 %30 %31
%30 = OpLabel
OpBranch %18
%31 = OpLabel
OpBranch %16
%16 = OpLabel
%32 = OpLoad %v4float %x
%33 = OpCompositeExtract %float %32 3
%34 = OpFOrdEqual %bool %33 %float_1
OpBranchConditional %34 %17 %18
%17 = OpLabel
OpBranch %14
%18 = OpLabel
OpBranch %35
%35 = OpLabel
OpLoopMerge %39 %38 None
OpBranch %36
%36 = OpLabel
%40 = OpAccessChain %_ptr_Function_float %x %int_2
%42 = OpLoad %float %40
%43 = OpFSub %float %42 %float_0_25
OpStore %40 %43
%44 = OpLoad %v4float %x
%45 = OpCompositeExtract %float %44 3
%46 = OpFOrdEqual %bool %45 %float_1
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
OpBranch %38
%48 = OpLabel
%49 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %49 %float_0
OpBranch %37
%37 = OpLabel
%51 = OpLoad %v4float %x
%52 = OpCompositeExtract %float %51 2
%53 = OpFOrdGreaterThan %bool %52 %float_0
OpBranchConditional %53 %38 %39
%38 = OpLabel
OpBranch %35
%39 = OpLabel
%54 = OpLoad %v4float %x
OpReturnValue %54
OpFunctionEnd

1 error
