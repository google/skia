OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
%float_14 = OpConstant %float 14
%float_6 = OpConstant %float 6
%float_5 = OpConstant %float 5
%float_32 = OpConstant %float 32
%float_33 = OpConstant %float 33
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_3 = OpConstant %float 3
%float_n4 = OpConstant %float -4
%float_n6 = OpConstant %float -6
%float_7 = OpConstant %float 7
%float_n8 = OpConstant %float -8
%float_9 = OpConstant %float 9
%float_n10 = OpConstant %float -10
%float_11 = OpConstant %float 11
%float_n12 = OpConstant %float -12
%float_2 = OpConstant %float 2
%float_0 = OpConstant %float 0
%float_8 = OpConstant %float 8
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_34
%19 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %19 %float_30
%21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %21 %float_64
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %23 %float_16
%25 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %25 %float_14
%27 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %27 %float_6
%29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %29 %float_5
%30 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %30 %float_16
%32 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %32 %float_32
%34 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %34 %float_33
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %float_1
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %38 %float_n2
%40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %40 %float_3
%42 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %42 %float_n4
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %float_5
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %float_n6
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %float_7
%49 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %49 %float_n8
%51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %51 %float_9
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %float_n10
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %55 %float_11
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %float_n12
%60 = OpExtInst %float %1 Sqrt %float_1
%59 = OpConvertFToS %int %60
%58 = OpConvertSToF %float %59
%61 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %61 %58
%64 = OpExtInst %float %1 Sqrt %float_2
%63 = OpConvertFToS %int %64
%62 = OpConvertSToF %float %63
%66 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %66 %62
%69 = OpExtInst %float %1 Sqrt %float_3
%68 = OpConvertFToS %int %69
%67 = OpConvertSToF %float %68
%70 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %70 %67
%72 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %72 %float_0
%75 = OpExtInst %float %1 Sqrt %float_5
%74 = OpConvertFToS %int %75
%73 = OpConvertSToF %float %74
%76 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %76 %73
%79 = OpExtInst %float %1 Sqrt %float_6
%78 = OpConvertFToS %int %79
%77 = OpConvertSToF %float %78
%80 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %80 %77
%81 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %81 %float_0
%84 = OpExtInst %float %1 Sqrt %float_8
%83 = OpConvertFToS %int %84
%82 = OpConvertSToF %float %83
%86 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %86 %82
%87 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %87 %float_0
%91 = OpExtInst %float %1 Sqrt %float_2
%90 = OpConvertFToS %int %91
OpStore %x %90
%92 = OpLoad %int %x
%94 = OpIAdd %int %92 %int_1
OpStore %x %94
%95 = OpLoad %int %x
%96 = OpISub %int %95 %int_1
OpStore %x %96
%97 = OpLoad %int %x
%99 = OpIMul %int %97 %int_2
OpStore %x %99
%100 = OpLoad %int %x
%101 = OpSDiv %int %100 %int_2
OpStore %x %101
%103 = OpLoad %int %x
%102 = OpConvertSToF %float %103
%104 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %104 %102
OpReturn
OpFunctionEnd
