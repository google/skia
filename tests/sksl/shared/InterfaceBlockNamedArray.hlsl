### Compilation failed:

error: SPIR-V validation error: Block decoration on target <id> '15[%_arr_testBlock_int_2]' must be a structure type
  OpDecorate %_arr_testBlock_int_2 Block

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %14
               OpMemberName %testBlock 0 "s"
               OpMemberName %testBlock 1 "m"
               OpMemberName %testBlock 2 "a"
               OpMemberName %testBlock 3 "am"
               OpName %sk_FragColor "sk_FragColor"  ; id %17
               OpName %main "main"                  ; id %6

               ; Annotations
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
               OpDecorate %7 Binding 123
               OpDecorate %7 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %38 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2   ; ArrayStride 32
  %testBlock = OpTypeStruct %float %mat2v2float %_arr_float_int_2 %_arr_mat2v2float_int_2
%_arr_testBlock_int_2 = OpTypeArray %testBlock %int_2   ; ArrayStride 144, Block
%_ptr_Uniform__arr_testBlock_int_2 = OpTypePointer Uniform %_arr_testBlock_int_2
          %7 = OpVariable %_ptr_Uniform__arr_testBlock_int_2 Uniform    ; Binding 123, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_3 = OpConstant %int 3


               ; Function main
       %main = OpFunction %void None %21

         %22 = OpLabel
         %24 =   OpAccessChain %_ptr_Uniform_float %7 %int_0 %int_0
         %26 =   OpLoad %float %24
         %28 =   OpAccessChain %_ptr_Uniform_v2float %7 %int_1 %int_1 %int_1
         %30 =   OpLoad %v2float %28
         %31 =   OpCompositeExtract %float %30 0
         %32 =   OpAccessChain %_ptr_Uniform_float %7 %int_0 %int_2 %int_1
         %33 =   OpLoad %float %32
         %35 =   OpAccessChain %_ptr_Uniform_v2float %7 %int_1 %int_3 %int_1 %int_0
         %36 =   OpLoad %v2float %35
         %37 =   OpCompositeExtract %float %36 1
         %38 =   OpCompositeConstruct %v4float %26 %31 %33 %37  ; RelaxedPrecision
                 OpStore %sk_FragColor %38
                 OpReturn
               OpFunctionEnd

1 error
