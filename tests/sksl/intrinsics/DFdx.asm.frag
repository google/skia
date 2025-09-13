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
               OpName %expected "expected"              ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
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
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
         %78 = OpConstantComposite %v2float %float_1 %float_1
         %96 = OpConstantComposite %v2float %float_1 %float_0
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
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %100 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %29
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %36 0    ; RelaxedPrecision
         %32 =   OpDPdx %float %37                  ; RelaxedPrecision
         %38 =   OpFOrdEqual %bool %32 %float_0
                 OpSelectionMerge %40 None
                 OpBranchConditional %38 %39 %40

         %39 =     OpLabel
         %42 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %43 =       OpLoad %v4float %42            ; RelaxedPrecision
         %44 =       OpVectorShuffle %v2float %43 %43 0 1   ; RelaxedPrecision
         %41 =       OpDPdx %v2float %44                    ; RelaxedPrecision
         %45 =       OpVectorShuffle %v2float %29 %29 0 1   ; RelaxedPrecision
         %46 =       OpFOrdEqual %v2bool %41 %45
         %48 =       OpAll %bool %46
                     OpBranch %40

         %40 = OpLabel
         %49 =   OpPhi %bool %false %26 %48 %39
                 OpSelectionMerge %51 None
                 OpBranchConditional %49 %50 %51

         %50 =     OpLabel
         %53 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =       OpLoad %v4float %53            ; RelaxedPrecision
         %55 =       OpVectorShuffle %v3float %54 %54 0 1 2     ; RelaxedPrecision
         %52 =       OpDPdx %v3float %55                        ; RelaxedPrecision
         %57 =       OpVectorShuffle %v3float %29 %29 0 1 2     ; RelaxedPrecision
         %58 =       OpFOrdEqual %v3bool %52 %57
         %60 =       OpAll %bool %58
                     OpBranch %51

         %51 = OpLabel
         %61 =   OpPhi %bool %false %40 %60 %50
                 OpSelectionMerge %63 None
                 OpBranchConditional %61 %62 %63

         %62 =     OpLabel
         %65 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %66 =       OpLoad %v4float %65            ; RelaxedPrecision
         %64 =       OpDPdx %v4float %66            ; RelaxedPrecision
         %67 =       OpFOrdEqual %v4bool %64 %29
         %69 =       OpAll %bool %67
                     OpBranch %63

         %63 = OpLabel
         %70 =   OpPhi %bool %false %51 %69 %62
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %75 =       OpLoad %v2float %25
         %76 =       OpVectorShuffle %v2float %75 %75 0 0
         %74 =       OpDPdx %v2float %76
         %73 =       OpExtInst %v2float %5 FSign %74
         %79 =       OpFOrdEqual %v2bool %73 %78
         %80 =       OpAll %bool %79
                     OpBranch %72

         %72 = OpLabel
         %81 =   OpPhi %bool %false %63 %80 %71
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
         %86 =       OpLoad %v2float %25
         %87 =       OpVectorShuffle %v2float %86 %86 1 1
         %85 =       OpDPdx %v2float %87
         %84 =       OpExtInst %v2float %5 FSign %85
         %88 =       OpFOrdEqual %v2bool %84 %20
         %89 =       OpAll %bool %88
                     OpBranch %83

         %83 = OpLabel
         %90 =   OpPhi %bool %false %72 %89 %82
                 OpSelectionMerge %92 None
                 OpBranchConditional %90 %91 %92

         %91 =     OpLabel
         %95 =       OpLoad %v2float %25
         %94 =       OpDPdx %v2float %95
         %93 =       OpExtInst %v2float %5 FSign %94
         %97 =       OpFOrdEqual %v2bool %93 %96
         %98 =       OpAll %bool %97
                     OpBranch %92

         %92 = OpLabel
         %99 =   OpPhi %bool %false %83 %98 %91
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
