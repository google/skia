               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %us RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %h RelaxedPrecision
               OpDecorate %s2s RelaxedPrecision
               OpDecorate %i2s RelaxedPrecision
               OpDecorate %us2s RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %ui2s RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %h2s RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %f2s RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %b2s RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %s2us RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %i2us RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %us2us RelaxedPrecision
               OpDecorate %ui2us RelaxedPrecision
               OpDecorate %h2us RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %f2us RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %b2us RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
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
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
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
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
       %bool = OpTypeBool
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
       %main = OpFunction %void None %11
         %12 = OpLabel
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
         %20 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %23 = OpLoad %float %20
         %24 = OpConvertFToS %int %23
               OpStore %s %24
         %26 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %27 = OpLoad %float %26
         %28 = OpConvertFToS %int %27
               OpStore %i %28
         %32 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %33 = OpLoad %float %32
         %34 = OpConvertFToU %uint %33
               OpStore %us %34
         %36 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %37 = OpLoad %float %36
         %38 = OpConvertFToU %uint %37
               OpStore %ui %38
         %41 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %42 = OpLoad %float %41
               OpStore %h %42
         %44 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %45 = OpLoad %float %44
               OpStore %f %45
               OpStore %s2s %24
               OpStore %i2s %28
         %49 = OpBitcast %int %34
               OpStore %us2s %49
         %51 = OpBitcast %int %38
               OpStore %ui2s %51
         %53 = OpConvertFToS %int %42
               OpStore %h2s %53
         %55 = OpConvertFToS %int %45
               OpStore %f2s %55
         %57 = OpSelect %int %true %int_1 %int_0
               OpStore %b2s %57
               OpStore %s2i %24
               OpStore %i2i %28
         %62 = OpBitcast %int %34
               OpStore %us2i %62
         %64 = OpBitcast %int %38
               OpStore %ui2i %64
         %66 = OpConvertFToS %int %42
               OpStore %h2i %66
         %68 = OpConvertFToS %int %45
               OpStore %f2i %68
         %70 = OpSelect %int %true %int_1 %int_0
               OpStore %b2i %70
         %72 = OpBitcast %uint %24
               OpStore %s2us %72
         %74 = OpBitcast %uint %28
               OpStore %i2us %74
               OpStore %us2us %34
               OpStore %ui2us %38
         %78 = OpConvertFToU %uint %42
               OpStore %h2us %78
         %80 = OpConvertFToU %uint %45
               OpStore %f2us %80
         %82 = OpSelect %uint %true %uint_1 %uint_0
               OpStore %b2us %82
         %86 = OpBitcast %uint %24
               OpStore %s2ui %86
         %88 = OpBitcast %uint %28
               OpStore %i2ui %88
               OpStore %us2ui %34
               OpStore %ui2ui %38
         %92 = OpConvertFToU %uint %42
               OpStore %h2ui %92
         %94 = OpConvertFToU %uint %45
               OpStore %f2ui %94
         %96 = OpSelect %uint %true %uint_1 %uint_0
               OpStore %b2ui %96
         %98 = OpConvertSToF %float %24
               OpStore %s2f %98
        %100 = OpConvertSToF %float %28
               OpStore %i2f %100
        %102 = OpConvertUToF %float %34
               OpStore %us2f %102
        %104 = OpConvertUToF %float %38
               OpStore %ui2f %104
               OpStore %h2f %42
               OpStore %f2f %45
        %108 = OpSelect %float %true %float_1 %float_0
               OpStore %b2f %108
        %111 = OpConvertSToF %float %24
        %112 = OpConvertSToF %float %28
        %113 = OpFAdd %float %111 %112
        %114 = OpConvertUToF %float %34
        %115 = OpFAdd %float %113 %114
        %116 = OpConvertUToF %float %38
        %117 = OpFAdd %float %115 %116
        %118 = OpFAdd %float %117 %42
        %119 = OpFAdd %float %118 %45
        %120 = OpConvertSToF %float %24
        %121 = OpFAdd %float %119 %120
        %122 = OpConvertSToF %float %28
        %123 = OpFAdd %float %121 %122
        %124 = OpConvertSToF %float %49
        %125 = OpFAdd %float %123 %124
        %126 = OpConvertSToF %float %51
        %127 = OpFAdd %float %125 %126
        %128 = OpConvertSToF %float %53
        %129 = OpFAdd %float %127 %128
        %130 = OpConvertSToF %float %55
        %131 = OpFAdd %float %129 %130
        %132 = OpConvertSToF %float %57
        %133 = OpFAdd %float %131 %132
        %134 = OpConvertSToF %float %24
        %135 = OpFAdd %float %133 %134
        %136 = OpConvertSToF %float %28
        %137 = OpFAdd %float %135 %136
        %138 = OpConvertSToF %float %62
        %139 = OpFAdd %float %137 %138
        %140 = OpConvertSToF %float %64
        %141 = OpFAdd %float %139 %140
        %142 = OpConvertSToF %float %66
        %143 = OpFAdd %float %141 %142
        %144 = OpConvertSToF %float %68
        %145 = OpFAdd %float %143 %144
        %146 = OpConvertSToF %float %70
        %147 = OpFAdd %float %145 %146
        %148 = OpConvertUToF %float %72
        %149 = OpFAdd %float %147 %148
        %150 = OpConvertUToF %float %74
        %151 = OpFAdd %float %149 %150
        %152 = OpConvertUToF %float %34
        %153 = OpFAdd %float %151 %152
        %154 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %154 %153
        %156 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
        %157 = OpLoad %float %156
        %158 = OpLoad %uint %ui2us
        %159 = OpConvertUToF %float %158
        %160 = OpLoad %uint %h2us
        %161 = OpConvertUToF %float %160
        %162 = OpFAdd %float %159 %161
        %163 = OpLoad %uint %f2us
        %164 = OpConvertUToF %float %163
        %165 = OpFAdd %float %162 %164
        %166 = OpLoad %uint %b2us
        %167 = OpConvertUToF %float %166
        %168 = OpFAdd %float %165 %167
        %169 = OpLoad %uint %s2ui
        %170 = OpConvertUToF %float %169
        %171 = OpFAdd %float %168 %170
        %172 = OpLoad %uint %i2ui
        %173 = OpConvertUToF %float %172
        %174 = OpFAdd %float %171 %173
        %175 = OpLoad %uint %us2ui
        %176 = OpConvertUToF %float %175
        %177 = OpFAdd %float %174 %176
        %178 = OpLoad %uint %ui2ui
        %179 = OpConvertUToF %float %178
        %180 = OpFAdd %float %177 %179
        %181 = OpLoad %uint %h2ui
        %182 = OpConvertUToF %float %181
        %183 = OpFAdd %float %180 %182
        %184 = OpLoad %uint %f2ui
        %185 = OpConvertUToF %float %184
        %186 = OpFAdd %float %183 %185
        %187 = OpLoad %uint %b2ui
        %188 = OpConvertUToF %float %187
        %189 = OpFAdd %float %186 %188
        %190 = OpLoad %float %s2f
        %191 = OpFAdd %float %189 %190
        %192 = OpLoad %float %i2f
        %193 = OpFAdd %float %191 %192
        %194 = OpLoad %float %us2f
        %195 = OpFAdd %float %193 %194
        %196 = OpLoad %float %ui2f
        %197 = OpFAdd %float %195 %196
        %198 = OpLoad %float %h2f
        %199 = OpFAdd %float %197 %198
        %200 = OpLoad %float %f2f
        %201 = OpFAdd %float %199 %200
        %202 = OpLoad %float %b2f
        %203 = OpFAdd %float %201 %202
        %204 = OpFAdd %float %157 %203
               OpStore %156 %204
               OpReturn
               OpFunctionEnd
