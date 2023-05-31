OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpName %i "i"
OpName %f "f"
OpName %val "val"
OpName %mask "mask"
OpName %imask "imask"
OpName %one "one"
OpName %m "m"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %one RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %m RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_5 = OpConstant %int 5
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%_ptr_Function_float = OpTypePointer Function %float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_2_5 = OpConstant %float 2.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%uint_0 = OpConstant %uint 0
%129 = OpConstantComposite %v2uint %uint_0 %uint_0
%v2bool = OpTypeVector %bool 2
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%i = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%val = OpVariable %_ptr_Function_uint Function
%mask = OpVariable %_ptr_Function_v2uint Function
%imask = OpVariable %_ptr_Function_v2int Function
%one = OpVariable %_ptr_Function_float Function
%m = OpVariable %_ptr_Function_mat4v4float Function
%146 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpStore %i %int_5
%34 = OpIAdd %int %int_5 %int_1
OpStore %i %34
OpSelectionMerge %37 None
OpBranchConditional %true %36 %37
%36 = OpLabel
%39 = OpIEqual %bool %34 %int_6
OpBranch %37
%37 = OpLabel
%40 = OpPhi %bool %false %25 %39 %36
OpStore %ok %40
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpIAdd %int %34 %int_1
OpStore %i %43
%45 = OpIEqual %bool %43 %int_7
OpBranch %42
%42 = OpLabel
%46 = OpPhi %bool %false %37 %45 %41
OpStore %ok %46
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%49 = OpLoad %int %i
%50 = OpISub %int %49 %int_1
OpStore %i %50
%51 = OpIEqual %bool %50 %int_6
OpBranch %48
%48 = OpLabel
%52 = OpPhi %bool %false %42 %51 %47
OpStore %ok %52
%53 = OpLoad %int %i
%54 = OpISub %int %53 %int_1
OpStore %i %54
OpSelectionMerge %56 None
OpBranchConditional %52 %55 %56
%55 = OpLabel
%57 = OpIEqual %bool %54 %int_5
OpBranch %56
%56 = OpLabel
%58 = OpPhi %bool %false %48 %57 %55
OpStore %ok %58
OpStore %f %float_0_5
%63 = OpFAdd %float %float_0_5 %float_1
OpStore %f %63
OpSelectionMerge %65 None
OpBranchConditional %58 %64 %65
%64 = OpLabel
%67 = OpFOrdEqual %bool %63 %float_1_5
OpBranch %65
%65 = OpLabel
%68 = OpPhi %bool %false %56 %67 %64
OpStore %ok %68
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpFAdd %float %63 %float_1
OpStore %f %71
%73 = OpFOrdEqual %bool %71 %float_2_5
OpBranch %70
%70 = OpLabel
%74 = OpPhi %bool %false %65 %73 %69
OpStore %ok %74
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %float %f
%78 = OpFSub %float %77 %float_1
OpStore %f %78
%79 = OpFOrdEqual %bool %78 %float_1_5
OpBranch %76
%76 = OpLabel
%80 = OpPhi %bool %false %70 %79 %75
OpStore %ok %80
%81 = OpLoad %float %f
%82 = OpFSub %float %81 %float_1
OpStore %f %82
OpSelectionMerge %84 None
OpBranchConditional %80 %83 %84
%83 = OpLabel
%85 = OpFOrdEqual %bool %82 %float_0_5
OpBranch %84
%84 = OpLabel
%86 = OpPhi %bool %false %76 %85 %83
OpStore %ok %86
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%93 = OpLoad %v4float %90
%94 = OpCompositeExtract %float %93 0
%95 = OpFOrdEqual %bool %94 %float_1
%89 = OpLogicalNot %bool %95
OpBranch %88
%88 = OpLabel
%96 = OpPhi %bool %false %84 %89 %87
OpStore %ok %96
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%101 = OpLoad %v4float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpConvertFToU %uint %102
OpStore %val %103
%107 = OpNot %uint %103
%108 = OpCompositeConstruct %v2uint %103 %107
OpStore %mask %108
%112 = OpNot %v2uint %108
%113 = OpCompositeExtract %uint %112 0
%114 = OpBitcast %int %113
%115 = OpCompositeExtract %uint %112 1
%116 = OpBitcast %int %115
%117 = OpCompositeConstruct %v2int %114 %116
OpStore %imask %117
%118 = OpNot %v2uint %108
%119 = OpNot %v2int %117
%120 = OpCompositeExtract %int %119 0
%121 = OpBitcast %uint %120
%122 = OpCompositeExtract %int %119 1
%123 = OpBitcast %uint %122
%124 = OpCompositeConstruct %v2uint %121 %123
%125 = OpBitwiseAnd %v2uint %118 %124
OpStore %mask %125
OpSelectionMerge %127 None
OpBranchConditional %96 %126 %127
%126 = OpLabel
%130 = OpIEqual %v2bool %125 %129
%132 = OpAll %bool %130
OpBranch %127
%127 = OpLabel
%133 = OpPhi %bool %false %88 %132 %126
OpStore %ok %133
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%136 = OpLoad %v4float %135
%137 = OpCompositeExtract %float %136 0
OpStore %one %137
%141 = OpCompositeConstruct %v4float %137 %float_0 %float_0 %float_0
%142 = OpCompositeConstruct %v4float %float_0 %137 %float_0 %float_0
%143 = OpCompositeConstruct %v4float %float_0 %float_0 %137 %float_0
%144 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %137
%145 = OpCompositeConstruct %mat4v4float %141 %142 %143 %144
OpStore %m %145
OpSelectionMerge %150 None
OpBranchConditional %133 %148 %149
%148 = OpLabel
%151 = OpFNegate %v4float %141
%152 = OpFNegate %v4float %142
%153 = OpFNegate %v4float %143
%154 = OpFNegate %v4float %144
%155 = OpCompositeConstruct %mat4v4float %151 %152 %153 %154
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%157 = OpLoad %v4float %156
%158 = OpFNegate %v4float %157
%159 = OpMatrixTimesVector %v4float %155 %158
OpStore %146 %159
OpBranch %150
%149 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%161 = OpLoad %v4float %160
OpStore %146 %161
OpBranch %150
%150 = OpLabel
%162 = OpLoad %v4float %146
OpReturnValue %162
OpFunctionEnd
