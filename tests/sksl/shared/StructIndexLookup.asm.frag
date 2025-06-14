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
               OpName %InnerLUT "InnerLUT"              ; id %29
               OpMemberName %InnerLUT 0 "values"
               OpName %OuterLUT "OuterLUT"          ; id %32
               OpMemberName %OuterLUT 0 "inner"
               OpName %Root "Root"                  ; id %34
               OpMemberName %Root 0 "outer"
               OpName %data "data"                  ; id %27
               OpName %expected "expected"          ; id %85
               OpName %i "i"                        ; id %87
               OpName %j "j"                        ; id %97
               OpName %k "k"                        ; id %119

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
               OpMemberDecorate %InnerLUT 0 Offset 0
               OpDecorate %_arr_InnerLUT_int_3 ArrayStride 16
               OpMemberDecorate %OuterLUT 0 Offset 0
               OpMemberDecorate %OuterLUT 0 RelaxedPrecision
               OpDecorate %_arr_OuterLUT_int_3 ArrayStride 48
               OpMemberDecorate %Root 0 Offset 0
               OpMemberDecorate %Root 0 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision

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
    %v3float = OpTypeVector %float 3
   %InnerLUT = OpTypeStruct %v3float
      %int_3 = OpConstant %int 3
%_arr_InnerLUT_int_3 = OpTypeArray %InnerLUT %int_3     ; ArrayStride 16
   %OuterLUT = OpTypeStruct %_arr_InnerLUT_int_3
%_arr_OuterLUT_int_3 = OpTypeArray %OuterLUT %int_3     ; ArrayStride 48
       %Root = OpTypeStruct %_arr_OuterLUT_int_3
