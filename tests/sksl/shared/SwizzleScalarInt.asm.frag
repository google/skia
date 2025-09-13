               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %i "i"                            ; id %27
               OpName %i4 "i4"                          ; id %34

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
         %i4 =   OpVariable %_ptr_Function_v4int Function
         %29 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %32 =   OpLoad %float %29                  ; RelaxedPrecision
         %33 =   OpConvertFToS %int %32
                 OpStore %i %33
         %37 =   OpCompositeConstruct %v4int %33 %33 %33 %33
                 OpStore %i4 %37
         %39 =   OpCompositeConstruct %v2int %33 %33
         %40 =   OpCompositeExtract %int %39 0
         %41 =   OpCompositeExtract %int %39 1
         %43 =   OpCompositeConstruct %v4int %40 %41 %int_0 %int_1
                 OpStore %i4 %43
         %44 =   OpCompositeConstruct %v4int %int_0 %33 %int_1 %int_0
                 OpStore %i4 %44
         %45 =   OpCompositeConstruct %v4int %int_0 %33 %int_0 %33
                 OpStore %i4 %45
         %46 =   OpConvertSToF %float %int_0        ; RelaxedPrecision
         %47 =   OpCompositeExtract %int %45 1
         %48 =   OpConvertSToF %float %47           ; RelaxedPrecision
         %49 =   OpCompositeExtract %int %45 2
         %50 =   OpConvertSToF %float %49           ; RelaxedPrecision
         %51 =   OpCompositeExtract %int %45 3
         %52 =   OpConvertSToF %float %51           ; RelaxedPrecision
         %53 =   OpCompositeConstruct %v4float %46 %48 %50 %52  ; RelaxedPrecision
                 OpReturnValue %53
               OpFunctionEnd
