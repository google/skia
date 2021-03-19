OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %B "B"
OpName %F "F"
OpName %I "I"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1_23000002 = OpConstant %float 1.23000002
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_int = OpTypePointer Function %int
%false = OpConstantFalse %bool
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%B = OpVariable %_ptr_Function_v3bool Function
%F = OpVariable %_ptr_Function_v3float Function
%I = OpVariable %_ptr_Function_v3int Function
%21 = OpAccessChain %_ptr_Function_bool %B %int_0
OpStore %21 %true
%25 = OpAccessChain %_ptr_Function_bool %B %int_1
OpStore %25 %true
%27 = OpAccessChain %_ptr_Function_bool %B %int_2
OpStore %27 %true
%33 = OpAccessChain %_ptr_Function_float %F %int_0
OpStore %33 %float_1_23000002
%36 = OpAccessChain %_ptr_Function_float %F %int_1
OpStore %36 %float_0
%38 = OpAccessChain %_ptr_Function_float %F %int_2
OpStore %38 %float_1
%42 = OpAccessChain %_ptr_Function_int %I %int_0
OpStore %42 %int_1
%44 = OpAccessChain %_ptr_Function_int %I %int_1
OpStore %44 %int_1
%45 = OpAccessChain %_ptr_Function_int %I %int_2
OpStore %45 %int_1
%46 = OpLoad %v3float %F
%47 = OpCompositeExtract %float %46 0
%48 = OpLoad %v3float %F
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v3float %F
%52 = OpCompositeExtract %float %51 2
%53 = OpFMul %float %50 %52
%55 = OpLoad %v3bool %B
%56 = OpCompositeExtract %bool %55 0
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %v3bool %B
%60 = OpCompositeExtract %bool %59 1
OpBranch %58
%58 = OpLabel
%61 = OpPhi %bool %false %16 %60 %57
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %v3bool %B
%65 = OpCompositeExtract %bool %64 2
OpBranch %63
%63 = OpLabel
%66 = OpPhi %bool %false %58 %65 %62
%67 = OpSelect %float %66 %float_1 %float_0
%68 = OpLoad %v3int %I
%69 = OpCompositeExtract %int %68 0
%70 = OpLoad %v3int %I
%71 = OpCompositeExtract %int %70 1
%72 = OpIMul %int %69 %71
%73 = OpLoad %v3int %I
%74 = OpCompositeExtract %int %73 2
%75 = OpIMul %int %72 %74
%76 = OpConvertSToF %float %75
%77 = OpCompositeConstruct %v4float %53 %67 %float_0 %76
OpReturnValue %77
OpFunctionEnd
