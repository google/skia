               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %integerInput "integerInput"      ; id %27

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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%float_2_1400001 = OpConstant %float 2.1400001
         %41 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
         %56 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%float_3_1400001 = OpConstant %float 3.1400001
         %66 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
         %75 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %76 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1


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
%integerInput =   OpVariable %_ptr_Function_int Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
         %34 =   OpConvertFToS %int %33
                 OpStore %integerInput %34
         %35 =   OpIEqual %bool %34 %int_0
                 OpSelectionMerge %39 None
                 OpBranchConditional %35 %37 %38

         %37 =     OpLabel
                     OpReturnValue %41

         %38 =     OpLabel
         %43 =       OpIEqual %bool %34 %int_1
                     OpSelectionMerge %46 None
                     OpBranchConditional %43 %44 %45

         %44 =         OpLabel
         %47 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %48 =           OpLoad %v4float %47        ; RelaxedPrecision
                         OpReturnValue %48

         %45 =         OpLabel
         %50 =           OpIEqual %bool %34 %int_2
                         OpSelectionMerge %53 None
                         OpBranchConditional %50 %51 %52

         %51 =             OpLabel
                             OpReturnValue %56

         %52 =             OpLabel
         %58 =               OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %59 =               OpLoad %v4float %58    ; RelaxedPrecision
         %60 =               OpCompositeExtract %float %59 0    ; RelaxedPrecision
         %61 =               OpFMul %float %60 %float_3_1400001
         %62 =               OpFOrdLessThan %bool %float_3_1400001 %61
                             OpSelectionMerge %65 None
                             OpBranchConditional %62 %63 %64

         %63 =                 OpLabel
                                 OpReturnValue %66

         %64 =                 OpLabel
         %67 =                   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =                   OpLoad %v4float %67    ; RelaxedPrecision
         %69 =                   OpCompositeExtract %float %68 0    ; RelaxedPrecision
         %70 =                   OpFMul %float %69 %float_2_1400001
         %71 =                   OpFOrdGreaterThanEqual %bool %float_2_1400001 %70
                                 OpSelectionMerge %74 None
                                 OpBranchConditional %71 %72 %73

         %72 =                     OpLabel
                                     OpReturnValue %75

         %73 =                     OpLabel
                                     OpReturnValue %76

         %74 =                 OpLabel
                                 OpBranch %65

         %65 =             OpLabel
                             OpBranch %53

         %53 =         OpLabel
                         OpBranch %46

         %46 =     OpLabel
                     OpBranch %39

         %39 = OpLabel
                 OpUnreachable
               OpFunctionEnd
