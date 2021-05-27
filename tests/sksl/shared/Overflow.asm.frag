OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %huge "huge"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %huge RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_9_99999962e_35 = OpConstant %float 9.99999962e+35
%float_1e_09 = OpConstant %float 1e+09
%float_1 = OpConstant %float 1
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
%huge = OpVariable %_ptr_Function_float Function
%27 = OpFMul %float %float_9_99999962e_35 %float_1e_09
%28 = OpFMul %float %27 %float_1e_09
%29 = OpFMul %float %28 %float_1e_09
%30 = OpFMul %float %29 %float_1e_09
%31 = OpFMul %float %30 %float_1e_09
%32 = OpFMul %float %31 %float_1e_09
%33 = OpFMul %float %32 %float_1e_09
%34 = OpFMul %float %33 %float_1e_09
%35 = OpFMul %float %34 %float_1e_09
%36 = OpFMul %float %35 %float_1e_09
%37 = OpFMul %float %36 %float_1e_09
%38 = OpFMul %float %37 %float_1e_09
%39 = OpFMul %float %38 %float_1e_09
%40 = OpFMul %float %39 %float_1e_09
%41 = OpFMul %float %40 %float_1e_09
%42 = OpFMul %float %41 %float_1e_09
%43 = OpFMul %float %42 %float_1e_09
%44 = OpFMul %float %43 %float_1e_09
%45 = OpFMul %float %44 %float_1e_09
%46 = OpFMul %float %45 %float_1e_09
%47 = OpFMul %float %46 %float_1e_09
%48 = OpFMul %float %47 %float_1e_09
OpStore %huge %48
%50 = OpLoad %float %huge
%51 = OpLoad %float %huge
%52 = OpCompositeConstruct %v4float %float_0 %50 %float_0 %51
%53 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%55 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%49 = OpExtInst %v4float %1 FClamp %52 %53 %55
OpReturnValue %49
OpFunctionEnd
