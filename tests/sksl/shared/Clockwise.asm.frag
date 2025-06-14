               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_Clockwise "sk_Clockwise"  ; id %7
               OpName %sk_FragColor "sk_FragColor"  ; id %10
               OpName %main "main"                  ; id %6
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"    ; id %19
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"

               ; Annotations
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %17 Binding 0
               OpDecorate %17 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input   ; BuiltIn FrontFacing
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float    ; Block
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %17 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform     ; Binding 0, DescriptorSet 0
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
    %float_0 = OpConstant %float 0
      %int_1 = OpConstant %int 1
     %int_n1 = OpConstant %int -1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %22 =   OpAccessChain %_ptr_Uniform_v2float %17 %int_0
         %24 =   OpLoad %v2float %22
         %25 =   OpCompositeExtract %float %24 1
         %27 =   OpFOrdGreaterThan %bool %25 %float_0
         %28 =   OpLoad %bool %sk_Clockwise         ; RelaxedPrecision
         %29 =   OpLogicalNotEqual %bool %27 %28
         %30 =   OpSelect %int %29 %int_1 %int_n1
         %33 =   OpConvertSToF %float %30           ; RelaxedPrecision
         %34 =   OpCompositeConstruct %v4float %33 %33 %33 %33  ; RelaxedPrecision
                 OpStore %sk_FragColor %34
                 OpReturn
               OpFunctionEnd
