               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %13
               OpMemberName %testBlock 0 "x"
               OpMemberName %testBlock 1 "w"
               OpMemberName %testBlock 2 "y"
               OpMemberName %testBlock 3 "z"
               OpName %sk_FragColor "sk_FragColor"  ; id %15
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpMemberDecorate %testBlock 0 Offset 0
               OpMemberDecorate %testBlock 0 RelaxedPrecision
               OpMemberDecorate %testBlock 1 Offset 4
               OpMemberDecorate %testBlock 2 Offset 16
               OpMemberDecorate %testBlock 2 RelaxedPrecision
               OpMemberDecorate %testBlock 3 Offset 48
               OpMemberDecorate %testBlock 3 ColMajor
               OpMemberDecorate %testBlock 3 MatrixStride 16
               OpMemberDecorate %testBlock 3 RelaxedPrecision
               OpDecorate %testBlock Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
  %testBlock = OpTypeStruct %float %int %_arr_float_int_2 %mat3v3float  ; Block
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
          %7 = OpVariable %_ptr_Uniform_testBlock Uniform   ; Binding 0, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
    %float_0 = OpConstant %float 0


               ; Function main
       %main = OpFunction %void None %19

         %20 = OpLabel
         %22 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %24 =   OpLoad %float %22                  ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_Uniform_float %7 %int_2 %int_0
         %26 =   OpLoad %float %25                  ; RelaxedPrecision
         %28 =   OpAccessChain %_ptr_Uniform_float %7 %int_2 %int_1
         %29 =   OpLoad %float %28                  ; RelaxedPrecision
         %31 =   OpCompositeConstruct %v4float %24 %26 %29 %float_0     ; RelaxedPrecision
                 OpStore %sk_FragColor %31
                 OpReturn
               OpFunctionEnd
