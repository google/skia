               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %ok "ok"                          ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
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
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float


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
         %ok =   OpVariable %_ptr_Function_bool Function
        %115 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
                 OpSelectionMerge %33 None
                 OpBranchConditional %true %32 %33

         %32 =     OpLabel
         %34 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %37 =       OpLoad %v4float %34            ; RelaxedPrecision
         %38 =       OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %40 =       OpFOrdEqual %bool %38 %float_1
                     OpBranch %33

         %33 = OpLabel
         %41 =   OpPhi %bool %false %26 %40 %32
                 OpStore %ok %41
                 OpSelectionMerge %43 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
         %44 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %45 =       OpLoad %v4float %44            ; RelaxedPrecision
         %46 =       OpCompositeExtract %float %45 0    ; RelaxedPrecision
         %47 =       OpFUnordNotEqual %bool %46 %float_1
                     OpBranch %43

         %43 = OpLabel
         %48 =   OpPhi %bool %false %33 %47 %42
                 OpStore %ok %48
                 OpSelectionMerge %50 None
                 OpBranchConditional %48 %49 %50

         %49 =     OpLabel
         %51 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %52 =       OpLoad %v4float %51            ; RelaxedPrecision
         %53 =       OpVectorShuffle %v2float %52 %52 1 0   ; RelaxedPrecision
         %54 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %56 =       OpLoad %v4float %54            ; RelaxedPrecision
         %57 =       OpVectorShuffle %v2float %56 %56 0 1   ; RelaxedPrecision
         %58 =       OpFOrdEqual %v2bool %53 %57
         %60 =       OpAll %bool %58
                     OpBranch %50

         %50 = OpLabel
         %61 =   OpPhi %bool %false %43 %60 %49
                 OpStore %ok %61
                 OpSelectionMerge %63 None
                 OpBranchConditional %61 %62 %63

         %62 =     OpLabel
         %64 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %65 =       OpLoad %v4float %64            ; RelaxedPrecision
         %66 =       OpVectorShuffle %v2float %65 %65 1 0   ; RelaxedPrecision
         %67 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
         %69 =       OpVectorShuffle %v2float %68 %68 0 1   ; RelaxedPrecision
         %70 =       OpFOrdEqual %v2bool %66 %69
         %71 =       OpAll %bool %70
                     OpBranch %63

         %63 = OpLabel
         %72 =   OpPhi %bool %false %50 %71 %62
                 OpStore %ok %72
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %75 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %76 =       OpLoad %v4float %75            ; RelaxedPrecision
         %77 =       OpVectorShuffle %v2float %76 %76 1 0   ; RelaxedPrecision
         %78 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %79 =       OpLoad %v4float %78            ; RelaxedPrecision
         %80 =       OpVectorShuffle %v2float %79 %79 0 1   ; RelaxedPrecision
         %81 =       OpFOrdEqual %v2bool %77 %80
         %82 =       OpAll %bool %81
                     OpSelectionMerge %84 None
                     OpBranchConditional %82 %84 %83

         %83 =         OpLabel
         %85 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %86 =           OpLoad %v4float %85        ; RelaxedPrecision
         %87 =           OpCompositeExtract %float %86 3    ; RelaxedPrecision
         %88 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %89 =           OpLoad %v4float %88        ; RelaxedPrecision
         %90 =           OpCompositeExtract %float %89 3    ; RelaxedPrecision
         %91 =           OpFUnordNotEqual %bool %87 %90
                         OpBranch %84

         %84 =     OpLabel
         %92 =       OpPhi %bool %true %73 %91 %83
                     OpBranch %74

         %74 = OpLabel
         %93 =   OpPhi %bool %false %63 %92 %84
                 OpStore %ok %93
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %94 %95

         %94 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
         %98 =       OpVectorShuffle %v2float %97 %97 1 0   ; RelaxedPrecision
         %99 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %100 =       OpLoad %v4float %99            ; RelaxedPrecision
        %101 =       OpVectorShuffle %v2float %100 %100 0 1     ; RelaxedPrecision
        %102 =       OpFUnordNotEqual %v2bool %98 %101
        %103 =       OpAny %bool %102
                     OpSelectionMerge %105 None
                     OpBranchConditional %103 %104 %105

        %104 =         OpLabel
        %106 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %107 =           OpLoad %v4float %106       ; RelaxedPrecision
        %108 =           OpCompositeExtract %float %107 3   ; RelaxedPrecision
        %109 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %110 =           OpLoad %v4float %109       ; RelaxedPrecision
        %111 =           OpCompositeExtract %float %110 3   ; RelaxedPrecision
        %112 =           OpFOrdEqual %bool %108 %111
                         OpBranch %105

        %105 =     OpLabel
        %113 =       OpPhi %bool %false %94 %112 %104
                     OpBranch %95

         %95 = OpLabel
        %114 =   OpPhi %bool %false %74 %113 %105
                 OpStore %ok %114
                 OpSelectionMerge %119 None
                 OpBranchConditional %114 %117 %118

        %117 =     OpLabel
        %120 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %121 =       OpLoad %v4float %120           ; RelaxedPrecision
                     OpStore %115 %121
                     OpBranch %119

        %118 =     OpLabel
        %122 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %123 =       OpLoad %v4float %122           ; RelaxedPrecision
                     OpStore %115 %123
                     OpBranch %119

        %119 = OpLabel
        %124 =   OpLoad %v4float %115               ; RelaxedPrecision
                 OpReturnValue %124
               OpFunctionEnd
