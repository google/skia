               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %inputVal "inputVal"              ; id %35

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_n1 = OpConstant %float -1
   %float_n4 = OpConstant %float -4
  %float_n16 = OpConstant %float -16
  %float_n64 = OpConstant %float -64
         %33 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
         %49 = OpConstantComposite %v4float %float_0 %float_2 %float_6 %float_12
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%float_0_0500000007 = OpConstant %float 0.0500000007
         %67 = OpConstantComposite %v2float %float_1 %float_2
         %69 = OpConstantComposite %v2float %float_0_0500000007 %float_0_0500000007
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_3 = OpConstant %float 3
         %81 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %83 = OpConstantComposite %v3float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v3bool = OpTypeVector %bool 3
    %float_4 = OpConstant %float 4
         %93 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %95 = OpConstantComposite %v4float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
   %inputVal =   OpVariable %_ptr_Function_v4float Function
         %98 =   OpVariable %_ptr_Function_v4float Function
         %28 =   OpExtInst %v4float %5 Sqrt %33
         %34 =   OpVectorShuffle %v2float %28 %28 0 1
                 OpStore %26 %34
         %37 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_0
         %40 =   OpLoad %mat2v2float %37
         %41 =   OpCompositeExtract %float %40 0 0
         %42 =   OpCompositeExtract %float %40 0 1
         %43 =   OpCompositeExtract %float %40 1 0
         %44 =   OpCompositeExtract %float %40 1 1
         %45 =   OpCompositeConstruct %v4float %41 %42 %43 %44
         %50 =   OpFAdd %v4float %45 %49
                 OpStore %inputVal %50
         %55 =   OpCompositeExtract %float %50 0
         %54 =   OpExtInst %float %5 Sqrt %55
         %57 =   OpFSub %float %54 %float_1
         %53 =   OpExtInst %float %5 FAbs %57
         %59 =   OpFOrdLessThan %bool %53 %float_0_0500000007
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %66 =       OpVectorShuffle %v2float %50 %50 0 1
         %65 =       OpExtInst %v2float %5 Sqrt %66
         %68 =       OpFSub %v2float %65 %67
         %64 =       OpExtInst %v2float %5 FAbs %68
         %63 =       OpFOrdLessThan %v2bool %64 %69
         %62 =       OpAll %bool %63
                     OpBranch %61

         %61 = OpLabel
         %71 =   OpPhi %bool %false %27 %62 %60
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
         %78 =       OpVectorShuffle %v3float %50 %50 0 1 2
         %77 =       OpExtInst %v3float %5 Sqrt %78
         %82 =       OpFSub %v3float %77 %81
         %76 =       OpExtInst %v3float %5 FAbs %82
         %75 =       OpFOrdLessThan %v3bool %76 %83
         %74 =       OpAll %bool %75
                     OpBranch %73

         %73 = OpLabel
         %85 =   OpPhi %bool %false %61 %74 %72
                 OpSelectionMerge %87 None
                 OpBranchConditional %85 %86 %87

         %86 =     OpLabel
         %91 =       OpExtInst %v4float %5 Sqrt %50
         %94 =       OpFSub %v4float %91 %93
         %90 =       OpExtInst %v4float %5 FAbs %94
         %89 =       OpFOrdLessThan %v4bool %90 %95
         %88 =       OpAll %bool %89
                     OpBranch %87

         %87 = OpLabel
         %97 =   OpPhi %bool %false %73 %88 %86
                 OpSelectionMerge %101 None
                 OpBranchConditional %97 %99 %100

         %99 =     OpLabel
        %102 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %105 =       OpLoad %v4float %102           ; RelaxedPrecision
                     OpStore %98 %105
                     OpBranch %101

        %100 =     OpLabel
        %106 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %108 =       OpLoad %v4float %106           ; RelaxedPrecision
                     OpStore %98 %108
                     OpBranch %101

        %101 = OpLabel
        %109 =   OpLoad %v4float %98                ; RelaxedPrecision
                 OpReturnValue %109
               OpFunctionEnd
