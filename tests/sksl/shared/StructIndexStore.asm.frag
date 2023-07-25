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
               OpMemberName %Root 0 "valueAtRoot"
               OpMemberName %Root 1 "outer"
               OpName %data "data"
               OpName %values "values"
               OpName %i "i"
               OpName %j "j"
               OpName %k "k"
               OpName %ok "ok"
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
               OpMemberDecorate %Root 1 Offset 16
               OpMemberDecorate %Root 1 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
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
        %int = OpTypeInt 32 1
    %v3float = OpTypeVector %float 3
   %InnerLUT = OpTypeStruct %v3float
      %int_3 = OpConstant %int 3
%_arr_InnerLUT_int_3 = OpTypeArray %InnerLUT %int_3
   %OuterLUT = OpTypeStruct %_arr_InnerLUT_int_3
%_arr_OuterLUT_int_3 = OpTypeArray %OuterLUT %int_3
       %Root = OpTypeStruct %int %_arr_OuterLUT_int_3
%_ptr_Function_Root = OpTypePointer Function %Root
   %int_1234 = OpConstant %int 1234
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %42 = OpConstantComposite %v3float %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
   %float_10 = OpConstant %float 10
  %float_100 = OpConstant %float 100
         %63 = OpConstantComposite %v3float %float_1 %float_10 %float_100
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
    %float_2 = OpConstant %float 2
   %float_20 = OpConstant %float 20
  %float_200 = OpConstant %float 200
        %109 = OpConstantComposite %v3float %float_2 %float_20 %float_200
      %int_2 = OpConstant %int 2
    %float_3 = OpConstant %float 3
   %float_30 = OpConstant %float 30
  %float_300 = OpConstant %float 300
        %121 = OpConstantComposite %v3float %float_3 %float_30 %float_300
    %float_4 = OpConstant %float 4
   %float_40 = OpConstant %float 40
  %float_400 = OpConstant %float 400
        %132 = OpConstantComposite %v3float %float_4 %float_40 %float_400
    %float_5 = OpConstant %float 5
   %float_50 = OpConstant %float 50
  %float_500 = OpConstant %float 500
        %143 = OpConstantComposite %v3float %float_5 %float_50 %float_500
    %float_6 = OpConstant %float 6
   %float_60 = OpConstant %float 60
  %float_600 = OpConstant %float 600
        %154 = OpConstantComposite %v3float %float_6 %float_60 %float_600
    %float_7 = OpConstant %float 7
   %float_70 = OpConstant %float 70
  %float_700 = OpConstant %float 700
        %165 = OpConstantComposite %v3float %float_7 %float_70 %float_700
    %float_8 = OpConstant %float 8
   %float_80 = OpConstant %float 80
  %float_800 = OpConstant %float 800
        %176 = OpConstantComposite %v3float %float_8 %float_80 %float_800
    %float_9 = OpConstant %float 9
   %float_90 = OpConstant %float 90
  %float_900 = OpConstant %float 900
        %187 = OpConstantComposite %v3float %float_9 %float_90 %float_900
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
     %values = OpVariable %_ptr_Function_v3float Function
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
          %k = OpVariable %_ptr_Function_int Function
         %ok = OpVariable %_ptr_Function_bool Function
        %191 = OpVariable %_ptr_Function_v4float Function
         %38 = OpAccessChain %_ptr_Function_int %data %int_0
               OpStore %38 %int_1234
               OpStore %values %42
               OpStore %i %int_0
               OpBranch %44
         %44 = OpLabel
               OpLoopMerge %48 %47 None
               OpBranch %45
         %45 = OpLabel
         %49 = OpLoad %int %i
         %50 = OpSLessThan %bool %49 %int_3
               OpBranchConditional %50 %46 %48
         %46 = OpLabel
               OpStore %j %int_0
               OpBranch %52
         %52 = OpLabel
               OpLoopMerge %56 %55 None
               OpBranch %53
         %53 = OpLabel
         %57 = OpLoad %int %j
         %58 = OpSLessThan %bool %57 %int_3
               OpBranchConditional %58 %54 %56
         %54 = OpLabel
         %59 = OpLoad %v3float %values
         %64 = OpFAdd %v3float %59 %63
               OpStore %values %64
               OpStore %k %int_0
               OpBranch %66
         %66 = OpLabel
               OpLoopMerge %70 %69 None
               OpBranch %67
         %67 = OpLabel
         %71 = OpLoad %int %k
         %72 = OpSLessThan %bool %71 %int_3
               OpBranchConditional %72 %68 %70
         %68 = OpLabel
         %73 = OpLoad %v3float %values
         %74 = OpLoad %int %k
         %75 = OpVectorExtractDynamic %float %73 %74
         %77 = OpLoad %int %i
         %78 = OpLoad %int %j
         %79 = OpLoad %int %k
         %80 = OpAccessChain %_ptr_Function_float %data %int_1 %77 %int_0 %78 %int_0 %79
               OpStore %80 %75
               OpBranch %69
         %69 = OpLabel
         %82 = OpLoad %int %k
         %83 = OpIAdd %int %82 %int_1
               OpStore %k %83
               OpBranch %66
         %70 = OpLabel
               OpBranch %55
         %55 = OpLabel
         %84 = OpLoad %int %j
         %85 = OpIAdd %int %84 %int_1
               OpStore %j %85
               OpBranch %52
         %56 = OpLabel
               OpBranch %47
         %47 = OpLabel
         %86 = OpLoad %int %i
         %87 = OpIAdd %int %86 %int_1
               OpStore %i %87
               OpBranch %44
         %48 = OpLabel
         %91 = OpAccessChain %_ptr_Function_int %data %int_0
         %92 = OpLoad %int %91
         %93 = OpIEqual %bool %92 %int_1234
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %96 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_0 %int_0
         %97 = OpLoad %v3float %96
         %98 = OpFOrdEqual %v3bool %97 %63
        %100 = OpAll %bool %98
               OpBranch %95
         %95 = OpLabel
        %101 = OpPhi %bool %false %48 %100 %94
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %104 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_1 %int_0
        %105 = OpLoad %v3float %104
        %110 = OpFOrdEqual %v3bool %105 %109
        %111 = OpAll %bool %110
               OpBranch %103
        %103 = OpLabel
        %112 = OpPhi %bool %false %95 %111 %102
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %116 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_2 %int_0
        %117 = OpLoad %v3float %116
        %122 = OpFOrdEqual %v3bool %117 %121
        %123 = OpAll %bool %122
               OpBranch %114
        %114 = OpLabel
        %124 = OpPhi %bool %false %103 %123 %113
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
        %127 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_0 %int_0
        %128 = OpLoad %v3float %127
        %133 = OpFOrdEqual %v3bool %128 %132
        %134 = OpAll %bool %133
               OpBranch %126
        %126 = OpLabel
        %135 = OpPhi %bool %false %114 %134 %125
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %138 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_1 %int_0
        %139 = OpLoad %v3float %138
        %144 = OpFOrdEqual %v3bool %139 %143
        %145 = OpAll %bool %144
               OpBranch %137
        %137 = OpLabel
        %146 = OpPhi %bool %false %126 %145 %136
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_2 %int_0
        %150 = OpLoad %v3float %149
        %155 = OpFOrdEqual %v3bool %150 %154
        %156 = OpAll %bool %155
               OpBranch %148
        %148 = OpLabel
        %157 = OpPhi %bool %false %137 %156 %147
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %160 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_0 %int_0
        %161 = OpLoad %v3float %160
        %166 = OpFOrdEqual %v3bool %161 %165
        %167 = OpAll %bool %166
               OpBranch %159
        %159 = OpLabel
        %168 = OpPhi %bool %false %148 %167 %158
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
        %171 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_1 %int_0
        %172 = OpLoad %v3float %171
        %177 = OpFOrdEqual %v3bool %172 %176
        %178 = OpAll %bool %177
               OpBranch %170
        %170 = OpLabel
        %179 = OpPhi %bool %false %159 %178 %169
               OpSelectionMerge %181 None
               OpBranchConditional %179 %180 %181
        %180 = OpLabel
        %182 = OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_2 %int_0
        %183 = OpLoad %v3float %182
        %188 = OpFOrdEqual %v3bool %183 %187
        %189 = OpAll %bool %188
               OpBranch %181
        %181 = OpLabel
        %190 = OpPhi %bool %false %170 %189 %180
               OpStore %ok %190
               OpSelectionMerge %195 None
               OpBranchConditional %190 %193 %194
        %193 = OpLabel
        %196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %198 = OpLoad %v4float %196
               OpStore %191 %198
               OpBranch %195
        %194 = OpLabel
        %199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %200 = OpLoad %v4float %199
               OpStore %191 %200
               OpBranch %195
        %195 = OpLabel
        %201 = OpLoad %v4float %191
               OpReturnValue %201
               OpFunctionEnd
