OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %A "A"
OpMemberName %A 0 "x"
OpMemberName %A 1 "y"
OpName %a1 "a1"
OpName %a4 "a4"
OpName %B "B"
OpMemberName %B 0 "x"
OpMemberName %B 1 "y"
OpMemberName %B 2 "z"
OpName %b1 "b1"
OpName %b4 "b4"
OpName %main "main"
OpName %C "C"
OpMemberName %C 0 "i"
OpMemberName %C 1 "j"
OpName %c1 "c1"
OpName %c2 "c2"
OpName %c3 "c3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %A 0 Offset 0
OpMemberDecorate %A 1 Offset 4
OpMemberDecorate %A 1 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %B 0 Offset 0
OpMemberDecorate %B 0 RelaxedPrecision
OpMemberDecorate %B 1 Offset 16
OpMemberDecorate %B 2 Binding 1
OpMemberDecorate %B 2 Offset 48
OpMemberDecorate %B 2 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpMemberDecorate %C 0 Offset 0
OpMemberDecorate %C 0 RelaxedPrecision
OpMemberDecorate %C 1 Offset 16
OpMemberDecorate %C 1 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%A = OpTypeStruct %int %float
%_ptr_Private_A = OpTypePointer Private %A
%a1 = OpVariable %_ptr_Private_A Private
%a4 = OpVariable %_ptr_Private_A Private
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%B = OpTypeStruct %float %_arr_float_int_2 %A
%_ptr_Private_B = OpTypePointer Private %B
%b1 = OpVariable %_ptr_Private_B Private
%b4 = OpVariable %_ptr_Private_B Private
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%int_4 = OpConstant %int 4
%float_5 = OpConstant %float 5
%void = OpTypeVoid
%32 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Private_int = OpTypePointer Private %int
%float_0 = OpConstant %float 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_A = OpTypePointer Function %A
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%C = OpTypeStruct %A %A
%_ptr_Function_C = OpTypePointer Function %C
%int_3 = OpConstant %int 3
%float_4 = OpConstant %float 4
%float_n2 = OpConstant %float -2
%int_n3 = OpConstant %int -3
%main = OpFunction %void None %32
%33 = OpLabel
%52 = OpVariable %_ptr_Function_A Function
%c1 = OpVariable %_ptr_Function_C Function
%c2 = OpVariable %_ptr_Function_C Function
%c3 = OpVariable %_ptr_Function_C Function
%17 = OpCompositeConstruct %A %int_1 %float_2
OpStore %a4 %17
%26 = OpCompositeConstruct %_arr_float_int_2 %float_2 %float_3
%29 = OpCompositeConstruct %A %int_4 %float_5
%30 = OpCompositeConstruct %B %float_1 %26 %29
OpStore %b4 %30
%35 = OpAccessChain %_ptr_Private_int %a1 %int_0
OpStore %35 %int_0
%38 = OpAccessChain %_ptr_Private_float %b1 %int_0
OpStore %38 %float_0
%40 = OpAccessChain %_ptr_Private_int %a1 %int_0
%41 = OpLoad %int %40
%42 = OpConvertSToF %float %41
%43 = OpAccessChain %_ptr_Private_float %b1 %int_0
%44 = OpLoad %float %43
%45 = OpFAdd %float %42 %44
%46 = OpAccessChain %_ptr_Private_float %a4 %int_1
%47 = OpLoad %float %46
%48 = OpFAdd %float %45 %47
%49 = OpAccessChain %_ptr_Private_float %b4 %int_0
%50 = OpLoad %float %49
%51 = OpFAdd %float %48 %50
%54 = OpCompositeConstruct %A %int_1 %float_2
OpStore %52 %54
%55 = OpAccessChain %_ptr_Function_float %52 %int_1
%57 = OpLoad %float %55
%58 = OpFAdd %float %51 %57
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %59 %58
%66 = OpCompositeConstruct %A %int_1 %float_2
%69 = OpCompositeConstruct %A %int_3 %float_4
%70 = OpCompositeConstruct %C %66 %69
OpStore %c1 %70
%72 = OpCompositeConstruct %A %int_1 %float_n2
%74 = OpCompositeConstruct %A %int_n3 %float_4
%75 = OpCompositeConstruct %C %72 %74
OpStore %c2 %75
%76 = OpCompositeConstruct %A %int_1 %float_2
%77 = OpCompositeConstruct %A %int_3 %float_4
%78 = OpCompositeConstruct %C %76 %77
OpStore %c3 %78
%79 = OpLoad %C %c1
%80 = OpLoad %C %c3
%81 = OpCompositeExtract %A %79 0
%82 = OpCompositeExtract %A %80 0
%83 = OpCompositeExtract %int %81 0
%84 = OpCompositeExtract %int %82 0
%85 = OpIEqual %bool %83 %84
%86 = OpCompositeExtract %float %81 1
%87 = OpCompositeExtract %float %82 1
%88 = OpFOrdEqual %bool %86 %87
%89 = OpLogicalAnd %bool %88 %85
%90 = OpCompositeExtract %A %79 1
%91 = OpCompositeExtract %A %80 1
%92 = OpCompositeExtract %int %90 0
%93 = OpCompositeExtract %int %91 0
%94 = OpIEqual %bool %92 %93
%95 = OpCompositeExtract %float %90 1
%96 = OpCompositeExtract %float %91 1
%97 = OpFOrdEqual %bool %95 %96
%98 = OpLogicalAnd %bool %97 %94
%99 = OpLogicalAnd %bool %98 %89
%100 = OpSelect %float %99 %float_1 %float_0
%101 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %101 %100
%102 = OpLoad %C %c2
%103 = OpLoad %C %c3
%104 = OpCompositeExtract %A %102 0
%105 = OpCompositeExtract %A %103 0
%106 = OpCompositeExtract %int %104 0
%107 = OpCompositeExtract %int %105 0
%108 = OpINotEqual %bool %106 %107
%109 = OpCompositeExtract %float %104 1
%110 = OpCompositeExtract %float %105 1
%111 = OpFOrdNotEqual %bool %109 %110
%112 = OpLogicalOr %bool %111 %108
%113 = OpCompositeExtract %A %102 1
%114 = OpCompositeExtract %A %103 1
%115 = OpCompositeExtract %int %113 0
%116 = OpCompositeExtract %int %114 0
%117 = OpINotEqual %bool %115 %116
%118 = OpCompositeExtract %float %113 1
%119 = OpCompositeExtract %float %114 1
%120 = OpFOrdNotEqual %bool %118 %119
%121 = OpLogicalOr %bool %120 %117
%122 = OpLogicalOr %bool %121 %112
%123 = OpSelect %float %122 %float_1 %float_0
%124 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %124 %123
%125 = OpAccessChain %_ptr_Function_A %c1 %int_0
%126 = OpLoad %A %125
%127 = OpAccessChain %_ptr_Function_A %c2 %int_1
%128 = OpLoad %A %127
%129 = OpCompositeExtract %int %126 0
%130 = OpCompositeExtract %int %128 0
%131 = OpINotEqual %bool %129 %130
%132 = OpCompositeExtract %float %126 1
%133 = OpCompositeExtract %float %128 1
%134 = OpFOrdNotEqual %bool %132 %133
%135 = OpLogicalOr %bool %134 %131
%136 = OpSelect %float %135 %float_1 %float_0
%137 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %137 %136
OpReturn
OpFunctionEnd
