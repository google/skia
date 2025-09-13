               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %inputVal "inputVal"              ; id %27
               OpName %expected "expected"              ; id %39

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
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
   %float_n2 = OpConstant %float -2
    %float_1 = OpConstant %float 1
    %float_8 = OpConstant %float 8
         %37 = OpConstantComposite %v4float %float_2 %float_n2 %float_1 %float_8
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %43 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_0_0500000007 = OpConstant %float 0.0500000007
    %v3float = OpTypeVector %float 3
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
   %inputVal =   OpVariable %_ptr_Function_v4float Function
   %expected =   OpVariable %_ptr_Function_v4float Function
        %100 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29
         %38 =   OpFAdd %v4float %32 %37
                 OpStore %inputVal %38
                 OpStore %expected %43
         %48 =   OpCompositeExtract %float %38 0
         %47 =   OpExtInst %float %5 Length %48
         %49 =   OpFSub %float %47 %float_3
         %46 =   OpExtInst %float %5 FAbs %49
         %51 =   OpFOrdLessThan %bool %46 %float_0_0500000007
                 OpSelectionMerge %53 None
                 OpBranchConditional %51 %52 %53

         %52 =     OpLabel
         %56 =       OpVectorShuffle %v2float %38 %38 0 1
         %55 =       OpExtInst %float %5 Length %56
         %57 =       OpFSub %float %55 %float_3
         %54 =       OpExtInst %float %5 FAbs %57
         %58 =       OpFOrdLessThan %bool %54 %float_0_0500000007
                     OpBranch %53

         %53 = OpLabel
         %59 =   OpPhi %bool %false %26 %58 %52
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %64 =       OpVectorShuffle %v3float %38 %38 0 1 2
         %63 =       OpExtInst %float %5 Length %64
         %66 =       OpFSub %float %63 %float_5
         %62 =       OpExtInst %float %5 FAbs %66
         %67 =       OpFOrdLessThan %bool %62 %float_0_0500000007
                     OpBranch %61

         %61 = OpLabel
         %68 =   OpPhi %bool %false %53 %67 %60
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %69 %70

         %69 =     OpLabel
         %72 =       OpExtInst %float %5 Length %38
         %73 =       OpFSub %float %72 %float_13
         %71 =       OpExtInst %float %5 FAbs %73
         %74 =       OpFOrdLessThan %bool %71 %float_0_0500000007
                     OpBranch %70

         %70 = OpLabel
         %75 =   OpPhi %bool %false %61 %74 %69
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpFSub %float %float_3 %float_3
         %78 =       OpExtInst %float %5 FAbs %79
         %80 =       OpFOrdLessThan %bool %78 %float_0_0500000007
                     OpBranch %77

         %77 = OpLabel
         %81 =   OpPhi %bool %false %70 %80 %76
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
         %85 =       OpFSub %float %float_3 %float_3
         %84 =       OpExtInst %float %5 FAbs %85
         %86 =       OpFOrdLessThan %bool %84 %float_0_0500000007
                     OpBranch %83

         %83 = OpLabel
         %87 =   OpPhi %bool %false %77 %86 %82
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %91 =       OpFSub %float %float_5 %float_5
         %90 =       OpExtInst %float %5 FAbs %91
         %92 =       OpFOrdLessThan %bool %90 %float_0_0500000007
                     OpBranch %89

         %89 = OpLabel
         %93 =   OpPhi %bool %false %83 %92 %88
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %94 %95

         %94 =     OpLabel
         %97 =       OpFSub %float %float_13 %float_13
         %96 =       OpExtInst %float %5 FAbs %97
         %98 =       OpFOrdLessThan %bool %96 %float_0_0500000007
                     OpBranch %95

         %95 = OpLabel
         %99 =   OpPhi %bool %false %89 %98 %94
                 OpSelectionMerge %103 None
                 OpBranchConditional %99 %101 %102

        %101 =     OpLabel
        %104 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %106 =       OpLoad %v4float %104           ; RelaxedPrecision
                     OpStore %100 %106
                     OpBranch %103

        %102 =     OpLabel
        %107 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %109 =       OpLoad %v4float %107           ; RelaxedPrecision
                     OpStore %100 %109
                     OpBranch %103

        %103 = OpLabel
        %110 =   OpLoad %v4float %100               ; RelaxedPrecision
                 OpReturnValue %110
               OpFunctionEnd
