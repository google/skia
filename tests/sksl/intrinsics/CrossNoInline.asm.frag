               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "ah"
               OpMemberName %_UniformBuffer 1 "bh"
               OpMemberName %_UniformBuffer 2 "af"
               OpMemberName %_UniformBuffer 3 "bf"
               OpName %cross_length_2d_ff2f2 "cross_length_2d_ff2f2"    ; id %6
               OpName %cross_length_2d_hh2h2 "cross_length_2d_hh2h2"    ; id %7
               OpName %main "main"                                      ; id %8

               ; Annotations
               OpDecorate %cross_length_2d_hh2h2 RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 8
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 16
               OpMemberDecorate %_UniformBuffer 3 Offset 24
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %18 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
       %void = OpTypeVoid
         %35 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


               ; Function cross_length_2d_ff2f2
%cross_length_2d_ff2f2 = OpFunction %float None %18
         %19 = OpFunctionParameter %_ptr_Function_v2float
         %20 = OpFunctionParameter %_ptr_Function_v2float

         %21 = OpLabel
         %23 =   OpLoad %v2float %19
         %24 =   OpLoad %v2float %20
         %26 =   OpCompositeConstruct %mat2v2float %23 %24
         %22 =   OpExtInst %float %5 Determinant %26
                 OpReturnValue %22
               OpFunctionEnd


               ; Function cross_length_2d_hh2h2
%cross_length_2d_hh2h2 = OpFunction %float None %18     ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %29 = OpLabel
         %31 =   OpLoad %v2float %27                ; RelaxedPrecision
         %32 =   OpLoad %v2float %28                ; RelaxedPrecision
         %33 =   OpCompositeConstruct %mat2v2float %31 %32  ; RelaxedPrecision
         %30 =   OpExtInst %float %5 Determinant %33        ; RelaxedPrecision
                 OpReturnValue %30
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %35

         %36 = OpLabel
         %41 =   OpVariable %_ptr_Function_v2float Function
         %45 =   OpVariable %_ptr_Function_v2float Function
         %52 =   OpVariable %_ptr_Function_v2float Function
         %56 =   OpVariable %_ptr_Function_v2float Function
         %37 =   OpAccessChain %_ptr_Uniform_v2float %13 %int_0
         %40 =   OpLoad %v2float %37                ; RelaxedPrecision
                 OpStore %41 %40
         %42 =   OpAccessChain %_ptr_Uniform_v2float %13 %int_1
         %44 =   OpLoad %v2float %42                ; RelaxedPrecision
                 OpStore %45 %44
         %46 =   OpFunctionCall %float %cross_length_2d_hh2h2 %41 %45
         %47 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %47 %46
         %49 =   OpAccessChain %_ptr_Uniform_v2float %13 %int_2
         %51 =   OpLoad %v2float %49
                 OpStore %52 %51
         %53 =   OpAccessChain %_ptr_Uniform_v2float %13 %int_3
         %55 =   OpLoad %v2float %53
                 OpStore %56 %55
         %57 =   OpFunctionCall %float %cross_length_2d_ff2f2 %52 %56
         %58 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
                 OpStore %58 %57
                 OpReturn
               OpFunctionEnd
