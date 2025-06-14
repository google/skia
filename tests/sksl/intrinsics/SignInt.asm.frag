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
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision

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
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %int_n1 = OpConstant %int -1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %33 = OpConstantComposite %v4int %int_n1 %int_0 %int_0 %int_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %104 = OpConstantComposite %v2int %int_n1 %int_0
        %111 = OpConstantComposite %v3int %int_n1 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %expected =   OpVariable %_ptr_Function_v4int Function
        %119 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %33
         %37 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %37                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0    ; RelaxedPrecision
         %41 =   OpConvertFToS %int %40
         %36 =   OpExtInst %int %5 SSign %41
         %42 =   OpIEqual %bool %36 %int_n1
                 OpSelectionMerge %44 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
         %46 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =       OpLoad %v4float %46            ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %47 %47 0 1   ; RelaxedPrecision
         %49 =       OpCompositeExtract %float %48 0        ; RelaxedPrecision
         %50 =       OpConvertFToS %int %49
         %51 =       OpCompositeExtract %float %48 1    ; RelaxedPrecision
         %52 =       OpConvertFToS %int %51
         %54 =       OpCompositeConstruct %v2int %50 %52
         %45 =       OpExtInst %v2int %5 SSign %54
         %55 =       OpVectorShuffle %v2int %33 %33 0 1
         %56 =       OpIEqual %v2bool %45 %55
         %58 =       OpAll %bool %56
                     OpBranch %44

         %44 = OpLabel
         %59 =   OpPhi %bool %false %26 %58 %43
                 OpSelectionMerge %61 None
                 OpBranchConditional %59 %60 %61

         %60 =     OpLabel
         %63 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %64 =       OpLoad %v4float %63            ; RelaxedPrecision
         %65 =       OpVectorShuffle %v3float %64 %64 0 1 2     ; RelaxedPrecision
         %67 =       OpCompositeExtract %float %65 0            ; RelaxedPrecision
         %68 =       OpConvertFToS %int %67
         %69 =       OpCompositeExtract %float %65 1    ; RelaxedPrecision
         %70 =       OpConvertFToS %int %69
         %71 =       OpCompositeExtract %float %65 2    ; RelaxedPrecision
         %72 =       OpConvertFToS %int %71
         %74 =       OpCompositeConstruct %v3int %68 %70 %72
         %62 =       OpExtInst %v3int %5 SSign %74
         %75 =       OpVectorShuffle %v3int %33 %33 0 1 2
         %76 =       OpIEqual %v3bool %62 %75
         %78 =       OpAll %bool %76
                     OpBranch %61

         %61 = OpLabel
         %79 =   OpPhi %bool %false %44 %78 %60
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %80 %81

         %80 =     OpLabel
         %83 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %84 =       OpLoad %v4float %83            ; RelaxedPrecision
         %85 =       OpCompositeExtract %float %84 0    ; RelaxedPrecision
         %86 =       OpConvertFToS %int %85
         %87 =       OpCompositeExtract %float %84 1    ; RelaxedPrecision
         %88 =       OpConvertFToS %int %87
         %89 =       OpCompositeExtract %float %84 2    ; RelaxedPrecision
         %90 =       OpConvertFToS %int %89
         %91 =       OpCompositeExtract %float %84 3    ; RelaxedPrecision
         %92 =       OpConvertFToS %int %91
         %93 =       OpCompositeConstruct %v4int %86 %88 %90 %92
         %82 =       OpExtInst %v4int %5 SSign %93
         %94 =       OpIEqual %v4bool %82 %33
         %96 =       OpAll %bool %94
                     OpBranch %81

         %81 = OpLabel
         %97 =   OpPhi %bool %false %61 %96 %80
                 OpSelectionMerge %99 None
                 OpBranchConditional %97 %98 %99

         %98 =     OpLabel
                     OpBranch %99

         %99 = OpLabel
        %101 =   OpPhi %bool %false %81 %true %98
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %105 =       OpVectorShuffle %v2int %33 %33 0 1
        %106 =       OpIEqual %v2bool %104 %105
        %107 =       OpAll %bool %106
                     OpBranch %103

        %103 = OpLabel
        %108 =   OpPhi %bool %false %99 %107 %102
                 OpSelectionMerge %110 None
                 OpBranchConditional %108 %109 %110

        %109 =     OpLabel
        %112 =       OpVectorShuffle %v3int %33 %33 0 1 2
        %113 =       OpIEqual %v3bool %111 %112
        %114 =       OpAll %bool %113
                     OpBranch %110

        %110 = OpLabel
        %115 =   OpPhi %bool %false %103 %114 %109
                 OpSelectionMerge %117 None
                 OpBranchConditional %115 %116 %117

        %116 =     OpLabel
                     OpBranch %117

        %117 = OpLabel
        %118 =   OpPhi %bool %false %110 %true %116
                 OpSelectionMerge %123 None
                 OpBranchConditional %118 %121 %122

        %121 =     OpLabel
        %124 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %125 =       OpLoad %v4float %124           ; RelaxedPrecision
                     OpStore %119 %125
                     OpBranch %123

        %122 =     OpLabel
        %126 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %128 =       OpLoad %v4float %126           ; RelaxedPrecision
                     OpStore %119 %128
                     OpBranch %123

        %123 = OpLabel
        %129 =   OpLoad %v4float %119               ; RelaxedPrecision
                 OpReturnValue %129
               OpFunctionEnd
