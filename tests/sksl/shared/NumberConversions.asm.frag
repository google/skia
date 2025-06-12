               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"                  ; id %2
               OpName %b "b"                        ; id %13
               OpName %s "s"                        ; id %17
               OpName %i "i"                        ; id %25
               OpName %us "us"                      ; id %29
               OpName %ui "ui"                      ; id %35
               OpName %h "h"                        ; id %39
               OpName %f "f"                        ; id %43
               OpName %s2s "s2s"                    ; id %46
               OpName %i2s "i2s"                    ; id %47
               OpName %us2s "us2s"                  ; id %48
               OpName %ui2s "ui2s"                  ; id %50
               OpName %h2s "h2s"                    ; id %52
               OpName %f2s "f2s"                    ; id %54
               OpName %b2s "b2s"                    ; id %56
               OpName %s2i "s2i"                    ; id %59
               OpName %i2i "i2i"                    ; id %60
               OpName %us2i "us2i"                  ; id %61
               OpName %ui2i "ui2i"                  ; id %63
               OpName %h2i "h2i"                    ; id %65
               OpName %f2i "f2i"                    ; id %67
               OpName %b2i "b2i"                    ; id %69
               OpName %s2us "s2us"                  ; id %71
               OpName %i2us "i2us"                  ; id %73
               OpName %us2us "us2us"                ; id %75
               OpName %ui2us "ui2us"                ; id %76
               OpName %h2us "h2us"                  ; id %77
               OpName %f2us "f2us"                  ; id %79
               OpName %b2us "b2us"                  ; id %81
               OpName %s2ui "s2ui"                  ; id %85
               OpName %i2ui "i2ui"                  ; id %87
               OpName %us2ui "us2ui"                ; id %89
               OpName %ui2ui "ui2ui"                ; id %90
               OpName %h2ui "h2ui"                  ; id %91
               OpName %f2ui "f2ui"                  ; id %93
               OpName %b2ui "b2ui"                  ; id %95
               OpName %s2f "s2f"                    ; id %97
               OpName %i2f "i2f"                    ; id %99
               OpName %us2f "us2f"                  ; id %101
               OpName %ui2f "ui2f"                  ; id %103
               OpName %h2f "h2f"                    ; id %105
               OpName %f2f "f2f"                    ; id %106
               OpName %b2f "b2f"                    ; id %107

               ; Annotations
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

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
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


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
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
         %20 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %23 =   OpLoad %float %20
         %24 =   OpConvertFToS %int %23             ; RelaxedPrecision
                 OpStore %s %24
         %26 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %27 =   OpLoad %float %26
         %28 =   OpConvertFToS %int %27
                 OpStore %i %28
         %32 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %33 =   OpLoad %float %32
         %34 =   OpConvertFToU %uint %33            ; RelaxedPrecision
                 OpStore %us %34
         %36 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %37 =   OpLoad %float %36
         %38 =   OpConvertFToU %uint %37
                 OpStore %ui %38
         %41 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %42 =   OpLoad %float %41
                 OpStore %h %42
         %44 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %45 =   OpLoad %float %44
                 OpStore %f %45
                 OpStore %s2s %24
                 OpStore %i2s %28
         %49 =   OpBitcast %int %34                 ; RelaxedPrecision
                 OpStore %us2s %49
         %51 =   OpBitcast %int %38                 ; RelaxedPrecision
                 OpStore %ui2s %51
         %53 =   OpConvertFToS %int %42             ; RelaxedPrecision
                 OpStore %h2s %53
         %55 =   OpConvertFToS %int %45             ; RelaxedPrecision
                 OpStore %f2s %55
         %57 =   OpSelect %int %true %int_1 %int_0  ; RelaxedPrecision
                 OpStore %b2s %57
                 OpStore %s2i %24
                 OpStore %i2i %28
         %62 =   OpBitcast %int %34
                 OpStore %us2i %62
         %64 =   OpBitcast %int %38
                 OpStore %ui2i %64
         %66 =   OpConvertFToS %int %42
                 OpStore %h2i %66
         %68 =   OpConvertFToS %int %45
                 OpStore %f2i %68
         %70 =   OpSelect %int %true %int_1 %int_0
                 OpStore %b2i %70
         %72 =   OpBitcast %uint %24                ; RelaxedPrecision
                 OpStore %s2us %72
         %74 =   OpBitcast %uint %28                ; RelaxedPrecision
                 OpStore %i2us %74
                 OpStore %us2us %34
                 OpStore %ui2us %38
         %78 =   OpConvertFToU %uint %42            ; RelaxedPrecision
                 OpStore %h2us %78
         %80 =   OpConvertFToU %uint %45            ; RelaxedPrecision
                 OpStore %f2us %80
         %82 =   OpSelect %uint %true %uint_1 %uint_0   ; RelaxedPrecision
                 OpStore %b2us %82
         %86 =   OpBitcast %uint %24
                 OpStore %s2ui %86
         %88 =   OpBitcast %uint %28
                 OpStore %i2ui %88
                 OpStore %us2ui %34
                 OpStore %ui2ui %38
         %92 =   OpConvertFToU %uint %42
                 OpStore %h2ui %92
         %94 =   OpConvertFToU %uint %45
                 OpStore %f2ui %94
         %96 =   OpSelect %uint %true %uint_1 %uint_0
                 OpStore %b2ui %96
         %98 =   OpConvertSToF %float %24
                 OpStore %s2f %98
        %100 =   OpConvertSToF %float %28
                 OpStore %i2f %100
        %102 =   OpConvertUToF %float %34
                 OpStore %us2f %102
        %104 =   OpConvertUToF %float %38
                 OpStore %ui2f %104
                 OpStore %h2f %42
                 OpStore %f2f %45
        %108 =   OpSelect %float %true %float_1 %float_0
                 OpStore %b2f %108
        %111 =   OpConvertSToF %float %24           ; RelaxedPrecision
        %112 =   OpConvertSToF %float %28           ; RelaxedPrecision
        %113 =   OpFAdd %float %111 %112            ; RelaxedPrecision
        %114 =   OpConvertUToF %float %34           ; RelaxedPrecision
        %115 =   OpFAdd %float %113 %114            ; RelaxedPrecision
        %116 =   OpConvertUToF %float %38           ; RelaxedPrecision
        %117 =   OpFAdd %float %115 %116            ; RelaxedPrecision
        %118 =   OpFAdd %float %117 %42             ; RelaxedPrecision
        %119 =   OpFAdd %float %118 %45             ; RelaxedPrecision
        %120 =   OpConvertSToF %float %24           ; RelaxedPrecision
        %121 =   OpFAdd %float %119 %120            ; RelaxedPrecision
        %122 =   OpConvertSToF %float %28           ; RelaxedPrecision
        %123 =   OpFAdd %float %121 %122            ; RelaxedPrecision
        %124 =   OpConvertSToF %float %49           ; RelaxedPrecision
        %125 =   OpFAdd %float %123 %124            ; RelaxedPrecision
        %126 =   OpConvertSToF %float %51           ; RelaxedPrecision
        %127 =   OpFAdd %float %125 %126            ; RelaxedPrecision
        %128 =   OpConvertSToF %float %53           ; RelaxedPrecision
        %129 =   OpFAdd %float %127 %128            ; RelaxedPrecision
        %130 =   OpConvertSToF %float %55           ; RelaxedPrecision
        %131 =   OpFAdd %float %129 %130            ; RelaxedPrecision
        %132 =   OpConvertSToF %float %57           ; RelaxedPrecision
        %133 =   OpFAdd %float %131 %132            ; RelaxedPrecision
        %134 =   OpConvertSToF %float %24           ; RelaxedPrecision
        %135 =   OpFAdd %float %133 %134            ; RelaxedPrecision
        %136 =   OpConvertSToF %float %28           ; RelaxedPrecision
        %137 =   OpFAdd %float %135 %136            ; RelaxedPrecision
        %138 =   OpConvertSToF %float %62           ; RelaxedPrecision
        %139 =   OpFAdd %float %137 %138            ; RelaxedPrecision
        %140 =   OpConvertSToF %float %64           ; RelaxedPrecision
        %141 =   OpFAdd %float %139 %140            ; RelaxedPrecision
        %142 =   OpConvertSToF %float %66           ; RelaxedPrecision
        %143 =   OpFAdd %float %141 %142            ; RelaxedPrecision
        %144 =   OpConvertSToF %float %68           ; RelaxedPrecision
        %145 =   OpFAdd %float %143 %144            ; RelaxedPrecision
        %146 =   OpConvertSToF %float %70           ; RelaxedPrecision
        %147 =   OpFAdd %float %145 %146            ; RelaxedPrecision
        %148 =   OpConvertUToF %float %72           ; RelaxedPrecision
        %149 =   OpFAdd %float %147 %148            ; RelaxedPrecision
        %150 =   OpConvertUToF %float %74           ; RelaxedPrecision
        %151 =   OpFAdd %float %149 %150            ; RelaxedPrecision
        %152 =   OpConvertUToF %float %34           ; RelaxedPrecision
        %153 =   OpFAdd %float %151 %152            ; RelaxedPrecision
        %154 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %154 %153
        %156 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
        %157 =   OpLoad %float %156                 ; RelaxedPrecision
        %158 =   OpLoad %uint %ui2us                ; RelaxedPrecision
        %159 =   OpConvertUToF %float %158          ; RelaxedPrecision
        %160 =   OpLoad %uint %h2us                 ; RelaxedPrecision
        %161 =   OpConvertUToF %float %160          ; RelaxedPrecision
        %162 =   OpFAdd %float %159 %161            ; RelaxedPrecision
        %163 =   OpLoad %uint %f2us                 ; RelaxedPrecision
        %164 =   OpConvertUToF %float %163          ; RelaxedPrecision
        %165 =   OpFAdd %float %162 %164            ; RelaxedPrecision
        %166 =   OpLoad %uint %b2us                 ; RelaxedPrecision
        %167 =   OpConvertUToF %float %166          ; RelaxedPrecision
        %168 =   OpFAdd %float %165 %167            ; RelaxedPrecision
        %169 =   OpLoad %uint %s2ui
        %170 =   OpConvertUToF %float %169          ; RelaxedPrecision
        %171 =   OpFAdd %float %168 %170            ; RelaxedPrecision
        %172 =   OpLoad %uint %i2ui
        %173 =   OpConvertUToF %float %172          ; RelaxedPrecision
        %174 =   OpFAdd %float %171 %173            ; RelaxedPrecision
        %175 =   OpLoad %uint %us2ui
        %176 =   OpConvertUToF %float %175          ; RelaxedPrecision
        %177 =   OpFAdd %float %174 %176            ; RelaxedPrecision
        %178 =   OpLoad %uint %ui2ui
        %179 =   OpConvertUToF %float %178          ; RelaxedPrecision
        %180 =   OpFAdd %float %177 %179            ; RelaxedPrecision
        %181 =   OpLoad %uint %h2ui
        %182 =   OpConvertUToF %float %181          ; RelaxedPrecision
        %183 =   OpFAdd %float %180 %182            ; RelaxedPrecision
        %184 =   OpLoad %uint %f2ui
        %185 =   OpConvertUToF %float %184          ; RelaxedPrecision
        %186 =   OpFAdd %float %183 %185            ; RelaxedPrecision
        %187 =   OpLoad %uint %b2ui
        %188 =   OpConvertUToF %float %187          ; RelaxedPrecision
        %189 =   OpFAdd %float %186 %188            ; RelaxedPrecision
        %190 =   OpLoad %float %s2f
        %191 =   OpFAdd %float %189 %190            ; RelaxedPrecision
        %192 =   OpLoad %float %i2f
        %193 =   OpFAdd %float %191 %192            ; RelaxedPrecision
        %194 =   OpLoad %float %us2f
        %195 =   OpFAdd %float %193 %194            ; RelaxedPrecision
        %196 =   OpLoad %float %ui2f
        %197 =   OpFAdd %float %195 %196            ; RelaxedPrecision
        %198 =   OpLoad %float %h2f
        %199 =   OpFAdd %float %197 %198            ; RelaxedPrecision
        %200 =   OpLoad %float %f2f
        %201 =   OpFAdd %float %199 %200            ; RelaxedPrecision
        %202 =   OpLoad %float %b2f
        %203 =   OpFAdd %float %201 %202            ; RelaxedPrecision
        %204 =   OpFAdd %float %157 %203            ; RelaxedPrecision
                 OpStore %156 %204
                 OpReturn
               OpFunctionEnd
