               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %outParameterWrite_vh4 "outParameterWrite_vh4"    ; id %6
               OpName %outParameterWriteIndirect_vh4 "outParameterWriteIndirect_vh4"    ; id %7
               OpName %inoutParameterWrite_vh4 "inoutParameterWrite_vh4"                ; id %8
               OpName %inoutParameterWriteIndirect_vh4 "inoutParameterWriteIndirect_vh4"    ; id %9
               OpName %main "main"                                                          ; id %10
               OpName %c "c"                                                                ; id %55

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %29 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %52 = OpTypeFunction %v4float %_ptr_Function_v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %25 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %25 %24
         %27 =   OpFunctionCall %v4float %main %25
                 OpStore %sk_FragColor %27
                 OpReturn
               OpFunctionEnd


               ; Function outParameterWrite_vh4
%outParameterWrite_vh4 = OpFunction %void None %29
         %30 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %31 = OpLabel
         %32 =   OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %35 =   OpLoad %v4float %32                ; RelaxedPrecision
                 OpStore %30 %35
                 OpReturn
               OpFunctionEnd


               ; Function outParameterWriteIndirect_vh4
%outParameterWriteIndirect_vh4 = OpFunction %void None %29
         %36 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %37 = OpLabel
         %38 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %39 =   OpFunctionCall %void %outParameterWrite_vh4 %38
         %40 =   OpLoad %v4float %38                ; RelaxedPrecision
                 OpStore %36 %40
                 OpReturn
               OpFunctionEnd


               ; Function inoutParameterWrite_vh4
%inoutParameterWrite_vh4 = OpFunction %void None %29
         %41 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %42 = OpLabel
         %43 =   OpLoad %v4float %41                ; RelaxedPrecision
         %44 =   OpLoad %v4float %41                ; RelaxedPrecision
         %45 =   OpFMul %v4float %43 %44            ; RelaxedPrecision
                 OpStore %41 %45
                 OpReturn
               OpFunctionEnd


               ; Function inoutParameterWriteIndirect_vh4
%inoutParameterWriteIndirect_vh4 = OpFunction %void None %29
         %46 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %47 = OpLabel
         %49 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %48 =   OpLoad %v4float %46                            ; RelaxedPrecision
                 OpStore %49 %48
         %50 =   OpFunctionCall %void %inoutParameterWrite_vh4 %49
         %51 =   OpLoad %v4float %49                ; RelaxedPrecision
                 OpStore %46 %51
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %52         ; RelaxedPrecision
         %53 = OpFunctionParameter %_ptr_Function_v2float

         %54 = OpLabel
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %56 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %59 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %62 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %65 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %57 =   OpFunctionCall %void %outParameterWrite_vh4 %56
         %58 =   OpLoad %v4float %56                ; RelaxedPrecision
                 OpStore %c %58
         %60 =   OpFunctionCall %void %outParameterWriteIndirect_vh4 %59
         %61 =   OpLoad %v4float %59                ; RelaxedPrecision
                 OpStore %c %61
                 OpStore %62 %61
         %63 =   OpFunctionCall %void %inoutParameterWrite_vh4 %62
         %64 =   OpLoad %v4float %62                ; RelaxedPrecision
                 OpStore %c %64
                 OpStore %65 %64
         %66 =   OpFunctionCall %void %inoutParameterWriteIndirect_vh4 %65
         %67 =   OpLoad %v4float %65                ; RelaxedPrecision
                 OpStore %c %67
                 OpReturnValue %67
               OpFunctionEnd
