               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %11
               OpMemberName %testBlock 0 "m1"
               OpMemberName %testBlock 1 "m2"
               OpName %sk_FragColor "sk_FragColor"  ; id %13
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %testBlock 0 Offset 16
               OpMemberDecorate %testBlock 0 ColMajor
               OpMemberDecorate %testBlock 0 MatrixStride 8
               OpMemberDecorate %testBlock 0 RelaxedPrecision
               OpMemberDecorate %testBlock 1 Offset 32
               OpMemberDecorate %testBlock 1 ColMajor
               OpMemberDecorate %testBlock 1 MatrixStride 8
               OpMemberDecorate %testBlock 1 RelaxedPrecision
               OpDecorate %testBlock Block
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
  %testBlock = OpTypeStruct %mat2v2float %mat2v2float   ; Block
%_ptr_PushConstant_testBlock = OpTypePointer PushConstant %testBlock
          %7 = OpVariable %_ptr_PushConstant_testBlock PushConstant
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_v2float = OpTypePointer PushConstant %v2float
      %int_1 = OpConstant %int 1


               ; Function main
       %main = OpFunction %void None %17

         %18 = OpLabel
         %20 =   OpAccessChain %_ptr_PushConstant_v2float %7 %int_0 %int_0
         %22 =   OpLoad %v2float %20                ; RelaxedPrecision
         %23 =   OpCompositeExtract %float %22 0    ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_PushConstant_v2float %7 %int_0 %int_1
         %26 =   OpLoad %v2float %25                ; RelaxedPrecision
         %27 =   OpCompositeExtract %float %26 1    ; RelaxedPrecision
         %28 =   OpAccessChain %_ptr_PushConstant_v2float %7 %int_1 %int_0
         %29 =   OpLoad %v2float %28                ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 0    ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_PushConstant_v2float %7 %int_1 %int_1
         %32 =   OpLoad %v2float %31                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
         %34 =   OpCompositeConstruct %v4float %23 %27 %30 %33  ; RelaxedPrecision
                 OpStore %sk_FragColor %34
                 OpReturn
               OpFunctionEnd
