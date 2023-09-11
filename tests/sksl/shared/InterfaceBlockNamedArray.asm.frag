### Compilation failed:

error: SPIR-V validation error: Block decoration on target <id> '12[%_arr_testBlock_int_2]' must be a structure type
  OpDecorate %_arr_testBlock_int_2 Block

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testBlock "testBlock"
               OpMemberName %testBlock 0 "s"
               OpMemberName %testBlock 1 "m"
               OpMemberName %testBlock 2 "a"
               OpMemberName %testBlock 3 "am"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
               OpMemberDecorate %testBlock 0 Offset 0
               OpMemberDecorate %testBlock 1 Offset 16
               OpMemberDecorate %testBlock 1 ColMajor
               OpMemberDecorate %testBlock 1 MatrixStride 16
               OpMemberDecorate %testBlock 2 Offset 48
               OpMemberDecorate %testBlock 3 Offset 80
               OpDecorate %_arr_testBlock_int_2 ArrayStride 144
               OpDecorate %_arr_testBlock_int_2 Block
               OpDecorate %3 Binding 123
               OpDecorate %3 DescriptorSet 0
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %38 RelaxedPrecision
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
  %testBlock = OpTypeStruct %float %mat2v2float %_arr_float_int_2 %_arr_mat2v2float_int_2
%_arr_testBlock_int_2 = OpTypeArray %testBlock %int_2
%_ptr_Uniform__arr_testBlock_int_2 = OpTypePointer Uniform %_arr_testBlock_int_2
          %3 = OpVariable %_ptr_Uniform__arr_testBlock_int_2 Uniform
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_3 = OpConstant %int 3
       %main = OpFunction %void None %21
         %22 = OpLabel
         %24 = OpAccessChain %_ptr_Uniform_float %3 %int_0 %int_0
         %26 = OpLoad %float %24
         %28 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %int_1 %int_1
         %30 = OpLoad %v2float %28
         %31 = OpCompositeExtract %float %30 0
         %32 = OpAccessChain %_ptr_Uniform_float %3 %int_0 %int_2 %int_1
         %33 = OpLoad %float %32
         %35 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %int_3 %int_1 %int_0
         %36 = OpLoad %v2float %35
         %37 = OpCompositeExtract %float %36 1
         %38 = OpCompositeConstruct %v4float %26 %31 %33 %37
               OpStore %sk_FragColor %38
               OpReturn
               OpFunctionEnd

1 error
