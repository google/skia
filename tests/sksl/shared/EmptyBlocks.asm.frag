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
OpName %color "color"
OpName %counter "counter"
OpName %counter_0 "counter"
OpName %counter_1 "counter"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %78 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%8 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%13 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%main = OpFunction %v4float None %8
%9 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%counter = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%counter_1 = OpVariable %_ptr_Function_int Function
OpStore %color %13
OpStore %counter %int_0
OpBranch %18
%18 = OpLabel
OpLoopMerge %22 %21 None
OpBranch %19
%19 = OpLabel
%23 = OpLoad %int %counter
%25 = OpSLessThan %bool %23 %int_10
OpBranchConditional %25 %20 %22
%20 = OpLabel
OpBranch %21
%21 = OpLabel
%27 = OpLoad %int %counter
%28 = OpIAdd %int %27 %int_1
OpStore %counter %28
OpBranch %18
%22 = OpLabel
OpStore %counter_0 %int_0
OpBranch %30
%30 = OpLabel
OpLoopMerge %34 %33 None
OpBranch %31
%31 = OpLabel
%35 = OpLoad %int %counter_0
%36 = OpSLessThan %bool %35 %int_10
OpBranchConditional %36 %32 %34
%32 = OpLabel
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %counter_0
%38 = OpIAdd %int %37 %int_1
OpStore %counter_0 %38
OpBranch %30
%34 = OpLabel
OpStore %counter_1 %int_0
OpBranch %40
%40 = OpLabel
OpLoopMerge %44 %43 None
OpBranch %41
%41 = OpLabel
%45 = OpLoad %int %counter_1
%46 = OpSLessThan %bool %45 %int_10
OpBranchConditional %46 %42 %44
%42 = OpLabel
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %counter_1
%48 = OpIAdd %int %47 %int_1
OpStore %counter_1 %48
OpBranch %40
%44 = OpLabel
%49 = OpExtInst %float %1 Sqrt %float_1
%51 = OpFOrdEqual %bool %49 %float_1
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%54 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %54 %float_1
OpBranch %53
%53 = OpLabel
%56 = OpExtInst %float %1 Sqrt %float_1
%58 = OpFOrdEqual %bool %56 %float_2
OpSelectionMerge %61 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
OpBranch %61
%60 = OpLabel
%62 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %62 %float_1
OpBranch %61
%61 = OpLabel
OpBranch %64
%64 = OpLabel
OpLoopMerge %68 %67 None
OpBranch %65
%65 = OpLabel
%69 = OpExtInst %float %1 Sqrt %float_1
%70 = OpFOrdEqual %bool %69 %float_2
OpBranchConditional %70 %66 %68
%66 = OpLabel
OpBranch %67
%67 = OpLabel
OpBranch %64
%68 = OpLabel
OpBranch %71
%71 = OpLabel
OpLoopMerge %75 %74 None
OpBranch %72
%72 = OpLabel
OpBranch %73
%73 = OpLabel
%76 = OpExtInst %float %1 Sqrt %float_1
%77 = OpFOrdEqual %bool %76 %float_2
OpBranchConditional %77 %74 %75
%74 = OpLabel
OpBranch %71
%75 = OpLabel
%78 = OpLoad %v4float %color
OpReturnValue %78
OpFunctionEnd

1 error
