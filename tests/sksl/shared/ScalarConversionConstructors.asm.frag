OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %f "f"
OpName %i "i"
OpName %u "u"
OpName %b "b"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %u1 "u1"
OpName %u2 "u2"
OpName %u3 "u3"
OpName %u4 "u4"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %b RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_float = OpTypePointer Private %float
%f = OpVariable %_ptr_Private_float Private
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%i = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%_ptr_Private_uint = OpTypePointer Private %uint
%u = OpVariable %_ptr_Private_uint Private
%uint_1 = OpConstant %uint 1
%_ptr_Private_bool = OpTypePointer Private %bool
%b = OpVariable %_ptr_Private_bool Private
%true = OpConstantTrue %bool
%void = OpTypeVoid
%25 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_0 = OpConstant %uint 0
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %25
%26 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_int Function
%u1 = OpVariable %_ptr_Function_uint Function
%u2 = OpVariable %_ptr_Function_uint Function
%u3 = OpVariable %_ptr_Function_uint Function
%u4 = OpVariable %_ptr_Function_uint Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
%b4 = OpVariable %_ptr_Function_bool Function
OpStore %f %float_1
OpStore %i %int_1
OpStore %u %uint_1
OpStore %b %true
%29 = OpLoad %float %f
OpStore %f1 %29
%31 = OpLoad %int %i
%32 = OpConvertSToF %float %31
OpStore %f2 %32
%34 = OpLoad %uint %u
%35 = OpConvertUToF %float %34
OpStore %f3 %35
%37 = OpLoad %bool %b
%38 = OpSelect %float %37 %float_1 %float_0
OpStore %f4 %38
%42 = OpLoad %float %f
%43 = OpConvertFToS %int %42
OpStore %i1 %43
%45 = OpLoad %int %i
OpStore %i2 %45
%47 = OpLoad %uint %u
%48 = OpBitcast %int %47
OpStore %i3 %48
%50 = OpLoad %bool %b
%51 = OpSelect %int %50 %int_1 %int_0
OpStore %i4 %51
%55 = OpLoad %float %f
%56 = OpConvertFToU %uint %55
OpStore %u1 %56
%58 = OpLoad %int %i
%59 = OpBitcast %uint %58
OpStore %u2 %59
%61 = OpLoad %uint %u
OpStore %u3 %61
%63 = OpLoad %bool %b
%64 = OpSelect %uint %63 %uint_1 %uint_0
OpStore %u4 %64
%68 = OpLoad %float %f
%69 = OpFUnordNotEqual %bool %68 %float_0
OpStore %b1 %69
%71 = OpLoad %int %i
%72 = OpINotEqual %bool %71 %int_0
OpStore %b2 %72
%74 = OpLoad %uint %u
%75 = OpINotEqual %bool %74 %uint_0
OpStore %b3 %75
%77 = OpLoad %bool %b
OpStore %b4 %77
%78 = OpLoad %float %f1
%79 = OpLoad %float %f2
%80 = OpFAdd %float %78 %79
%81 = OpLoad %float %f3
%82 = OpFAdd %float %80 %81
%83 = OpLoad %float %f4
%84 = OpFAdd %float %82 %83
%85 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %85 %84
%87 = OpLoad %int %i1
%88 = OpLoad %int %i2
%89 = OpIAdd %int %87 %88
%90 = OpLoad %int %i3
%91 = OpIAdd %int %89 %90
%92 = OpLoad %int %i4
%93 = OpIAdd %int %91 %92
%94 = OpConvertSToF %float %93
%95 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %95 %94
%96 = OpLoad %uint %u1
%97 = OpLoad %uint %u2
%98 = OpIAdd %uint %96 %97
%99 = OpLoad %uint %u3
%100 = OpIAdd %uint %98 %99
%101 = OpLoad %uint %u4
%102 = OpIAdd %uint %100 %101
%103 = OpConvertUToF %float %102
%104 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %104 %103
%105 = OpLoad %bool %b1
OpSelectionMerge %107 None
OpBranchConditional %105 %107 %106
%106 = OpLabel
%108 = OpLoad %bool %b2
OpBranch %107
%107 = OpLabel
%109 = OpPhi %bool %true %26 %108 %106
OpSelectionMerge %111 None
OpBranchConditional %109 %111 %110
%110 = OpLabel
%112 = OpLoad %bool %b3
OpBranch %111
%111 = OpLabel
%113 = OpPhi %bool %true %107 %112 %110
OpSelectionMerge %115 None
OpBranchConditional %113 %115 %114
%114 = OpLabel
%116 = OpLoad %bool %b4
OpBranch %115
%115 = OpLabel
%117 = OpPhi %bool %true %111 %116 %114
%118 = OpSelect %float %117 %float_1 %float_0
%119 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %119 %118
OpReturn
OpFunctionEnd
