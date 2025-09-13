               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %f4 "f4"                          ; id %28
               OpName %ok "ok"                          ; id %39

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
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %136 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
    %float_4 = OpConstant %float 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %54 = OpConstantComposite %v2float %float_1 %float_2
         %55 = OpConstantComposite %v2float %float_3 %float_4
         %56 = OpConstantComposite %mat2v2float %54 %55
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
         %77 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %78 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %79 = OpConstantComposite %v3float %float_3 %float_4 %float_1
         %80 = OpConstantComposite %mat3v3float %77 %78 %79
     %v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
        %114 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %115 = OpConstantComposite %mat4v4float %114 %114 %114 %114
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


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
         %f4 =   OpVariable %_ptr_Function_v4float Function
         %ok =   OpVariable %_ptr_Function_bool Function
        %129 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %33 =   OpLoad %mat2v2float %30
         %34 =   OpCompositeExtract %float %33 0 0
         %35 =   OpCompositeExtract %float %33 0 1
         %36 =   OpCompositeExtract %float %33 1 0
         %37 =   OpCompositeExtract %float %33 1 1
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37
                 OpStore %f4 %38
         %42 =   OpVectorShuffle %v3float %38 %38 0 1 2
         %45 =   OpCompositeExtract %float %42 0
         %46 =   OpCompositeExtract %float %42 1
         %47 =   OpCompositeConstruct %v2float %45 %46
         %48 =   OpCompositeExtract %float %42 2
         %49 =   OpCompositeConstruct %v2float %48 %float_4
         %50 =   OpCompositeConstruct %mat2v2float %47 %49
         %58 =   OpFOrdEqual %v2bool %47 %54
         %59 =   OpAll %bool %58
         %60 =   OpFOrdEqual %v2bool %49 %55
         %61 =   OpAll %bool %60
         %62 =   OpLogicalAnd %bool %59 %61
                 OpStore %ok %62
                 OpSelectionMerge %65 None
                 OpBranchConditional %62 %64 %65

         %64 =     OpLabel
         %66 =       OpVectorShuffle %v2float %38 %38 0 1
         %67 =       OpVectorShuffle %v2float %38 %38 2 3
         %68 =       OpCompositeExtract %float %66 0
         %69 =       OpCompositeExtract %float %66 1
         %70 =       OpCompositeExtract %float %67 0
         %71 =       OpCompositeConstruct %v3float %68 %69 %70
         %72 =       OpCompositeExtract %float %67 1
         %73 =       OpCompositeConstruct %v3float %72 %34 %35
         %74 =       OpCompositeConstruct %v3float %36 %37 %34
         %76 =       OpCompositeConstruct %mat3v3float %71 %73 %74
         %82 =       OpFOrdEqual %v3bool %71 %77
         %83 =       OpAll %bool %82
         %84 =       OpFOrdEqual %v3bool %73 %78
         %85 =       OpAll %bool %84
         %86 =       OpLogicalAnd %bool %83 %85
         %87 =       OpFOrdEqual %v3bool %74 %79
         %88 =       OpAll %bool %87
         %89 =       OpLogicalAnd %bool %86 %88
                     OpBranch %65

         %65 = OpLabel
         %90 =   OpPhi %bool %false %27 %89 %64
                 OpStore %ok %90
                 OpSelectionMerge %92 None
                 OpBranchConditional %90 %91 %92

         %91 =     OpLabel
         %93 =       OpVectorShuffle %v3float %38 %38 0 1 2
         %94 =       OpVectorShuffle %v3float %38 %38 3 0 1
         %95 =       OpVectorShuffle %v4float %38 %38 2 3 0 1
         %96 =       OpVectorShuffle %v2float %38 %38 2 3
         %97 =       OpCompositeExtract %float %93 0
         %98 =       OpCompositeExtract %float %93 1
         %99 =       OpCompositeExtract %float %93 2
        %100 =       OpCompositeExtract %float %94 0
        %101 =       OpCompositeConstruct %v4float %97 %98 %99 %100
        %102 =       OpCompositeExtract %float %94 1
        %103 =       OpCompositeExtract %float %94 2
        %104 =       OpCompositeExtract %float %95 0
        %105 =       OpCompositeExtract %float %95 1
        %106 =       OpCompositeConstruct %v4float %102 %103 %104 %105
        %107 =       OpCompositeExtract %float %95 2
        %108 =       OpCompositeExtract %float %95 3
        %109 =       OpCompositeExtract %float %96 0
        %110 =       OpCompositeExtract %float %96 1
        %111 =       OpCompositeConstruct %v4float %107 %108 %109 %110
        %113 =       OpCompositeConstruct %mat4v4float %101 %106 %111 %38
        %117 =       OpFOrdEqual %v4bool %101 %114
        %118 =       OpAll %bool %117
        %119 =       OpFOrdEqual %v4bool %106 %114
        %120 =       OpAll %bool %119
        %121 =       OpLogicalAnd %bool %118 %120
        %122 =       OpFOrdEqual %v4bool %111 %114
        %123 =       OpAll %bool %122
        %124 =       OpLogicalAnd %bool %121 %123
        %125 =       OpFOrdEqual %v4bool %38 %114
        %126 =       OpAll %bool %125
        %127 =       OpLogicalAnd %bool %124 %126
                     OpBranch %92

         %92 = OpLabel
        %128 =   OpPhi %bool %false %65 %127 %91
                 OpStore %ok %128
                 OpSelectionMerge %132 None
                 OpBranchConditional %128 %130 %131

        %130 =     OpLabel
        %133 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %136 =       OpLoad %v4float %133           ; RelaxedPrecision
                     OpStore %129 %136
                     OpBranch %132

        %131 =     OpLabel
        %137 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %139 =       OpLoad %v4float %137           ; RelaxedPrecision
                     OpStore %129 %139
                     OpBranch %132

        %132 = OpLabel
        %140 =   OpLoad %v4float %129               ; RelaxedPrecision
                 OpReturnValue %140
               OpFunctionEnd
