               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %7
               OpMemberName %testBlock 0 "m1"
               OpMemberName %testBlock 1 "m2"
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %main "main"                  ; id %2

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
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
  %testBlock = OpTypeStruct %mat2v2float %mat2v2float   ; Block
%_ptr_PushConstant_testBlock = OpTypePointer PushConstant %testBlock
          %3 = OpVariable %_ptr_PushConstant_testBlock PushConstant
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_v2float = OpTypePointer PushConstant %v2float
      %int_1 = OpConstant %int 1


               ; Function main
       %main = OpFunction %void None %13

         %14 = OpLabel
         %17 =   OpAccessChain %_ptr_PushConstant_v2float %3 %int_0 %int_0
         %19 =   OpLoad %v2float %17                ; RelaxedPrecision
         %20 =   OpCompositeExtract %float %19 0    ; RelaxedPrecision
         %22 =   OpAccessChain %_ptr_PushConstant_v2float %3 %int_0 %int_1
         %23 =   OpLoad %v2float %22                ; RelaxedPrecision
         %24 =   OpCompositeExtract %float %23 1    ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_PushConstant_v2float %3 %int_1 %int_0
         %26 =   OpLoad %v2float %25                ; RelaxedPrecision
         %27 =   OpCompositeExtract %float %26 0    ; RelaxedPrecision
         %28 =   OpAccessChain %_ptr_PushConstant_v2float %3 %int_1 %int_1
         %29 =   OpLoad %v2float %28                ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 1    ; RelaxedPrecision
         %31 =   OpCompositeConstruct %v4float %20 %24 %27 %30  ; RelaxedPrecision
                 OpStore %sk_FragColor %31
                 OpReturn
               OpFunctionEnd
