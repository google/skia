               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %InnerLUT "InnerLUT"
               OpMemberName %InnerLUT 0 "values"
               OpName %OuterLUT "OuterLUT"
               OpMemberName %OuterLUT 0 "inner"
               OpName %Root "Root"
               OpMemberName %Root 0 "outer"
               OpName %data "data"
               OpName %expected "expected"
               OpName %i "i"
               OpName %j "j"
               OpName %k "k"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpMemberDecorate %InnerLUT 0 Offset 0
               OpDecorate %_arr_InnerLUT_int_3 ArrayStride 16
               OpMemberDecorate %OuterLUT 0 Offset 0
               OpMemberDecorate %OuterLUT 0 RelaxedPrecision
               OpDecorate %_arr_OuterLUT_int_3 ArrayStride 48
               OpMemberDecorate %Root 0 Offset 0
               OpMemberDecorate %Root 0 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
   %InnerLUT = OpTypeStruct %v3float
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
%_arr_InnerLUT_int_3 = OpTypeArray %InnerLUT %int_3
   %OuterLUT = OpTypeStruct %_arr_InnerLUT_int_3
%_arr_OuterLUT_int_3 = OpTypeArray %OuterLUT %int_3
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
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
       %data = OpVariable %_ptr_Function_Root Function
   %expected = OpVariable %_ptr_Function_v3float Function
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
          %k = OpVariable %_ptr_Function_int Function
         %41 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_0 %int_0
               OpStore %41 %39
         %48 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_1 %int_0
               OpStore %48 %46
         %54 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_0 %int_0 %int_2 %int_0
               OpStore %54 %52
         %59 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_0 %int_0
               OpStore %59 %58
         %64 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_1 %int_0
               OpStore %64 %63
         %69 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_1 %int_0 %int_2 %int_0
               OpStore %69 %68
         %74 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_0 %int_0
               OpStore %74 %73
         %79 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_1 %int_0
               OpStore %79 %78
         %84 = OpAccessChain %_ptr_Function_v3float %data %int_0 %int_2 %int_0 %int_2 %int_0
               OpStore %84 %83
               OpStore %expected %86
               OpStore %i %int_0
               OpBranch %89
         %89 = OpLabel
               OpLoopMerge %93 %92 None
               OpBranch %90
         %90 = OpLabel
         %94 = OpLoad %int %i
         %95 = OpSLessThan %bool %94 %int_3
               OpBranchConditional %95 %91 %93
         %91 = OpLabel
               OpStore %j %int_0
               OpBranch %97
         %97 = OpLabel
               OpLoopMerge %101 %100 None
               OpBranch %98
         %98 = OpLabel
        %102 = OpLoad %int %j
        %103 = OpSLessThan %bool %102 %int_3
               OpBranchConditional %103 %99 %101
         %99 = OpLabel
        %104 = OpLoad %v3float %expected
        %105 = OpFAdd %v3float %104 %39
               OpStore %expected %105
        %106 = OpLoad %int %i
        %107 = OpLoad %int %j
        %108 = OpAccessChain %_ptr_Function_v3float %data %int_0 %106 %int_0 %107 %int_0
        %109 = OpLoad %v3float %108
        %110 = OpFUnordNotEqual %v3bool %109 %105
        %112 = OpAny %bool %110
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %117 = OpLoad %v4float %115
               OpReturnValue %117
        %114 = OpLabel
               OpStore %k %int_0
               OpBranch %119
        %119 = OpLabel
               OpLoopMerge %123 %122 None
               OpBranch %120
        %120 = OpLabel
        %124 = OpLoad %int %k
        %125 = OpSLessThan %bool %124 %int_3
               OpBranchConditional %125 %121 %123
        %121 = OpLabel
        %126 = OpLoad %int %i
        %127 = OpLoad %int %j
        %128 = OpAccessChain %_ptr_Function_v3float %data %int_0 %126 %int_0 %127 %int_0
        %129 = OpLoad %v3float %128
        %130 = OpLoad %int %k
        %131 = OpVectorExtractDynamic %float %129 %130
        %132 = OpLoad %v3float %expected
        %133 = OpLoad %int %k
        %134 = OpVectorExtractDynamic %float %132 %133
        %135 = OpFUnordNotEqual %bool %131 %134
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %139 = OpLoad %v4float %138
               OpReturnValue %139
        %137 = OpLabel
               OpBranch %122
        %122 = OpLabel
        %140 = OpLoad %int %k
        %141 = OpIAdd %int %140 %int_1
               OpStore %k %141
               OpBranch %119
        %123 = OpLabel
               OpBranch %100
        %100 = OpLabel
        %142 = OpLoad %int %j
        %143 = OpIAdd %int %142 %int_1
               OpStore %j %143
               OpBranch %97
        %101 = OpLabel
               OpBranch %92
         %92 = OpLabel
        %144 = OpLoad %int %i
        %145 = OpIAdd %int %144 %int_1
               OpStore %i %145
               OpBranch %89
         %93 = OpLabel
        %146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %147 = OpLoad %v4float %146
               OpReturnValue %147
               OpFunctionEnd
