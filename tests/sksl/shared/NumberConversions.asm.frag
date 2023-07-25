               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"
               OpName %b "b"
               OpName %s "s"
               OpName %i "i"
               OpName %us "us"
               OpName %ui "ui"
               OpName %h "h"
               OpName %f "f"
               OpName %s2s "s2s"
               OpName %i2s "i2s"
               OpName %us2s "us2s"
               OpName %ui2s "ui2s"
               OpName %h2s "h2s"
               OpName %f2s "f2s"
               OpName %b2s "b2s"
               OpName %s2i "s2i"
               OpName %i2i "i2i"
               OpName %us2i "us2i"
               OpName %ui2i "ui2i"
               OpName %h2i "h2i"
               OpName %f2i "f2i"
               OpName %b2i "b2i"
               OpName %s2us "s2us"
               OpName %i2us "i2us"
               OpName %us2us "us2us"
               OpName %ui2us "ui2us"
               OpName %h2us "h2us"
               OpName %f2us "f2us"
               OpName %b2us "b2us"
               OpName %s2ui "s2ui"
               OpName %i2ui "i2ui"
               OpName %us2ui "us2ui"
               OpName %ui2ui "ui2ui"
               OpName %h2ui "h2ui"
               OpName %f2ui "f2ui"
               OpName %b2ui "b2ui"
               OpName %s2f "s2f"
               OpName %i2f "i2f"
               OpName %us2f "us2f"
               OpName %ui2f "ui2f"
               OpName %h2f "h2f"
               OpName %f2f "f2f"
               OpName %b2f "b2f"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %us RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %h RelaxedPrecision
               OpDecorate %s2s RelaxedPrecision
               OpDecorate %i2s RelaxedPrecision
               OpDecorate %us2s RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %ui2s RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %h2s RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %f2s RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %b2s RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %s2us RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %i2us RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %us2us RelaxedPrecision
               OpDecorate %ui2us RelaxedPrecision
               OpDecorate %h2us RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %f2us RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %b2us RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %14
         %15 = OpLabel
          %b = OpVariable %_ptr_Function_bool Function
          %s = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
         %us = OpVariable %_ptr_Function_uint Function
         %ui = OpVariable %_ptr_Function_uint Function
          %h = OpVariable %_ptr_Function_float Function
          %f = OpVariable %_ptr_Function_float Function
        %s2s = OpVariable %_ptr_Function_int Function
        %i2s = OpVariable %_ptr_Function_int Function
       %us2s = OpVariable %_ptr_Function_int Function
       %ui2s = OpVariable %_ptr_Function_int Function
        %h2s = OpVariable %_ptr_Function_int Function
        %f2s = OpVariable %_ptr_Function_int Function
        %b2s = OpVariable %_ptr_Function_int Function
        %s2i = OpVariable %_ptr_Function_int Function
        %i2i = OpVariable %_ptr_Function_int Function
       %us2i = OpVariable %_ptr_Function_int Function
       %ui2i = OpVariable %_ptr_Function_int Function
        %h2i = OpVariable %_ptr_Function_int Function
        %f2i = OpVariable %_ptr_Function_int Function
        %b2i = OpVariable %_ptr_Function_int Function
       %s2us = OpVariable %_ptr_Function_uint Function
       %i2us = OpVariable %_ptr_Function_uint Function
      %us2us = OpVariable %_ptr_Function_uint Function
      %ui2us = OpVariable %_ptr_Function_uint Function
       %h2us = OpVariable %_ptr_Function_uint Function
       %f2us = OpVariable %_ptr_Function_uint Function
       %b2us = OpVariable %_ptr_Function_uint Function
       %s2ui = OpVariable %_ptr_Function_uint Function
       %i2ui = OpVariable %_ptr_Function_uint Function
      %us2ui = OpVariable %_ptr_Function_uint Function
      %ui2ui = OpVariable %_ptr_Function_uint Function
       %h2ui = OpVariable %_ptr_Function_uint Function
       %f2ui = OpVariable %_ptr_Function_uint Function
       %b2ui = OpVariable %_ptr_Function_uint Function
        %s2f = OpVariable %_ptr_Function_float Function
        %i2f = OpVariable %_ptr_Function_float Function
       %us2f = OpVariable %_ptr_Function_float Function
       %ui2f = OpVariable %_ptr_Function_float Function
        %h2f = OpVariable %_ptr_Function_float Function
        %f2f = OpVariable %_ptr_Function_float Function
        %b2f = OpVariable %_ptr_Function_float Function
               OpStore %b %true
         %22 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %25 = OpLoad %float %22
         %26 = OpConvertFToS %int %25
               OpStore %s %26
         %28 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %29 = OpLoad %float %28
         %30 = OpConvertFToS %int %29
               OpStore %i %30
         %34 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %35 = OpLoad %float %34
         %36 = OpConvertFToU %uint %35
               OpStore %us %36
         %38 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %39 = OpLoad %float %38
         %40 = OpConvertFToU %uint %39
               OpStore %ui %40
         %43 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %44 = OpLoad %float %43
               OpStore %h %44
         %46 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %47 = OpLoad %float %46
               OpStore %f %47
               OpStore %s2s %26
               OpStore %i2s %30
         %51 = OpBitcast %int %36
               OpStore %us2s %51
         %53 = OpBitcast %int %40
               OpStore %ui2s %53
         %55 = OpConvertFToS %int %44
               OpStore %h2s %55
         %57 = OpConvertFToS %int %47
               OpStore %f2s %57
         %59 = OpSelect %int %true %int_1 %int_0
               OpStore %b2s %59
               OpStore %s2i %26
               OpStore %i2i %30
         %64 = OpBitcast %int %36
               OpStore %us2i %64
         %66 = OpBitcast %int %40
               OpStore %ui2i %66
         %68 = OpConvertFToS %int %44
               OpStore %h2i %68
         %70 = OpConvertFToS %int %47
               OpStore %f2i %70
         %72 = OpSelect %int %true %int_1 %int_0
               OpStore %b2i %72
         %74 = OpBitcast %uint %26
               OpStore %s2us %74
         %76 = OpBitcast %uint %30
               OpStore %i2us %76
               OpStore %us2us %36
               OpStore %ui2us %40
         %80 = OpConvertFToU %uint %44
               OpStore %h2us %80
         %82 = OpConvertFToU %uint %47
               OpStore %f2us %82
         %84 = OpSelect %uint %true %uint_1 %uint_0
               OpStore %b2us %84
         %88 = OpBitcast %uint %26
               OpStore %s2ui %88
         %90 = OpBitcast %uint %30
               OpStore %i2ui %90
               OpStore %us2ui %36
               OpStore %ui2ui %40
         %94 = OpConvertFToU %uint %44
               OpStore %h2ui %94
         %96 = OpConvertFToU %uint %47
               OpStore %f2ui %96
         %98 = OpSelect %uint %true %uint_1 %uint_0
               OpStore %b2ui %98
        %100 = OpConvertSToF %float %26
               OpStore %s2f %100
        %102 = OpConvertSToF %float %30
               OpStore %i2f %102
        %104 = OpConvertUToF %float %36
               OpStore %us2f %104
        %106 = OpConvertUToF %float %40
               OpStore %ui2f %106
               OpStore %h2f %44
               OpStore %f2f %47
        %110 = OpSelect %float %true %float_1 %float_0
               OpStore %b2f %110
        %113 = OpConvertSToF %float %26
        %114 = OpConvertSToF %float %30
        %115 = OpFAdd %float %113 %114
        %116 = OpConvertUToF %float %36
        %117 = OpFAdd %float %115 %116
        %118 = OpConvertUToF %float %40
        %119 = OpFAdd %float %117 %118
        %120 = OpFAdd %float %119 %44
        %121 = OpFAdd %float %120 %47
        %122 = OpConvertSToF %float %26
        %123 = OpFAdd %float %121 %122
        %124 = OpConvertSToF %float %30
        %125 = OpFAdd %float %123 %124
        %126 = OpConvertSToF %float %51
        %127 = OpFAdd %float %125 %126
        %128 = OpConvertSToF %float %53
        %129 = OpFAdd %float %127 %128
        %130 = OpConvertSToF %float %55
        %131 = OpFAdd %float %129 %130
        %132 = OpConvertSToF %float %57
        %133 = OpFAdd %float %131 %132
        %134 = OpConvertSToF %float %59
        %135 = OpFAdd %float %133 %134
        %136 = OpConvertSToF %float %26
        %137 = OpFAdd %float %135 %136
        %138 = OpConvertSToF %float %30
        %139 = OpFAdd %float %137 %138
        %140 = OpConvertSToF %float %64
        %141 = OpFAdd %float %139 %140
        %142 = OpConvertSToF %float %66
        %143 = OpFAdd %float %141 %142
        %144 = OpConvertSToF %float %68
        %145 = OpFAdd %float %143 %144
        %146 = OpConvertSToF %float %70
        %147 = OpFAdd %float %145 %146
        %148 = OpConvertSToF %float %72
        %149 = OpFAdd %float %147 %148
        %150 = OpConvertUToF %float %74
        %151 = OpFAdd %float %149 %150
        %152 = OpConvertUToF %float %76
        %153 = OpFAdd %float %151 %152
        %154 = OpConvertUToF %float %36
        %155 = OpFAdd %float %153 %154
        %156 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %156 %155
        %158 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
        %159 = OpLoad %float %158
        %160 = OpLoad %uint %ui2us
        %161 = OpConvertUToF %float %160
        %162 = OpLoad %uint %h2us
        %163 = OpConvertUToF %float %162
        %164 = OpFAdd %float %161 %163
        %165 = OpLoad %uint %f2us
        %166 = OpConvertUToF %float %165
        %167 = OpFAdd %float %164 %166
        %168 = OpLoad %uint %b2us
        %169 = OpConvertUToF %float %168
        %170 = OpFAdd %float %167 %169
        %171 = OpLoad %uint %s2ui
        %172 = OpConvertUToF %float %171
        %173 = OpFAdd %float %170 %172
        %174 = OpLoad %uint %i2ui
        %175 = OpConvertUToF %float %174
        %176 = OpFAdd %float %173 %175
        %177 = OpLoad %uint %us2ui
        %178 = OpConvertUToF %float %177
        %179 = OpFAdd %float %176 %178
        %180 = OpLoad %uint %ui2ui
        %181 = OpConvertUToF %float %180
        %182 = OpFAdd %float %179 %181
        %183 = OpLoad %uint %h2ui
        %184 = OpConvertUToF %float %183
        %185 = OpFAdd %float %182 %184
        %186 = OpLoad %uint %f2ui
        %187 = OpConvertUToF %float %186
        %188 = OpFAdd %float %185 %187
        %189 = OpLoad %uint %b2ui
        %190 = OpConvertUToF %float %189
        %191 = OpFAdd %float %188 %190
        %192 = OpLoad %float %s2f
        %193 = OpFAdd %float %191 %192
        %194 = OpLoad %float %i2f
        %195 = OpFAdd %float %193 %194
        %196 = OpLoad %float %us2f
        %197 = OpFAdd %float %195 %196
        %198 = OpLoad %float %ui2f
        %199 = OpFAdd %float %197 %198
        %200 = OpLoad %float %h2f
        %201 = OpFAdd %float %199 %200
        %202 = OpLoad %float %f2f
        %203 = OpFAdd %float %201 %202
        %204 = OpLoad %float %b2f
        %205 = OpFAdd %float %203 %204
        %206 = OpFAdd %float %159 %205
               OpStore %158 %206
               OpReturn
               OpFunctionEnd
