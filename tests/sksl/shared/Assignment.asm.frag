               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %globalVar "globalVar"
               OpName %S "S"
               OpMemberName %S 0 "f"
               OpMemberName %S 1 "af"
               OpMemberName %S 2 "h4"
               OpMemberName %S 3 "ah4"
               OpName %globalStruct "globalStruct"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %keepAlive_vh "keepAlive_vh"
               OpName %keepAlive_vf "keepAlive_vf"
               OpName %keepAlive_vi "keepAlive_vi"
               OpName %assignToFunctionParameter_vif "assignToFunctionParameter_vif"
               OpName %main "main"
               OpName %i "i"
               OpName %i4 "i4"
               OpName %f3x3 "f3x3"
               OpName %x "x"
               OpName %ai "ai"
               OpName %ai4 "ai4"
               OpName %ah3x3 "ah3x3"
               OpName %af4 "af4"
               OpName %s "s"
               OpName %l "l"
               OpName %repeat "repeat"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalVar RelaxedPrecision
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpDecorate %_arr_v4float_int_5 ArrayStride 16
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 16
               OpMemberDecorate %S 2 Offset 96
               OpMemberDecorate %S 2 RelaxedPrecision
               OpMemberDecorate %S 3 Offset 112
               OpMemberDecorate %S 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %23 Binding 0
               OpDecorate %23 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %_arr_int_int_1 ArrayStride 16
               OpDecorate %_arr_v4int_int_1 ArrayStride 16
               OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_v4float = OpTypePointer Private %v4float
  %globalVar = OpVariable %_ptr_Private_v4float Private
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
          %S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Private_S = OpTypePointer Private %S
%globalStruct = OpVariable %_ptr_Private_S Private
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %32 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %37 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Function_int = OpTypePointer Function %int
         %43 = OpTypeFunction %void %_ptr_Function_int
         %46 = OpTypeFunction %void %_ptr_Function_int %_ptr_Function_float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
         %52 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
         %63 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
         %76 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %77 = OpConstantComposite %v3float %float_4 %float_5 %float_6
         %78 = OpConstantComposite %v3float %float_7 %float_8 %float_9
         %79 = OpConstantComposite %mat3v3float %76 %77 %78
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
        %102 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
        %110 = OpConstantComposite %v3float %float_9 %float_9 %float_9
        %114 = OpConstantComposite %v2float %float_5 %float_5
        %118 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %139 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %28
         %29 = OpLabel
         %33 = OpVariable %_ptr_Function_v2float Function
               OpStore %33 %32
         %35 = OpFunctionCall %v4float %main %33
               OpStore %sk_FragColor %35
               OpReturn
               OpFunctionEnd
%keepAlive_vh = OpFunction %void None %37
         %38 = OpFunctionParameter %_ptr_Function_float
         %39 = OpLabel
               OpReturn
               OpFunctionEnd
%keepAlive_vf = OpFunction %void None %37
         %40 = OpFunctionParameter %_ptr_Function_float
         %41 = OpLabel
               OpReturn
               OpFunctionEnd
%keepAlive_vi = OpFunction %void None %43
         %44 = OpFunctionParameter %_ptr_Function_int
         %45 = OpLabel
               OpReturn
               OpFunctionEnd
