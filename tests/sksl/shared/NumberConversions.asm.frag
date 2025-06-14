               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"                  ; id %6
               OpName %b "b"                        ; id %17
               OpName %s "s"                        ; id %21
               OpName %i "i"                        ; id %28
               OpName %us "us"                      ; id %32
               OpName %ui "ui"                      ; id %38
               OpName %h "h"                        ; id %42
               OpName %f "f"                        ; id %46
               OpName %s2s "s2s"                    ; id %49
               OpName %i2s "i2s"                    ; id %50
               OpName %us2s "us2s"                  ; id %51
               OpName %ui2s "ui2s"                  ; id %53
               OpName %h2s "h2s"                    ; id %55
               OpName %f2s "f2s"                    ; id %57
               OpName %b2s "b2s"                    ; id %59
               OpName %s2i "s2i"                    ; id %62
               OpName %i2i "i2i"                    ; id %63
               OpName %us2i "us2i"                  ; id %64
               OpName %ui2i "ui2i"                  ; id %66
               OpName %h2i "h2i"                    ; id %68
               OpName %f2i "f2i"                    ; id %70
               OpName %b2i "b2i"                    ; id %72
               OpName %s2us "s2us"                  ; id %74
               OpName %i2us "i2us"                  ; id %76
               OpName %us2us "us2us"                ; id %78
               OpName %ui2us "ui2us"                ; id %79
               OpName %h2us "h2us"                  ; id %80
               OpName %f2us "f2us"                  ; id %82
               OpName %b2us "b2us"                  ; id %84
               OpName %s2ui "s2ui"                  ; id %88
               OpName %i2ui "i2ui"                  ; id %90
               OpName %us2ui "us2ui"                ; id %92
               OpName %ui2ui "ui2ui"                ; id %93
               OpName %h2ui "h2ui"                  ; id %94
               OpName %f2ui "f2ui"                  ; id %96
               OpName %b2ui "b2ui"                  ; id %98
               OpName %s2f "s2f"                    ; id %100
               OpName %i2f "i2f"                    ; id %102
               OpName %us2f "us2f"                  ; id %104
               OpName %ui2f "ui2f"                  ; id %106
               OpName %h2f "h2f"                    ; id %108
               OpName %f2f "f2f"                    ; id %109
               OpName %b2f "b2f"                    ; id %110

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %s RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %us RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %h RelaxedPrecision
               OpDecorate %s2s RelaxedPrecision
               OpDecorate %i2s RelaxedPrecision
               OpDecorate %us2s RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %ui2s RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %h2s RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %f2s RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %b2s RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %s2us RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %i2us RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %us2us RelaxedPrecision
               OpDecorate %ui2us RelaxedPrecision
               OpDecorate %h2us RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %f2us RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %b2us RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
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
               OpDecorate %156 RelaxedPrecision
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
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
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


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
          %b =   OpVariable %_ptr_Function_bool Function
          %s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
          %i =   OpVariable %_ptr_Function_int Function
         %us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
         %ui =   OpVariable %_ptr_Function_uint Function
          %h =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %f =   OpVariable %_ptr_Function_float Function
        %s2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
        %i2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
       %us2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
       %ui2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
        %h2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
        %f2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
        %b2s =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
        %s2i =   OpVariable %_ptr_Function_int Function
        %i2i =   OpVariable %_ptr_Function_int Function
       %us2i =   OpVariable %_ptr_Function_int Function
       %ui2i =   OpVariable %_ptr_Function_int Function
        %h2i =   OpVariable %_ptr_Function_int Function
        %f2i =   OpVariable %_ptr_Function_int Function
        %b2i =   OpVariable %_ptr_Function_int Function
       %s2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
       %i2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
      %us2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
      %ui2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
       %h2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
       %f2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
       %b2us =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
       %s2ui =   OpVariable %_ptr_Function_uint Function
       %i2ui =   OpVariable %_ptr_Function_uint Function
      %us2ui =   OpVariable %_ptr_Function_uint Function
      %ui2ui =   OpVariable %_ptr_Function_uint Function
       %h2ui =   OpVariable %_ptr_Function_uint Function
       %f2ui =   OpVariable %_ptr_Function_uint Function
       %b2ui =   OpVariable %_ptr_Function_uint Function
        %s2f =   OpVariable %_ptr_Function_float Function
        %i2f =   OpVariable %_ptr_Function_float Function
       %us2f =   OpVariable %_ptr_Function_float Function
       %ui2f =   OpVariable %_ptr_Function_float Function
        %h2f =   OpVariable %_ptr_Function_float Function
        %f2f =   OpVariable %_ptr_Function_float Function
        %b2f =   OpVariable %_ptr_Function_float Function
                 OpStore %b %true
         %23 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %26 =   OpLoad %float %23
         %27 =   OpConvertFToS %int %26             ; RelaxedPrecision
                 OpStore %s %27
         %29 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %30 =   OpLoad %float %29
         %31 =   OpConvertFToS %int %30
                 OpStore %i %31
         %35 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %36 =   OpLoad %float %35
         %37 =   OpConvertFToU %uint %36            ; RelaxedPrecision
                 OpStore %us %37
         %39 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %40 =   OpLoad %float %39
         %41 =   OpConvertFToU %uint %40
                 OpStore %ui %41
         %44 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %45 =   OpLoad %float %44
                 OpStore %h %45
         %47 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %48 =   OpLoad %float %47
                 OpStore %f %48
                 OpStore %s2s %27
                 OpStore %i2s %31
         %52 =   OpBitcast %int %37                 ; RelaxedPrecision
                 OpStore %us2s %52
         %54 =   OpBitcast %int %41                 ; RelaxedPrecision
                 OpStore %ui2s %54
         %56 =   OpConvertFToS %int %45             ; RelaxedPrecision
                 OpStore %h2s %56
         %58 =   OpConvertFToS %int %48             ; RelaxedPrecision
                 OpStore %f2s %58
         %60 =   OpSelect %int %true %int_1 %int_0  ; RelaxedPrecision
                 OpStore %b2s %60
                 OpStore %s2i %27
                 OpStore %i2i %31
         %65 =   OpBitcast %int %37
                 OpStore %us2i %65
         %67 =   OpBitcast %int %41
                 OpStore %ui2i %67
         %69 =   OpConvertFToS %int %45
                 OpStore %h2i %69
         %71 =   OpConvertFToS %int %48
                 OpStore %f2i %71
         %73 =   OpSelect %int %true %int_1 %int_0
                 OpStore %b2i %73
         %75 =   OpBitcast %uint %27                ; RelaxedPrecision
                 OpStore %s2us %75
         %77 =   OpBitcast %uint %31                ; RelaxedPrecision
                 OpStore %i2us %77
                 OpStore %us2us %37
                 OpStore %ui2us %41
         %81 =   OpConvertFToU %uint %45            ; RelaxedPrecision
                 OpStore %h2us %81
         %83 =   OpConvertFToU %uint %48            ; RelaxedPrecision
                 OpStore %f2us %83
         %85 =   OpSelect %uint %true %uint_1 %uint_0   ; RelaxedPrecision
                 OpStore %b2us %85
         %89 =   OpBitcast %uint %27
                 OpStore %s2ui %89
         %91 =   OpBitcast %uint %31
                 OpStore %i2ui %91
                 OpStore %us2ui %37
                 OpStore %ui2ui %41
         %95 =   OpConvertFToU %uint %45
                 OpStore %h2ui %95
         %97 =   OpConvertFToU %uint %48
                 OpStore %f2ui %97
         %99 =   OpSelect %uint %true %uint_1 %uint_0
                 OpStore %b2ui %99
        %101 =   OpConvertSToF %float %27
                 OpStore %s2f %101
        %103 =   OpConvertSToF %float %31
                 OpStore %i2f %103
        %105 =   OpConvertUToF %float %37
                 OpStore %us2f %105
        %107 =   OpConvertUToF %float %41
                 OpStore %ui2f %107
                 OpStore %h2f %45
                 OpStore %f2f %48
        %111 =   OpSelect %float %true %float_1 %float_0
                 OpStore %b2f %111
        %114 =   OpConvertSToF %float %27           ; RelaxedPrecision
        %115 =   OpConvertSToF %float %31           ; RelaxedPrecision
        %116 =   OpFAdd %float %114 %115            ; RelaxedPrecision
        %117 =   OpConvertUToF %float %37           ; RelaxedPrecision
        %118 =   OpFAdd %float %116 %117            ; RelaxedPrecision
        %119 =   OpConvertUToF %float %41           ; RelaxedPrecision
        %120 =   OpFAdd %float %118 %119            ; RelaxedPrecision
        %121 =   OpFAdd %float %120 %45             ; RelaxedPrecision
        %122 =   OpFAdd %float %121 %48             ; RelaxedPrecision
        %123 =   OpConvertSToF %float %27           ; RelaxedPrecision
        %124 =   OpFAdd %float %122 %123            ; RelaxedPrecision
        %125 =   OpConvertSToF %float %31           ; RelaxedPrecision
        %126 =   OpFAdd %float %124 %125            ; RelaxedPrecision
        %127 =   OpConvertSToF %float %52           ; RelaxedPrecision
        %128 =   OpFAdd %float %126 %127            ; RelaxedPrecision
        %129 =   OpConvertSToF %float %54           ; RelaxedPrecision
        %130 =   OpFAdd %float %128 %129            ; RelaxedPrecision
        %131 =   OpConvertSToF %float %56           ; RelaxedPrecision
        %132 =   OpFAdd %float %130 %131            ; RelaxedPrecision
        %133 =   OpConvertSToF %float %58           ; RelaxedPrecision
        %134 =   OpFAdd %float %132 %133            ; RelaxedPrecision
        %135 =   OpConvertSToF %float %60           ; RelaxedPrecision
        %136 =   OpFAdd %float %134 %135            ; RelaxedPrecision
        %137 =   OpConvertSToF %float %27           ; RelaxedPrecision
        %138 =   OpFAdd %float %136 %137            ; RelaxedPrecision
        %139 =   OpConvertSToF %float %31           ; RelaxedPrecision
        %140 =   OpFAdd %float %138 %139            ; RelaxedPrecision
        %141 =   OpConvertSToF %float %65           ; RelaxedPrecision
        %142 =   OpFAdd %float %140 %141            ; RelaxedPrecision
        %143 =   OpConvertSToF %float %67           ; RelaxedPrecision
        %144 =   OpFAdd %float %142 %143            ; RelaxedPrecision
        %145 =   OpConvertSToF %float %69           ; RelaxedPrecision
        %146 =   OpFAdd %float %144 %145            ; RelaxedPrecision
        %147 =   OpConvertSToF %float %71           ; RelaxedPrecision
        %148 =   OpFAdd %float %146 %147            ; RelaxedPrecision
        %149 =   OpConvertSToF %float %73           ; RelaxedPrecision
        %150 =   OpFAdd %float %148 %149            ; RelaxedPrecision
        %151 =   OpConvertUToF %float %75           ; RelaxedPrecision
        %152 =   OpFAdd %float %150 %151            ; RelaxedPrecision
        %153 =   OpConvertUToF %float %77           ; RelaxedPrecision
        %154 =   OpFAdd %float %152 %153            ; RelaxedPrecision
        %155 =   OpConvertUToF %float %37           ; RelaxedPrecision
        %156 =   OpFAdd %float %154 %155            ; RelaxedPrecision
        %157 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %157 %156
        %159 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
        %160 =   OpLoad %float %159                 ; RelaxedPrecision
        %161 =   OpLoad %uint %ui2us                ; RelaxedPrecision
        %162 =   OpConvertUToF %float %161          ; RelaxedPrecision
        %163 =   OpLoad %uint %h2us                 ; RelaxedPrecision
        %164 =   OpConvertUToF %float %163          ; RelaxedPrecision
        %165 =   OpFAdd %float %162 %164            ; RelaxedPrecision
        %166 =   OpLoad %uint %f2us                 ; RelaxedPrecision
        %167 =   OpConvertUToF %float %166          ; RelaxedPrecision
        %168 =   OpFAdd %float %165 %167            ; RelaxedPrecision
        %169 =   OpLoad %uint %b2us                 ; RelaxedPrecision
        %170 =   OpConvertUToF %float %169          ; RelaxedPrecision
        %171 =   OpFAdd %float %168 %170            ; RelaxedPrecision
        %172 =   OpLoad %uint %s2ui
        %173 =   OpConvertUToF %float %172          ; RelaxedPrecision
        %174 =   OpFAdd %float %171 %173            ; RelaxedPrecision
        %175 =   OpLoad %uint %i2ui
        %176 =   OpConvertUToF %float %175          ; RelaxedPrecision
        %177 =   OpFAdd %float %174 %176            ; RelaxedPrecision
        %178 =   OpLoad %uint %us2ui
        %179 =   OpConvertUToF %float %178          ; RelaxedPrecision
        %180 =   OpFAdd %float %177 %179            ; RelaxedPrecision
        %181 =   OpLoad %uint %ui2ui
        %182 =   OpConvertUToF %float %181          ; RelaxedPrecision
        %183 =   OpFAdd %float %180 %182            ; RelaxedPrecision
        %184 =   OpLoad %uint %h2ui
        %185 =   OpConvertUToF %float %184          ; RelaxedPrecision
        %186 =   OpFAdd %float %183 %185            ; RelaxedPrecision
        %187 =   OpLoad %uint %f2ui
        %188 =   OpConvertUToF %float %187          ; RelaxedPrecision
        %189 =   OpFAdd %float %186 %188            ; RelaxedPrecision
        %190 =   OpLoad %uint %b2ui
        %191 =   OpConvertUToF %float %190          ; RelaxedPrecision
        %192 =   OpFAdd %float %189 %191            ; RelaxedPrecision
        %193 =   OpLoad %float %s2f
        %194 =   OpFAdd %float %192 %193            ; RelaxedPrecision
        %195 =   OpLoad %float %i2f
        %196 =   OpFAdd %float %194 %195            ; RelaxedPrecision
        %197 =   OpLoad %float %us2f
        %198 =   OpFAdd %float %196 %197            ; RelaxedPrecision
        %199 =   OpLoad %float %ui2f
        %200 =   OpFAdd %float %198 %199            ; RelaxedPrecision
        %201 =   OpLoad %float %h2f
        %202 =   OpFAdd %float %200 %201            ; RelaxedPrecision
        %203 =   OpLoad %float %f2f
        %204 =   OpFAdd %float %202 %203            ; RelaxedPrecision
        %205 =   OpLoad %float %b2f
        %206 =   OpFAdd %float %204 %205            ; RelaxedPrecision
        %207 =   OpFAdd %float %160 %206            ; RelaxedPrecision
                 OpStore %159 %207
                 OpReturn
               OpFunctionEnd
