               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %InnerLUT "InnerLUT"              ; id %25
               OpMemberName %InnerLUT 0 "values"
               OpName %OuterLUT "OuterLUT"          ; id %29
               OpMemberName %OuterLUT 0 "inner"
               OpName %Root "Root"                  ; id %31
               OpMemberName %Root 0 "outer"
               OpName %data "data"                  ; id %23
               OpName %expected "expected"          ; id %82
               OpName %i "i"                        ; id %84
               OpName %j "j"                        ; id %94
               OpName %k "k"                        ; id %116

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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpMemberDecorate %InnerLUT 0 Offset 0
               OpDecorate %_arr_InnerLUT_int_3 ArrayStride 16
               OpMemberDecorate %OuterLUT 0 Offset 0
               OpMemberDecorate %OuterLUT 0 RelaxedPrecision
               OpDecorate %_arr_OuterLUT_int_3 ArrayStride 48
               OpMemberDecorate %Root 0 Offset 0
               OpMemberDecorate %Root 0 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
   %InnerLUT = OpTypeStruct %v3float
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
%_arr_InnerLUT_int_3 = OpTypeArray %InnerLUT %int_3     ; ArrayStride 16
   %OuterLUT = OpTypeStruct %_arr_InnerLUT_int_3
%_arr_OuterLUT_int_3 = OpTypeArray %OuterLUT %int_3     ; ArrayStride 48
       %Root = OpTypeStruct %_arr_OuterLUT_int_3
