OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %B "B"
OpName %F "F"
OpName %I "I"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_int = OpTypePointer Function %int
%false = OpConstantFalse %bool
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
%B = OpVariable %_ptr_Function_v3bool Function
%F = OpVariable %_ptr_Function_v3float Function
%I = OpVariable %_ptr_Function_v3int Function
%27 = OpAccessChain %_ptr_Function_bool %B %int_0
OpStore %27 %true
%31 = OpAccessChain %_ptr_Function_bool %B %int_1
OpStore %31 %true
%33 = OpAccessChain %_ptr_Function_bool %B %int_2
OpStore %33 %true
%39 = OpAccessChain %_ptr_Function_float %F %int_0
OpStore %39 %float_1_23000002
%41 = OpAccessChain %_ptr_Function_float %F %int_1
OpStore %41 %float_0
%43 = OpAccessChain %_ptr_Function_float %F %int_2
OpStore %43 %float_1
%47 = OpAccessChain %_ptr_Function_int %I %int_0
OpStore %47 %int_1
%49 = OpAccessChain %_ptr_Function_int %I %int_1
OpStore %49 %int_1
%50 = OpAccessChain %_ptr_Function_int %I %int_2
OpStore %50 %int_1
%51 = OpLoad %v3float %F
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v3float %F
%54 = OpCompositeExtract %float %53 1
%55 = OpFMul %float %52 %54
%56 = OpLoad %v3float %F
%57 = OpCompositeExtract %float %56 2
%58 = OpFMul %float %55 %57
%60 = OpLoad %v3bool %B
%61 = OpCompositeExtract %bool %60 0
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %v3bool %B
%65 = OpCompositeExtract %bool %64 1
OpBranch %63
%63 = OpLabel
%66 = OpPhi %bool %false %22 %65 %62
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %v3bool %B
%70 = OpCompositeExtract %bool %69 2
OpBranch %68
%68 = OpLabel
%71 = OpPhi %bool %false %63 %70 %67
%72 = OpSelect %float %71 %float_1 %float_0
%73 = OpLoad %v3int %I
%74 = OpCompositeExtract %int %73 0
%75 = OpLoad %v3int %I
%76 = OpCompositeExtract %int %75 1
%77 = OpIMul %int %74 %76
%78 = OpLoad %v3int %I
%79 = OpCompositeExtract %int %78 2
%80 = OpIMul %int %77 %79
%81 = OpConvertSToF %float %80
%82 = OpCompositeConstruct %v4float %58 %72 %float_0 %81
OpReturnValue %82
OpFunctionEnd