%assignToFunctionParameter_vif = OpFunction %void None %46
         %47 = OpFunctionParameter %_ptr_Function_int
         %48 = OpFunctionParameter %_ptr_Function_float
         %49 = OpLabel
               OpStore %47 %int_1
               OpStore %48 %float_1
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %52
         %53 = OpFunctionParameter %_ptr_Function_v2float
         %54 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %i4 = OpVariable %_ptr_Function_v4int Function
       %f3x3 = OpVariable %_ptr_Function_mat3v3float Function
          %x = OpVariable %_ptr_Function_v4float Function
         %ai = OpVariable %_ptr_Function__arr_int_int_1 Function
        %ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
      %ah3x3 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
        %af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
          %s = OpVariable %_ptr_Function_S Function
        %121 = OpVariable %_ptr_Function_int Function
        %126 = OpVariable %_ptr_Function_float Function
          %l = OpVariable %_ptr_Function_float Function
     %repeat = OpVariable %_ptr_Function_float Function
        %145 = OpVariable %_ptr_Function_float Function
        %151 = OpVariable %_ptr_Function_float Function
        %155 = OpVariable %_ptr_Function_int Function
        %160 = OpVariable %_ptr_Function_int Function
        %165 = OpVariable %_ptr_Function_int Function
        %171 = OpVariable %_ptr_Function_int Function
        %176 = OpVariable %_ptr_Function_float Function
        %181 = OpVariable %_ptr_Function_float Function
        %185 = OpVariable %_ptr_Function_float Function
        %191 = OpVariable %_ptr_Function_float Function
        %195 = OpVariable %_ptr_Function_float Function
               OpStore %i %int_0
               OpStore %i4 %63
               OpStore %f3x3 %79
         %82 = OpAccessChain %_ptr_Function_float %x %int_3
               OpStore %82 %float_0
         %83 = OpLoad %v4float %x
         %84 = OpVectorShuffle %v4float %83 %32 5 4 2 3
               OpStore %x %84
         %88 = OpAccessChain %_ptr_Function_int %ai %int_0
               OpStore %88 %int_0
         %92 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
               OpStore %92 %63
         %96 = OpAccessChain %_ptr_Function_mat3v3float %ah3x3 %int_0
               OpStore %96 %79
        %100 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
        %101 = OpAccessChain %_ptr_Function_float %100 %int_0
               OpStore %101 %float_0
        %103 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
        %104 = OpLoad %v4float %103
        %105 = OpVectorShuffle %v4float %104 %102 6 4 7 5
               OpStore %103 %105
        %108 = OpAccessChain %_ptr_Function_float %s %int_0
               OpStore %108 %float_0
        %109 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
               OpStore %109 %float_0
        %111 = OpAccessChain %_ptr_Function_v4float %s %int_2
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v4float %112 %110 5 6 4 3
               OpStore %111 %113
        %115 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v4float %116 %114 0 4 2 5
               OpStore %115 %117
               OpStore %globalVar %118
        %119 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
               OpStore %119 %float_0
               OpStore %121 %int_0
        %122 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
        %124 = OpAccessChain %_ptr_Function_float %122 %int_0
        %125 = OpLoad %float %124
               OpStore %126 %125
        %127 = OpFunctionCall %void %assignToFunctionParameter_vif %121 %126
        %128 = OpLoad %float %126
               OpStore %124 %128
               OpStore %l %float_0
        %130 = OpAccessChain %_ptr_Function_int %ai %int_0
        %131 = OpLoad %int %130
        %132 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
        %133 = OpLoad %v4int %132
        %134 = OpCompositeExtract %int %133 0
        %135 = OpIAdd %int %131 %134
               OpStore %130 %135
        %136 = OpAccessChain %_ptr_Function_float %s %int_0
               OpStore %136 %float_1
        %137 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
               OpStore %137 %float_2
        %138 = OpAccessChain %_ptr_Function_v4float %s %int_2
               OpStore %138 %102
        %140 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
               OpStore %140 %139
               OpStore %repeat %float_1
               OpStore %repeat %float_1
        %142 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
        %143 = OpAccessChain %_ptr_Function_float %142 %int_0
        %144 = OpLoad %float %143
               OpStore %145 %144
        %146 = OpFunctionCall %void %keepAlive_vf %145
        %147 = OpLoad %float %145
               OpStore %143 %147
        %148 = OpAccessChain %_ptr_Function_v3float %ah3x3 %int_0 %int_0
        %149 = OpAccessChain %_ptr_Function_float %148 %int_0
        %150 = OpLoad %float %149
               OpStore %151 %150
        %152 = OpFunctionCall %void %keepAlive_vh %151
        %153 = OpLoad %float %151
               OpStore %149 %153
        %154 = OpLoad %int %i
               OpStore %155 %154
        %156 = OpFunctionCall %void %keepAlive_vi %155
        %157 = OpLoad %int %155
               OpStore %i %157
        %158 = OpAccessChain %_ptr_Function_int %i4 %int_1
        %159 = OpLoad %int %158
               OpStore %160 %159
        %161 = OpFunctionCall %void %keepAlive_vi %160
        %162 = OpLoad %int %160
               OpStore %158 %162
        %163 = OpAccessChain %_ptr_Function_int %ai %int_0
        %164 = OpLoad %int %163
               OpStore %165 %164
        %166 = OpFunctionCall %void %keepAlive_vi %165
        %167 = OpLoad %int %165
               OpStore %163 %167
        %168 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
        %169 = OpAccessChain %_ptr_Function_int %168 %int_0
        %170 = OpLoad %int %169
               OpStore %171 %170
        %172 = OpFunctionCall %void %keepAlive_vi %171
        %173 = OpLoad %int %171
               OpStore %169 %173
        %174 = OpAccessChain %_ptr_Function_float %x %int_1
        %175 = OpLoad %float %174
               OpStore %176 %175
        %177 = OpFunctionCall %void %keepAlive_vh %176
        %178 = OpLoad %float %176
               OpStore %174 %178
        %179 = OpAccessChain %_ptr_Function_float %s %int_0
        %180 = OpLoad %float %179
               OpStore %181 %180
        %182 = OpFunctionCall %void %keepAlive_vf %181
        %183 = OpLoad %float %181
               OpStore %179 %183
        %184 = OpLoad %float %l
               OpStore %185 %184
        %186 = OpFunctionCall %void %keepAlive_vh %185
        %187 = OpLoad %float %185
               OpStore %l %187
        %188 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
        %189 = OpAccessChain %_ptr_Function_float %188 %int_0
        %190 = OpLoad %float %189
               OpStore %191 %190
        %192 = OpFunctionCall %void %keepAlive_vf %191
        %193 = OpLoad %float %191
               OpStore %189 %193
        %194 = OpLoad %float %repeat
               OpStore %195 %194
        %196 = OpFunctionCall %void %keepAlive_vf %195
        %197 = OpLoad %float %195
               OpStore %repeat %197
        %198 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %200 = OpLoad %v4float %198
               OpReturnValue %200
               OpFunctionEnd
