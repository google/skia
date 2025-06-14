               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "expected"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6

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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
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
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
         %95 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %104 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


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
        %110 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %29 =   OpExtInst %float %5 Tan %34        ; RelaxedPrecision
         %35 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %37 =   OpLoad %v4float %35                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =   OpFOrdEqual %bool %29 %38
                 OpSelectionMerge %41 None
                 OpBranchConditional %39 %40 %41

         %40 =     OpLabel
         %43 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =       OpLoad %v4float %43            ; RelaxedPrecision
         %45 =       OpVectorShuffle %v2float %44 %44 0 1   ; RelaxedPrecision
         %42 =       OpExtInst %v2float %5 Tan %45          ; RelaxedPrecision
         %46 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %47 =       OpLoad %v4float %46            ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %47 %47 0 1   ; RelaxedPrecision
         %49 =       OpFOrdEqual %v2bool %42 %48
         %51 =       OpAll %bool %49
                     OpBranch %41

         %41 = OpLabel
         %52 =   OpPhi %bool %false %26 %51 %40
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %56 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 =       OpLoad %v4float %56            ; RelaxedPrecision
         %58 =       OpVectorShuffle %v3float %57 %57 0 1 2     ; RelaxedPrecision
         %55 =       OpExtInst %v3float %5 Tan %58              ; RelaxedPrecision
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =       OpLoad %v4float %60            ; RelaxedPrecision
         %62 =       OpVectorShuffle %v3float %61 %61 0 1 2     ; RelaxedPrecision
         %63 =       OpFOrdEqual %v3bool %55 %62
         %65 =       OpAll %bool %63
                     OpBranch %54

         %54 = OpLabel
         %66 =   OpPhi %bool %false %41 %65 %53
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %69 =       OpExtInst %v4float %5 Tan %71  ; RelaxedPrecision
         %72 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %73 =       OpLoad %v4float %72            ; RelaxedPrecision
         %74 =       OpFOrdEqual %v4bool %69 %73
         %76 =       OpAll %bool %74
                     OpBranch %68

         %68 = OpLabel
         %77 =   OpPhi %bool %false %54 %76 %67
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
         %80 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %81 =       OpLoad %v4float %80            ; RelaxedPrecision
         %82 =       OpCompositeExtract %float %81 0    ; RelaxedPrecision
         %83 =       OpFOrdEqual %bool %float_0 %82
                     OpBranch %79

         %79 = OpLabel
         %84 =   OpPhi %bool %false %68 %83 %78
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
         %89 =       OpVectorShuffle %v2float %88 %88 0 1   ; RelaxedPrecision
         %90 =       OpFOrdEqual %v2bool %20 %89
         %91 =       OpAll %bool %90
                     OpBranch %86

         %86 = OpLabel
         %92 =   OpPhi %bool %false %79 %91 %85
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
         %98 =       OpVectorShuffle %v3float %97 %97 0 1 2     ; RelaxedPrecision
         %99 =       OpFOrdEqual %v3bool %95 %98
        %100 =       OpAll %bool %99
                     OpBranch %94

         %94 = OpLabel
        %101 =   OpPhi %bool %false %86 %100 %93
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %105 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %106 =       OpLoad %v4float %105           ; RelaxedPrecision
        %107 =       OpFOrdEqual %v4bool %104 %106
        %108 =       OpAll %bool %107
                     OpBranch %103

        %103 = OpLabel
        %109 =   OpPhi %bool %false %94 %108 %102
                 OpSelectionMerge %114 None
                 OpBranchConditional %109 %112 %113

        %112 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %117 =       OpLoad %v4float %115           ; RelaxedPrecision
                     OpStore %110 %117
                     OpBranch %114

        %113 =     OpLabel
        %118 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %120 =       OpLoad %v4float %118           ; RelaxedPrecision
                     OpStore %110 %120
                     OpBranch %114

        %114 = OpLabel
        %121 =   OpLoad %v4float %110               ; RelaxedPrecision
                 OpReturnValue %121
               OpFunctionEnd
