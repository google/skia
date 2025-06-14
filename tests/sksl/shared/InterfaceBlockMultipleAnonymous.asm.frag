               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlockA "testBlockA"      ; id %10
               OpMemberName %testBlockA 0 "x"
               OpName %testBlockB "testBlockB"      ; id %13
               OpMemberName %testBlockB 0 "y"
               OpName %sk_FragColor "sk_FragColor"  ; id %15
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %testBlockA 0 Offset 0
               OpDecorate %testBlockA Block
               OpDecorate %7 Binding 1
               OpDecorate %7 DescriptorSet 0
               OpMemberDecorate %testBlockB 0 Offset 0
               OpDecorate %testBlockB Block
               OpDecorate %12 Binding 2
               OpDecorate %12 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
 %testBlockA = OpTypeStruct %v2float                ; Block
%_ptr_Uniform_testBlockA = OpTypePointer Uniform %testBlockA
          %7 = OpVariable %_ptr_Uniform_testBlockA Uniform  ; Binding 1, DescriptorSet 0
 %testBlockB = OpTypeStruct %v2float                        ; Block
%_ptr_Uniform_testBlockB = OpTypePointer Uniform %testBlockB
         %12 = OpVariable %_ptr_Uniform_testBlockB Uniform  ; Binding 2, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function main
       %main = OpFunction %void None %19

         %20 = OpLabel
         %22 =   OpAccessChain %_ptr_Uniform_v2float %7 %int_0
         %24 =   OpLoad %v2float %22
         %25 =   OpCompositeExtract %float %24 0    ; RelaxedPrecision
         %26 =   OpCompositeExtract %float %24 1    ; RelaxedPrecision
         %27 =   OpAccessChain %_ptr_Uniform_v2float %12 %int_0
         %28 =   OpLoad %v2float %27
         %29 =   OpCompositeExtract %float %28 0    ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %28 1    ; RelaxedPrecision
         %31 =   OpCompositeConstruct %v4float %25 %26 %29 %30  ; RelaxedPrecision
                 OpStore %sk_FragColor %31
                 OpReturn
               OpFunctionEnd
