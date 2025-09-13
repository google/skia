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
               OpName %scalar "scalar"                  ; id %27
               OpName %array "array"                    ; id %29

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
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %36 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1   ; ArrayStride 16
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_0_5 = OpConstant %float 0.5
    %float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
      %int_3 = OpConstant %int 3
    %float_4 = OpConstant %float 4
    %v3float = OpTypeVector %float 3
         %50 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %51 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %52 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
%mat3v3float = OpTypeMatrix %v3float 3
         %54 = OpConstantComposite %mat3v3float %50 %51 %52
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
         %62 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
        %119 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
     %v4bool = OpTypeVector %bool 4


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
     %scalar =   OpVariable %_ptr_Function_v4float Function
      %array =   OpVariable %_ptr_Function__arr_v4float_int_1 Function
         %72 =   OpVariable %_ptr_Function_float Function
        %108 =   OpVariable %_ptr_Function_float Function
        %130 =   OpVariable %_ptr_Function_v4float Function
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %38 =   OpVectorTimesScalar %v4float %36 %float_0_5
                 OpStore %scalar %38
         %40 =   OpAccessChain %_ptr_Function_float %scalar %int_3
                 OpStore %40 %float_2
         %43 =   OpAccessChain %_ptr_Function_float %scalar %int_1
         %44 =   OpLoad %float %43
         %46 =   OpFMul %float %44 %float_4
                 OpStore %43 %46
         %47 =   OpLoad %v4float %scalar
         %48 =   OpVectorShuffle %v3float %47 %47 1 2 3
         %55 =   OpVectorTimesMatrix %v3float %48 %54
         %56 =   OpLoad %v4float %scalar
         %57 =   OpVectorShuffle %v4float %56 %55 0 4 5 6
                 OpStore %scalar %57
         %58 =   OpLoad %v4float %scalar
         %59 =   OpVectorShuffle %v4float %58 %58 2 1 3 0
         %63 =   OpFAdd %v4float %59 %62
         %64 =   OpLoad %v4float %scalar
         %65 =   OpVectorShuffle %v4float %64 %63 7 5 4 6
                 OpStore %scalar %65
         %66 =   OpAccessChain %_ptr_Function_float %scalar %int_0
         %67 =   OpLoad %float %66
         %68 =   OpCompositeExtract %float %65 3
         %70 =   OpFOrdLessThanEqual %bool %68 %float_1
                 OpSelectionMerge %75 None
                 OpBranchConditional %70 %73 %74

         %73 =     OpLabel
         %76 =       OpCompositeExtract %float %65 2
                     OpStore %72 %76
                     OpBranch %75

         %74 =     OpLabel
                     OpStore %72 %float_0
                     OpBranch %75

         %75 = OpLabel
         %77 =   OpLoad %float %72
         %78 =   OpFAdd %float %67 %77
                 OpStore %66 %78
         %79 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %80 =   OpLoad %v4float %79                ; RelaxedPrecision
         %81 =   OpVectorTimesScalar %v4float %80 %float_0_5
         %82 =   OpAccessChain %_ptr_Function_v4float %array %int_0
                 OpStore %82 %81
         %83 =   OpAccessChain %_ptr_Function_v4float %array %int_0
         %84 =   OpAccessChain %_ptr_Function_float %83 %int_3
                 OpStore %84 %float_2
         %85 =   OpAccessChain %_ptr_Function_v4float %array %int_0
         %86 =   OpAccessChain %_ptr_Function_float %85 %int_1
         %87 =   OpLoad %float %86
         %88 =   OpFMul %float %87 %float_4
                 OpStore %86 %88
         %89 =   OpAccessChain %_ptr_Function_v4float %array %int_0
         %90 =   OpLoad %v4float %89
         %91 =   OpVectorShuffle %v3float %90 %90 1 2 3
         %92 =   OpVectorTimesMatrix %v3float %91 %54
         %93 =   OpLoad %v4float %89
         %94 =   OpVectorShuffle %v4float %93 %92 0 4 5 6
                 OpStore %89 %94
         %95 =   OpAccessChain %_ptr_Function_v4float %array %int_0
         %96 =   OpLoad %v4float %95
         %97 =   OpVectorShuffle %v4float %96 %96 2 1 3 0
         %98 =   OpFAdd %v4float %97 %62
         %99 =   OpLoad %v4float %95
        %100 =   OpVectorShuffle %v4float %99 %98 7 5 4 6
                 OpStore %95 %100
        %101 =   OpAccessChain %_ptr_Function_v4float %array %int_0
        %102 =   OpAccessChain %_ptr_Function_float %101 %int_0
        %103 =   OpLoad %float %102
        %104 =   OpAccessChain %_ptr_Function_v4float %array %int_0
        %105 =   OpLoad %v4float %104
        %106 =   OpCompositeExtract %float %105 3
        %107 =   OpFOrdLessThanEqual %bool %106 %float_1
                 OpSelectionMerge %111 None
                 OpBranchConditional %107 %109 %110

        %109 =     OpLabel
        %112 =       OpAccessChain %_ptr_Function_v4float %array %int_0
        %113 =       OpLoad %v4float %112
        %114 =       OpCompositeExtract %float %113 2
                     OpStore %108 %114
                     OpBranch %111

        %110 =     OpLabel
                     OpStore %108 %float_0
                     OpBranch %111

        %111 = OpLabel
        %115 =   OpLoad %float %108
        %116 =   OpFAdd %float %103 %115
                 OpStore %102 %116
        %118 =   OpLoad %v4float %scalar
        %120 =   OpFOrdEqual %v4bool %118 %119
        %122 =   OpAll %bool %120
                 OpSelectionMerge %124 None
                 OpBranchConditional %122 %123 %124

        %123 =     OpLabel
        %125 =       OpAccessChain %_ptr_Function_v4float %array %int_0
        %126 =       OpLoad %v4float %125
        %127 =       OpFOrdEqual %v4bool %126 %119
        %128 =       OpAll %bool %127
                     OpBranch %124

        %124 = OpLabel
        %129 =   OpPhi %bool %false %111 %128 %123
                 OpSelectionMerge %133 None
                 OpBranchConditional %129 %131 %132

        %131 =     OpLabel
        %134 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %135 =       OpLoad %v4float %134           ; RelaxedPrecision
                     OpStore %130 %135
                     OpBranch %133

        %132 =     OpLabel
        %136 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %137 =       OpLoad %v4float %136           ; RelaxedPrecision
                     OpStore %130 %137
                     OpBranch %133

        %133 = OpLabel
        %138 =   OpLoad %v4float %130               ; RelaxedPrecision
                 OpReturnValue %138
               OpFunctionEnd