%_ptr_Function_Root = OpTypePointer Function %Root
    %float_1 = OpConstant %float 1
   %float_10 = OpConstant %float 10
  %float_100 = OpConstant %float 100
         %36 = OpConstantComposite %v3float %float_1 %float_10 %float_100
      %int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_2 = OpConstant %float 2
   %float_20 = OpConstant %float 20
  %float_200 = OpConstant %float 200
         %43 = OpConstantComposite %v3float %float_2 %float_20 %float_200
      %int_1 = OpConstant %int 1
    %float_3 = OpConstant %float 3
   %float_30 = OpConstant %float 30
  %float_300 = OpConstant %float 300
         %49 = OpConstantComposite %v3float %float_3 %float_30 %float_300
      %int_2 = OpConstant %int 2
    %float_4 = OpConstant %float 4
   %float_40 = OpConstant %float 40
  %float_400 = OpConstant %float 400
         %55 = OpConstantComposite %v3float %float_4 %float_40 %float_400
    %float_5 = OpConstant %float 5
   %float_50 = OpConstant %float 50
  %float_500 = OpConstant %float 500
         %60 = OpConstantComposite %v3float %float_5 %float_50 %float_500
    %float_6 = OpConstant %float 6
   %float_60 = OpConstant %float 60
  %float_600 = OpConstant %float 600
         %65 = OpConstantComposite %v3float %float_6 %float_60 %float_600
    %float_7 = OpConstant %float 7
   %float_70 = OpConstant %float 70
  %float_700 = OpConstant %float 700
         %70 = OpConstantComposite %v3float %float_7 %float_70 %float_700
    %float_8 = OpConstant %float 8
   %float_80 = OpConstant %float 80
  %float_800 = OpConstant %float 800
         %75 = OpConstantComposite %v3float %float_8 %float_80 %float_800
    %float_9 = OpConstant %float 9
   %float_90 = OpConstant %float 90
  %float_900 = OpConstant %float 900
         %80 = OpConstantComposite %v3float %float_9 %float_90 %float_900
         %83 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_int = OpTypePointer Function %int
       %bool = OpTypeBool
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
       %data =   OpVariable %_ptr_Function_Root Function
   %expected =   OpVariable %_ptr_Function_v3float Function
          %i =   OpVariable %_ptr_Function_int Function
          %j =   OpVariable %_ptr_Function_int Function
          %k =   OpVariable %_ptr_Function_int Function
         %38 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_0 %int_0
                 OpStore %38 %36
         %45 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_1 %int_0
                 OpStore %45 %43
         %51 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_2 %int_0
                 OpStore %51 %49
         %56 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_0 %int_0
                 OpStore %56 %55
         %61 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_1 %int_0
                 OpStore %61 %60
         %66 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_2 %int_0
                 OpStore %66 %65
         %71 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_0 %int_0
                 OpStore %71 %70
         %76 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_1 %int_0
                 OpStore %76 %75
         %81 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_2 %int_0
                 OpStore %81 %80
                 OpStore %expected %83
                 OpStore %i %int_0
                 OpBranch %86

         %86 = OpLabel
                 OpLoopMerge %90 %89 None
                 OpBranch %87

         %87 =     OpLabel
         %91 =       OpLoad %int %i
         %92 =       OpSLessThan %bool %91 %int_3
                     OpBranchConditional %92 %88 %90

         %88 =         OpLabel
                         OpStore %j %int_0
                         OpBranch %95

         %95 =         OpLabel
                         OpLoopMerge %99 %98 None
                         OpBranch %96

         %96 =             OpLabel
        %100 =               OpLoad %int %j
        %101 =               OpSLessThan %bool %100 %int_3
                             OpBranchConditional %101 %97 %99

         %97 =                 OpLabel
        %102 =                   OpLoad %v3float %expected
        %103 =                   OpFAdd %v3float %102 %36
                                 OpStore %expected %103
        %104 =                   OpLoad %int %i
        %105 =                   OpLoad %int %j
        %106 =                   OpAccessChain %_ptr_Function_v3float %data %int_0 %104 %int_0 %105 %int_0
        %107 =                   OpLoad %v3float %106
        %108 =                   OpFUnordNotEqual %v3bool %107 %103
        %110 =                   OpAny %bool %108
                                 OpSelectionMerge %112 None
                                 OpBranchConditional %110 %111 %112

        %111 =                     OpLabel
        %113 =                       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %115 =                       OpLoad %v4float %113   ; RelaxedPrecision
                                     OpReturnValue %115

        %112 =                 OpLabel
                                 OpStore %k %int_0
                                 OpBranch %117

        %117 =                 OpLabel
                                 OpLoopMerge %121 %120 None
                                 OpBranch %118

        %118 =                     OpLabel
        %122 =                       OpLoad %int %k
        %123 =                       OpSLessThan %bool %122 %int_3
                                     OpBranchConditional %123 %119 %121

        %119 =                         OpLabel
        %124 =                           OpLoad %int %i
        %125 =                           OpLoad %int %j
        %126 =                           OpAccessChain %_ptr_Function_v3float %data %int_0 %124 %int_0 %125 %int_0
        %127 =                           OpLoad %v3float %126
        %128 =                           OpLoad %int %k
        %129 =                           OpVectorExtractDynamic %float %127 %128
        %130 =                           OpLoad %v3float %expected
        %131 =                           OpLoad %int %k
        %132 =                           OpVectorExtractDynamic %float %130 %131
        %133 =                           OpFUnordNotEqual %bool %129 %132
                                         OpSelectionMerge %135 None
                                         OpBranchConditional %133 %134 %135

        %134 =                             OpLabel
        %136 =                               OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %137 =                               OpLoad %v4float %136   ; RelaxedPrecision
                                             OpReturnValue %137

        %135 =                         OpLabel
                                         OpBranch %120

        %120 =                   OpLabel
        %138 =                     OpLoad %int %k
        %139 =                     OpIAdd %int %138 %int_1
                                   OpStore %k %139
                                   OpBranch %117

        %121 =                 OpLabel
                                 OpBranch %98

         %98 =           OpLabel
        %140 =             OpLoad %int %j
        %141 =             OpIAdd %int %140 %int_1
                           OpStore %j %141
                           OpBranch %95

         %99 =         OpLabel
                         OpBranch %89

         %89 =   OpLabel
        %142 =     OpLoad %int %i
        %143 =     OpIAdd %int %142 %int_1
                   OpStore %i %143
                   OpBranch %86

         %90 = OpLabel
        %144 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %145 =   OpLoad %v4float %144               ; RelaxedPrecision
                 OpReturnValue %145
               OpFunctionEnd
