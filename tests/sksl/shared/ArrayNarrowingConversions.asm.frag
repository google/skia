OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %i2 "i2"
OpName %s2 "s2"
OpName %f2 "f2"
OpName %h2 "h2"
OpName %cf2 "cf2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_int_int_2 ArrayStride 16
OpDecorate %s2 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_ptr_Function__arr_int_int_2_0 = OpTypePointer Function %_arr_int_int_2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%s2 = OpVariable %_ptr_Function__arr_int_int_2_0 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2_0 Function
%cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%95 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%35 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %s2 %35
%41 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %41
%44 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %h2 %44
%45 = OpLoad %_arr_int_int_2 %s2
OpStore %i2 %45
%46 = OpLoad %_arr_int_int_2 %i2
OpStore %s2 %46
%47 = OpLoad %_arr_float_int_2 %h2
OpStore %f2 %47
%48 = OpLoad %_arr_float_int_2 %f2
OpStore %h2 %48
OpStore %cf2 %41
%51 = OpLoad %_arr_int_int_2 %i2
%52 = OpLoad %_arr_int_int_2 %s2
%53 = OpCompositeExtract %int %51 0
%54 = OpCompositeExtract %int %52 0
%55 = OpIEqual %bool %53 %54
%56 = OpCompositeExtract %int %51 1
%57 = OpCompositeExtract %int %52 1
%58 = OpIEqual %bool %56 %57
%59 = OpLogicalAnd %bool %58 %55
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %_arr_float_int_2 %f2
%63 = OpLoad %_arr_float_int_2 %h2
%64 = OpCompositeExtract %float %62 0
%65 = OpCompositeExtract %float %63 0
%66 = OpFOrdEqual %bool %64 %65
%67 = OpCompositeExtract %float %62 1
%68 = OpCompositeExtract %float %63 1
%69 = OpFOrdEqual %bool %67 %68
%70 = OpLogicalAnd %bool %69 %66
OpBranch %61
%61 = OpLabel
%71 = OpPhi %bool %false %25 %70 %60
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %_arr_int_int_2 %i2
%75 = OpCompositeExtract %int %74 0
%76 = OpCompositeExtract %int %32 0
%77 = OpIEqual %bool %75 %76
%78 = OpCompositeExtract %int %74 1
%79 = OpCompositeExtract %int %32 1
%80 = OpIEqual %bool %78 %79
%81 = OpLogicalAnd %bool %80 %77
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %61 %81 %72
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %_arr_float_int_2 %h2
%86 = OpLoad %_arr_float_int_2 %cf2
%87 = OpCompositeExtract %float %85 0
%88 = OpCompositeExtract %float %86 0
%89 = OpFOrdEqual %bool %87 %88
%90 = OpCompositeExtract %float %85 1
%91 = OpCompositeExtract %float %86 1
%92 = OpFOrdEqual %bool %90 %91
%93 = OpLogicalAnd %bool %92 %89
OpBranch %84
%84 = OpLabel
%94 = OpPhi %bool %false %73 %93 %83
OpSelectionMerge %99 None
OpBranchConditional %94 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %100
OpStore %95 %103
OpBranch %99
%98 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
OpStore %95 %105
OpBranch %99
%99 = OpLabel
%106 = OpLoad %v4float %95
OpReturnValue %106
OpFunctionEnd
