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
               OpMemberName %Root 0 "valueAtRoot"
               OpMemberName %Root 1 "outer"
               OpName %data "data"                  ; id %27
               OpName %values "values"              ; id %40
               OpName %i "i"                        ; id %43
               OpName %j "j"                        ; id %52
               OpName %k "k"                        ; id %66
               OpName %ok "ok"                      ; id %89

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
               OpMemberDecorate %Root 1 Offset 16
               OpMemberDecorate %Root 1 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision

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
       %Root = OpTypeStruct %int %_arr_OuterLUT_int_3
%_ptr_Function_Root = OpTypePointer Function %Root
   %int_1234 = OpConstant %int 1234
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %42 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
   %float_10 = OpConstant %float 10
  %float_100 = OpConstant %float 100
         %64 = OpConstantComposite %v3float %float_1 %float_10 %float_100
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
    %float_2 = OpConstant %float 2
   %float_20 = OpConstant %float 20
  %float_200 = OpConstant %float 200
        %110 = OpConstantComposite %v3float %float_2 %float_20 %float_200
      %int_2 = OpConstant %int 2
    %float_3 = OpConstant %float 3
   %float_30 = OpConstant %float 30
  %float_300 = OpConstant %float 300
        %122 = OpConstantComposite %v3float %float_3 %float_30 %float_300
    %float_4 = OpConstant %float 4
   %float_40 = OpConstant %float 40
  %float_400 = OpConstant %float 400
        %133 = OpConstantComposite %v3float %float_4 %float_40 %float_400
    %float_5 = OpConstant %float 5
   %float_50 = OpConstant %float 50
  %float_500 = OpConstant %float 500
        %144 = OpConstantComposite %v3float %float_5 %float_50 %float_500
    %float_6 = OpConstant %float 6
   %float_60 = OpConstant %float 60
  %float_600 = OpConstant %float 600
        %155 = OpConstantComposite %v3float %float_6 %float_60 %float_600
    %float_7 = OpConstant %float 7
   %float_70 = OpConstant %float 70
  %float_700 = OpConstant %float 700
        %166 = OpConstantComposite %v3float %float_7 %float_70 %float_700
    %float_8 = OpConstant %float 8
   %float_80 = OpConstant %float 80
  %float_800 = OpConstant %float 800
        %177 = OpConstantComposite %v3float %float_8 %float_80 %float_800
    %float_9 = OpConstant %float 9
   %float_90 = OpConstant %float 90
  %float_900 = OpConstant %float 900
        %188 = OpConstantComposite %v3float %float_9 %float_90 %float_900
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
     %values =   OpVariable %_ptr_Function_v3float Function
          %i =   OpVariable %_ptr_Function_int Function
          %j =   OpVariable %_ptr_Function_int Function
          %k =   OpVariable %_ptr_Function_int Function
         %ok =   OpVariable %_ptr_Function_bool Function
        %192 =   OpVariable %_ptr_Function_v4float Function
         %38 =   OpAccessChain %_ptr_Function_int %data %int_0
                 OpStore %38 %int_1234
                 OpStore %values %42
                 OpStore %i %int_0
                 OpBranch %44

         %44 = OpLabel
                 OpLoopMerge %48 %47 None
                 OpBranch %45

         %45 =     OpLabel
         %49 =       OpLoad %int %i
         %50 =       OpSLessThan %bool %49 %int_3
                     OpBranchConditional %50 %46 %48

         %46 =         OpLabel
                         OpStore %j %int_0
                         OpBranch %53

         %53 =         OpLabel
                         OpLoopMerge %57 %56 None
                         OpBranch %54

         %54 =             OpLabel
         %58 =               OpLoad %int %j
         %59 =               OpSLessThan %bool %58 %int_3
                             OpBranchConditional %59 %55 %57

         %55 =                 OpLabel
         %60 =                   OpLoad %v3float %values
         %65 =                   OpFAdd %v3float %60 %64
                                 OpStore %values %65
                                 OpStore %k %int_0
                                 OpBranch %67

         %67 =                 OpLabel
                                 OpLoopMerge %71 %70 None
                                 OpBranch %68

         %68 =                     OpLabel
         %72 =                       OpLoad %int %k
         %73 =                       OpSLessThan %bool %72 %int_3
                                     OpBranchConditional %73 %69 %71

         %69 =                         OpLabel
         %74 =                           OpLoad %v3float %values
         %75 =                           OpLoad %int %k
         %76 =                           OpVectorExtractDynamic %float %74 %75
         %78 =                           OpLoad %int %i
         %79 =                           OpLoad %int %j
         %80 =                           OpLoad %int %k
         %81 =                           OpAccessChain %_ptr_Function_float %data %int_1 %78 %int_0 %79 %int_0 %80
                                         OpStore %81 %76
                                         OpBranch %70

         %70 =                   OpLabel
         %83 =                     OpLoad %int %k
         %84 =                     OpIAdd %int %83 %int_1
                                   OpStore %k %84
                                   OpBranch %67

         %71 =                 OpLabel
                                 OpBranch %56

         %56 =           OpLabel
         %85 =             OpLoad %int %j
         %86 =             OpIAdd %int %85 %int_1
                           OpStore %j %86
                           OpBranch %53

         %57 =         OpLabel
                         OpBranch %47

         %47 =   OpLabel
         %87 =     OpLoad %int %i
         %88 =     OpIAdd %int %87 %int_1
                   OpStore %i %88
                   OpBranch %44

         %48 = OpLabel
         %92 =   OpAccessChain %_ptr_Function_int %data %int_0
         %93 =   OpLoad %int %92
         %94 =   OpIEqual %bool %93 %int_1234
                 OpSelectionMerge %96 None
                 OpBranchConditional %94 %95 %96

         %95 =     OpLabel
         %97 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_0 %int_0
         %98 =       OpLoad %v3float %97
         %99 =       OpFOrdEqual %v3bool %98 %64
        %101 =       OpAll %bool %99
                     OpBranch %96

         %96 = OpLabel
        %102 =   OpPhi %bool %false %48 %101 %95
                 OpSelectionMerge %104 None
                 OpBranchConditional %102 %103 %104

        %103 =     OpLabel
        %105 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_1 %int_0
        %106 =       OpLoad %v3float %105
        %111 =       OpFOrdEqual %v3bool %106 %110
        %112 =       OpAll %bool %111
                     OpBranch %104

        %104 = OpLabel
        %113 =   OpPhi %bool %false %96 %112 %103
                 OpSelectionMerge %115 None
                 OpBranchConditional %113 %114 %115

        %114 =     OpLabel
        %117 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_0 %int_0 %int_2 %int_0
        %118 =       OpLoad %v3float %117
        %123 =       OpFOrdEqual %v3bool %118 %122
        %124 =       OpAll %bool %123
                     OpBranch %115

        %115 = OpLabel
        %125 =   OpPhi %bool %false %104 %124 %114
                 OpSelectionMerge %127 None
                 OpBranchConditional %125 %126 %127

        %126 =     OpLabel
        %128 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_0 %int_0
        %129 =       OpLoad %v3float %128
        %134 =       OpFOrdEqual %v3bool %129 %133
        %135 =       OpAll %bool %134
                     OpBranch %127

        %127 = OpLabel
        %136 =   OpPhi %bool %false %115 %135 %126
                 OpSelectionMerge %138 None
                 OpBranchConditional %136 %137 %138

        %137 =     OpLabel
        %139 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_1 %int_0
        %140 =       OpLoad %v3float %139
        %145 =       OpFOrdEqual %v3bool %140 %144
        %146 =       OpAll %bool %145
                     OpBranch %138

        %138 = OpLabel
        %147 =   OpPhi %bool %false %127 %146 %137
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %150 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_1 %int_0 %int_2 %int_0
        %151 =       OpLoad %v3float %150
        %156 =       OpFOrdEqual %v3bool %151 %155
        %157 =       OpAll %bool %156
                     OpBranch %149

        %149 = OpLabel
        %158 =   OpPhi %bool %false %138 %157 %148
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
        %161 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_0 %int_0
        %162 =       OpLoad %v3float %161
        %167 =       OpFOrdEqual %v3bool %162 %166
        %168 =       OpAll %bool %167
                     OpBranch %160

        %160 = OpLabel
        %169 =   OpPhi %bool %false %149 %168 %159
                 OpSelectionMerge %171 None
                 OpBranchConditional %169 %170 %171

        %170 =     OpLabel
        %172 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_1 %int_0
        %173 =       OpLoad %v3float %172
        %178 =       OpFOrdEqual %v3bool %173 %177
        %179 =       OpAll %bool %178
                     OpBranch %171

        %171 = OpLabel
        %180 =   OpPhi %bool %false %160 %179 %170
                 OpSelectionMerge %182 None
                 OpBranchConditional %180 %181 %182

        %181 =     OpLabel
        %183 =       OpAccessChain %_ptr_Function_v3float %data %int_1 %int_2 %int_0 %int_2 %int_0
        %184 =       OpLoad %v3float %183
        %189 =       OpFOrdEqual %v3bool %184 %188
        %190 =       OpAll %bool %189
                     OpBranch %182

        %182 = OpLabel
        %191 =   OpPhi %bool %false %171 %190 %181
                 OpStore %ok %191
                 OpSelectionMerge %196 None
                 OpBranchConditional %191 %194 %195

        %194 =     OpLabel
        %197 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %199 =       OpLoad %v4float %197           ; RelaxedPrecision
                     OpStore %192 %199
                     OpBranch %196

        %195 =     OpLabel
        %200 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %201 =       OpLoad %v4float %200           ; RelaxedPrecision
                     OpStore %192 %201
                     OpBranch %196

        %196 = OpLabel
        %202 =   OpLoad %v4float %192               ; RelaxedPrecision
                 OpReturnValue %202
               OpFunctionEnd
