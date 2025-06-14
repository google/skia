               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%float_n0_021816615 = OpConstant %float -0.021816615
%float_0_000500000024 = OpConstant %float 0.000500000024
         %49 = OpConstantComposite %v2float %float_n0_021816615 %float_0
         %51 = OpConstantComposite %v2float %float_0_000500000024 %float_0_000500000024
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
%float_0_0130899698 = OpConstant %float 0.0130899698
         %65 = OpConstantComposite %v3float %float_n0_021816615 %float_0 %float_0_0130899698
         %67 = OpConstantComposite %v3float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v3bool = OpTypeVector %bool 3
%float_0_0392699093 = OpConstant %float 0.0392699093
         %79 = OpConstantComposite %v4float %float_n0_021816615 %float_0 %float_0_0130899698 %float_0_0392699093
         %81 = OpConstantComposite %v4float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


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
         %84 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %31
         %35 =   OpCompositeExtract %float %34 0
         %30 =   OpExtInst %float %5 Radians %35
         %37 =   OpFSub %float %30 %float_n0_021816615
         %29 =   OpExtInst %float %5 FAbs %37
         %39 =   OpFOrdLessThan %bool %29 %float_0_000500000024
                 OpSelectionMerge %41 None
                 OpBranchConditional %39 %40 %41

         %40 =     OpLabel
         %46 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =       OpLoad %v4float %46
         %48 =       OpVectorShuffle %v2float %47 %47 0 1
         %45 =       OpExtInst %v2float %5 Radians %48
         %50 =       OpFSub %v2float %45 %49
         %44 =       OpExtInst %v2float %5 FAbs %50
         %43 =       OpFOrdLessThan %v2bool %44 %51
         %42 =       OpAll %bool %43
                     OpBranch %41

         %41 = OpLabel
         %53 =   OpPhi %bool %false %26 %42 %40
                 OpSelectionMerge %55 None
                 OpBranchConditional %53 %54 %55

         %54 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %61 =       OpLoad %v4float %60
         %62 =       OpVectorShuffle %v3float %61 %61 0 1 2
         %59 =       OpExtInst %v3float %5 Radians %62
         %66 =       OpFSub %v3float %59 %65
         %58 =       OpExtInst %v3float %5 FAbs %66
         %57 =       OpFOrdLessThan %v3bool %58 %67
         %56 =       OpAll %bool %57
                     OpBranch %55

         %55 = OpLabel
         %69 =   OpPhi %bool %false %41 %56 %54
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %77 =       OpLoad %v4float %76
         %75 =       OpExtInst %v4float %5 Radians %77
         %80 =       OpFSub %v4float %75 %79
         %74 =       OpExtInst %v4float %5 FAbs %80
         %73 =       OpFOrdLessThan %v4bool %74 %81
         %72 =       OpAll %bool %73
                     OpBranch %71

         %71 = OpLabel
         %83 =   OpPhi %bool %false %55 %72 %70
                 OpSelectionMerge %88 None
                 OpBranchConditional %83 %86 %87

         %86 =     OpLabel
         %89 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %91 =       OpLoad %v4float %89            ; RelaxedPrecision
                     OpStore %84 %91
                     OpBranch %88

         %87 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %94 =       OpLoad %v4float %92            ; RelaxedPrecision
                     OpStore %84 %94
                     OpBranch %88

         %88 = OpLabel
         %95 =   OpLoad %v4float %84                ; RelaxedPrecision
                 OpReturnValue %95
               OpFunctionEnd
