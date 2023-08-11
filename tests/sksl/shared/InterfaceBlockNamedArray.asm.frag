### Compilation failed:

error: SPIR-V validation error: Block decoration on target <id> '10[%_arr_testBlock_int_2]' must be a structure type
  OpDecorate %_arr_testBlock_int_2 Block

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testBlock "testBlock"
               OpMemberName %testBlock 0 "x"
               OpMemberName %testBlock 1 "m"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpMemberDecorate %testBlock 0 Offset 0
               OpMemberDecorate %testBlock 1 Offset 16
               OpMemberDecorate %testBlock 1 ColMajor
               OpMemberDecorate %testBlock 1 MatrixStride 16
               OpDecorate %_arr_testBlock_int_2 ArrayStride 48
               OpDecorate %_arr_testBlock_int_2 Block
               OpDecorate %3 Binding 123
               OpDecorate %3 DescriptorSet 0
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
  %testBlock = OpTypeStruct %float %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
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
         %19 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %19
         %20 = OpLabel
         %22 = OpAccessChain %_ptr_Uniform_float %3 %int_0 %int_0
         %24 = OpLoad %float %22
         %26 = OpAccessChain %_ptr_Uniform_v2float %3 %int_0 %int_1 %int_0
         %28 = OpLoad %v2float %26
         %29 = OpCompositeExtract %float %28 1
         %30 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %int_1 %int_1
         %31 = OpLoad %v2float %30
         %32 = OpCompositeExtract %float %31 0
         %33 = OpCompositeExtract %float %31 1
         %34 = OpCompositeConstruct %v4float %24 %29 %32 %33
               OpStore %sk_FragColor %34
               OpReturn
               OpFunctionEnd

1 error
