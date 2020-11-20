OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_34 = OpConstant %float 34
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_30 = OpConstant %float 30
%float_64 = OpConstant %float 64
%float_16 = OpConstant %float 16
%float_19 = OpConstant %float 19
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_3 = OpConstant %float 3
%float_n4 = OpConstant %float -4
%float_5 = OpConstant %float 5
%float_n6 = OpConstant %float -6
%float_7 = OpConstant %float 7
%float_n8 = OpConstant %float -8
%float_9 = OpConstant %float 9
%float_n10 = OpConstant %float -10
%float_11 = OpConstant %float 11
%float_n12 = OpConstant %float -12
%float_1_0 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3_0 = OpConstant %float 3
%float_0 = OpConstant %float 0
%float_5_0 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_8 = OpConstant %float 8
%float_2_0 = OpConstant %float 2
%main = OpFunction %void None %11
%12 = OpLabel
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_34
%19 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %19 %float_30
%21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %21 %float_64
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %23 %float_16
%25 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %25 %float_19
%27 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %27 %float_1
%29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %29 %float_n2
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %31 %float_3
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %float_n4
%35 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %35 %float_5
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %float_n6
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %float_7
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %float_n8
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %float_9
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %float_n10
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %float_11
%49 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %49 %float_n12
%50 = OpExtInst %float %1 Sqrt %float_1_0
%52 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %52 %50
%53 = OpExtInst %float %1 Sqrt %float_2
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %55 %53
%56 = OpExtInst %float %1 Sqrt %float_3_0
%58 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %58 %56
%60 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %60 %float_0
%61 = OpExtInst %float %1 Sqrt %float_5_0
%63 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %63 %61
%64 = OpExtInst %float %1 Sqrt %float_6
%66 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %66 %64
%67 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %67 %float_0
%68 = OpExtInst %float %1 Sqrt %float_8
%70 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %70 %68
%71 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %71 %float_0
%72 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%73 = OpLoad %float %72
%74 = OpFAdd %float %73 %float_1
OpStore %72 %74
%75 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%76 = OpLoad %float %75
%77 = OpFSub %float %76 %float_1
OpStore %75 %77
%78 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%79 = OpLoad %float %78
%81 = OpFMul %float %79 %float_2_0
OpStore %78 %81
%82 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%83 = OpLoad %float %82
%84 = OpFDiv %float %83 %float_2_0
OpStore %82 %84
OpReturn
OpFunctionEnd