%_ptr_Function_Root = OpTypePointer Function %Root
    %float_1 = OpConstant %float 1
   %float_10 = OpConstant %float 10
  %float_100 = OpConstant %float 100
         %39 = OpConstantComposite %v3float %float_1 %float_10 %float_100
      %int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_2 = OpConstant %float 2
   %float_20 = OpConstant %float 20
  %float_200 = OpConstant %float 200
         %46 = OpConstantComposite %v3float %float_2 %float_20 %float_200
      %int_1 = OpConstant %int 1
    %float_3 = OpConstant %float 3
   %float_30 = OpConstant %float 30
  %float_300 = OpConstant %float 300
         %52 = OpConstantComposite %v3float %float_3 %float_30 %float_300
      %int_2 = OpConstant %int 2
    %float_4 = OpConstant %float 4
   %float_40 = OpConstant %float 40
  %float_400 = OpConstant %float 400
         %58 = OpConstantComposite %v3float %float_4 %float_40 %float_400
    %float_5 = OpConstant %float 5
   %float_50 = OpConstant %float 50
  %float_500 = OpConstant %float 500
         %63 = OpConstantComposite %v3float %float_5 %float_50 %float_500
    %float_6 = OpConstant %float 6
   %float_60 = OpConstant %float 60
  %float_600 = OpConstant %float 600
         %68 = OpConstantComposite %v3float %float_6 %float_60 %float_600
    %float_7 = OpConstant %float 7
   %float_70 = OpConstant %float 70
  %float_700 = OpConstant %float 700
         %73 = OpConstantComposite %v3float %float_7 %float_70 %float_700
    %float_8 = OpConstant %float 8
   %float_80 = OpConstant %float 80
  %float_800 = OpConstant %float 800
         %78 = OpConstantComposite %v3float %float_8 %float_80 %float_800
    %float_9 = OpConstant %float 9
   %float_90 = OpConstant %float 90
  %float_900 = OpConstant %float 900
         %83 = OpConstantComposite %v3float %float_9 %float_90 %float_900
         %86 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_int = OpTypePointer Function %int
       %bool = OpTypeBool
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


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
       %data =   OpVariable %_ptr_Function_Root Function
   %expected =   OpVariable %_ptr_Function_v3float Function
          %i =   OpVariable %_ptr_Function_int Function
          %j =   OpVariable %_ptr_Function_int Function
          %k =   OpVariable %_ptr_Function_int Function
         %41 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_0 %int_0
                 OpStore %41 %39
         %48 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_1 %int_0
                 OpStore %48 %46
         %54 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_2 %int_0
                 OpStore %54 %52
         %59 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_0 %int_0
                 OpStore %59 %58
         %64 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_1 %int_0
                 OpStore %64 %63
         %69 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_2 %int_0
                 OpStore %69 %68
         %74 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_0 %int_0
                 OpStore %74 %73
         %79 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_1 %int_0
                 OpStore %79 %78
         %84 =   OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_2 %int_0
                 OpStore %84 %83
                 OpStore %expected %86
                 OpStore %i %int_0
                 OpBranch %89

         %89 = OpLabel
                 OpLoopMerge %93 %92 None
                 OpBranch %90

         %90 =     OpLabel
         %94 =       OpLoad %int %i
         %95 =       OpSLessThan %bool %94 %int_3
                     OpBranchConditional %95 %91 %93

         %91 =         OpLabel
                         OpStore %j %int_0
                         OpBranch %98

         %98 =         OpLabel
                         OpLoopMerge %102 %101 None
                         OpBranch %99

         %99 =             OpLabel
        %103 =               OpLoad %int %j
        %104 =               OpSLessThan %bool %103 %int_3
                             OpBranchConditional %104 %100 %102

        %100 =                 OpLabel
        %105 =                   OpLoad %v3float %expected
        %106 =                   OpFAdd %v3float %105 %39
                                 OpStore %expected %106
        %107 =                   OpLoad %int %i
        %108 =                   OpLoad %int %j
        %109 =                   OpAccessChain %_ptr_Function_v3float %data %int_0 %107 %int_0 %108 %int_0
        %110 =                   OpLoad %v3float %109
        %111 =                   OpFUnordNotEqual %v3bool %110 %106
        %113 =                   OpAny %bool %111
                                 OpSelectionMerge %115 None
                                 OpBranchConditional %113 %114 %115

        %114 =                     OpLabel
        %116 =                       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %118 =                       OpLoad %v4float %116   ; RelaxedPrecision
                                     OpReturnValue %118

        %115 =                 OpLabel
                                 OpStore %k %int_0
                                 OpBranch %120

        %120 =                 OpLabel
                                 OpLoopMerge %124 %123 None
                                 OpBranch %121

        %121 =                     OpLabel
        %125 =                       OpLoad %int %k
        %126 =                       OpSLessThan %bool %125 %int_3
                                     OpBranchConditional %126 %122 %124

        %122 =                         OpLabel
        %127 =                           OpLoad %int %i
        %128 =                           OpLoad %int %j
        %129 =                           OpAccessChain %_ptr_Function_v3float %data %int_0 %127 %int_0 %128 %int_0
        %130 =                           OpLoad %v3float %129
        %131 =                           OpLoad %int %k
        %132 =                           OpVectorExtractDynamic %float %130 %131
        %133 =                           OpLoad %v3float %expected
        %134 =                           OpLoad %int %k
        %135 =                           OpVectorExtractDynamic %float %133 %134
        %136 =                           OpFUnordNotEqual %bool %132 %135
                                         OpSelectionMerge %138 None
                                         OpBranchConditional %136 %137 %138

        %137 =                             OpLabel
        %139 =                               OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %140 =                               OpLoad %v4float %139   ; RelaxedPrecision
                                             OpReturnValue %140

        %138 =                         OpLabel
                                         OpBranch %123

        %123 =                   OpLabel
        %141 =                     OpLoad %int %k
        %142 =                     OpIAdd %int %141 %int_1
                                   OpStore %k %142
                                   OpBranch %120

        %124 =                 OpLabel
                                 OpBranch %101

        %101 =           OpLabel
        %143 =             OpLoad %int %j
        %144 =             OpIAdd %int %143 %int_1
                           OpStore %j %144
                           OpBranch %98

        %102 =         OpLabel
                         OpBranch %92

         %92 =   OpLabel
        %145 =     OpLoad %int %i
        %146 =     OpIAdd %int %145 %int_1
                   OpStore %i %146
                   OpBranch %89

         %93 = OpLabel
        %147 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %148 =   OpLoad %v4float %147               ; RelaxedPrecision
                 OpReturnValue %148
               OpFunctionEnd
