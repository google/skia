OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %i "i"
OpName %i_0 "i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%int_2 = OpConstant %int 2
%int_100 = OpConstant %int 100
%main = OpFunction %void None %11
%12 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
OpKill
%17 = OpLabel
OpStore %i %int_0
OpBranch %18
%18 = OpLabel
OpLoopMerge %22 %21 None
OpBranch %19
%19 = OpLabel
%23 = OpLoad %int %i
%25 = OpSLessThan %bool %23 %int_10
OpBranchConditional %25 %20 %22
%20 = OpLabel
%26 = OpLoad %v4float %sk_FragColor
%28 = OpVectorTimesScalar %v4float %26 %float_0_5
OpStore %sk_FragColor %28
%29 = OpLoad %int %i
%31 = OpIAdd %int %29 %int_1
OpStore %i %31
OpBranch %21
%21 = OpLabel
OpBranch %18
%22 = OpLabel
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %v4float %sk_FragColor
%39 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%40 = OpFAdd %v4float %37 %39
OpStore %sk_FragColor %40
OpBranch %34
%34 = OpLabel
OpBranch %35
%35 = OpLabel
%41 = OpLoad %v4float %sk_FragColor
%42 = OpCompositeExtract %float %41 0
%44 = OpFOrdLessThan %bool %42 %float_0_75
OpBranchConditional %44 %32 %36
%36 = OpLabel
OpStore %i_0 %int_0
OpBranch %46
%46 = OpLabel
OpLoopMerge %50 %49 None
OpBranch %47
%47 = OpLabel
%51 = OpLoad %int %i_0
%52 = OpSLessThan %bool %51 %int_10
OpBranchConditional %52 %48 %50
%48 = OpLabel
%53 = OpLoad %int %i_0
%55 = OpSMod %int %53 %int_2
%56 = OpIEqual %bool %55 %int_1
OpSelectionMerge %59 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
OpBranch %50
%58 = OpLabel
%60 = OpLoad %int %i_0
%62 = OpSGreaterThan %bool %60 %int_100
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
OpReturn
%64 = OpLabel
OpBranch %49
%65 = OpLabel
OpBranch %59
%59 = OpLabel
OpBranch %49
%49 = OpLabel
%66 = OpLoad %int %i_0
%67 = OpIAdd %int %66 %int_1
OpStore %i_0 %67
OpBranch %46
%50 = OpLabel
OpReturn
OpFunctionEnd
