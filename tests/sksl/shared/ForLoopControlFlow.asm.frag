### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '10[%colorWhite]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %colorWhite = OpVariable %_ptr_Uniform_v4float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %colorWhite "colorWhite"
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
OpDecorate %colorWhite RelaxedPrecision
OpDecorate %colorWhite DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%colorWhite = OpVariable %_ptr_Uniform_v4float Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%17 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %14
%15 = OpLabel
%16 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %16
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %17
%18 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v4float %colorWhite
OpStore %x %21
OpStore %r %float_n5
OpBranch %25
%25 = OpLabel
OpLoopMerge %29 %28 None
OpBranch %26
%26 = OpLabel
%30 = OpLoad %float %r
%32 = OpFOrdLessThan %bool %30 %float_5
OpBranchConditional %32 %27 %29
%27 = OpLabel
%34 = OpLoad %float %r
%33 = OpExtInst %float %1 FClamp %34 %float_0 %float_1
%37 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %37 %33
%40 = OpLoad %v4float %x
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %41 %float_0
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
OpBranch %29
%44 = OpLabel
OpBranch %28
%28 = OpLabel
%45 = OpLoad %float %r
%46 = OpFAdd %float %45 %float_1
OpStore %r %46
OpBranch %25
%29 = OpLabel
OpStore %b %float_5
OpBranch %48
%48 = OpLabel
OpLoopMerge %52 %51 None
OpBranch %49
%49 = OpLabel
%53 = OpLoad %float %b
%54 = OpFOrdGreaterThanEqual %bool %53 %float_0
OpBranchConditional %54 %50 %52
%50 = OpLabel
%55 = OpLoad %float %b
%56 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %56 %55
%58 = OpLoad %v4float %x
%59 = OpCompositeExtract %float %58 3
%60 = OpFOrdEqual %bool %59 %float_1
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
OpBranch %51
%62 = OpLabel
%63 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %63 %float_0
OpBranch %51
%51 = OpLabel
%65 = OpLoad %float %b
%66 = OpFSub %float %65 %float_1
OpStore %b %66
OpBranch %48
%52 = OpLabel
%67 = OpLoad %v4float %x
OpReturnValue %67
OpFunctionEnd

1 error
