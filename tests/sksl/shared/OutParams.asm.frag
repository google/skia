               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %29
               OpName %_UniformBuffer "_UniformBuffer"  ; id %34
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %36
               OpName %out_half_vh "out_half_vh"        ; id %6
               OpName %out_half2_vh2 "out_half2_vh2"    ; id %7
               OpName %out_half3_vh3 "out_half3_vh3"    ; id %8
               OpName %out_half4_vh4 "out_half4_vh4"    ; id %9
               OpName %out_half2x2_vh22 "out_half2x2_vh22"  ; id %10
               OpName %out_half3x3_vh33 "out_half3x3_vh33"  ; id %11
               OpName %out_half4x4_vh44 "out_half4x4_vh44"  ; id %12
               OpName %out_int_vi "out_int_vi"              ; id %13
               OpName %out_int2_vi2 "out_int2_vi2"          ; id %14
               OpName %out_int3_vi3 "out_int3_vi3"          ; id %15
               OpName %out_int4_vi4 "out_int4_vi4"          ; id %16
               OpName %out_float_vf "out_float_vf"          ; id %17
               OpName %out_float2_vf2 "out_float2_vf2"      ; id %18
               OpName %out_float3_vf3 "out_float3_vf3"      ; id %19
               OpName %out_float4_vf4 "out_float4_vf4"      ; id %20
               OpName %out_float2x2_vf22 "out_float2x2_vf22"    ; id %21
               OpName %out_float3x3_vf33 "out_float3x3_vf33"    ; id %22
               OpName %out_float4x4_vf44 "out_float4x4_vf44"    ; id %23
               OpName %out_bool_vb "out_bool_vb"                ; id %24
               OpName %out_bool2_vb2 "out_bool2_vb2"            ; id %25
               OpName %out_bool3_vb3 "out_bool3_vb3"            ; id %26
               OpName %out_bool4_vb4 "out_bool4_vb4"            ; id %27
               OpName %main "main"                              ; id %28
               OpName %h "h"                                    ; id %245
               OpName %h2 "h2"                                  ; id %249
               OpName %h3 "h3"                                  ; id %253
               OpName %h4 "h4"                                  ; id %257
               OpName %h2x2 "h2x2"                              ; id %276
               OpName %h3x3 "h3x3"                              ; id %280
               OpName %h4x4 "h4x4"                              ; id %284
               OpName %i "i"                                    ; id %304
               OpName %i2 "i2"                                  ; id %308
               OpName %i3 "i3"                                  ; id %312
               OpName %i4 "i4"                                  ; id %316
               OpName %f "f"                                    ; id %329
               OpName %f2 "f2"                                  ; id %333
               OpName %f3 "f3"                                  ; id %337
               OpName %f4 "f4"                                  ; id %341
               OpName %f2x2 "f2x2"                              ; id %354
               OpName %f3x3 "f3x3"                              ; id %358
               OpName %f4x4 "f4x4"                              ; id %362
               OpName %b "b"                                    ; id %371
               OpName %b2 "b2"                                  ; id %375
               OpName %b3 "b3"                                  ; id %379
               OpName %b4 "b4"                                  ; id %383
               OpName %ok "ok"                                  ; id %396

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %33 Binding 0
               OpDecorate %33 DescriptorSet 0
               OpDecorate %48 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %h RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %h2 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %h3 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %h4 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %h2x2 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %h3x3 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %h4x4 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %301 RelaxedPrecision
               OpDecorate %303 RelaxedPrecision
               OpDecorate %402 RelaxedPrecision
               OpDecorate %403 RelaxedPrecision
               OpDecorate %404 RelaxedPrecision
               OpDecorate %405 RelaxedPrecision
               OpDecorate %406 RelaxedPrecision
               OpDecorate %407 RelaxedPrecision
               OpDecorate %408 RelaxedPrecision
               OpDecorate %409 RelaxedPrecision
               OpDecorate %410 RelaxedPrecision
               OpDecorate %411 RelaxedPrecision
               OpDecorate %413 RelaxedPrecision
               OpDecorate %414 RelaxedPrecision
               OpDecorate %415 RelaxedPrecision
               OpDecorate %417 RelaxedPrecision
               OpDecorate %418 RelaxedPrecision
               OpDecorate %419 RelaxedPrecision
               OpDecorate %421 RelaxedPrecision
               OpDecorate %422 RelaxedPrecision
               OpDecorate %423 RelaxedPrecision
               OpDecorate %468 RelaxedPrecision
               OpDecorate %471 RelaxedPrecision
               OpDecorate %476 RelaxedPrecision
               OpDecorate %481 RelaxedPrecision
               OpDecorate %490 RelaxedPrecision
               OpDecorate %492 RelaxedPrecision
               OpDecorate %493 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %33 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %38 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %42 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %47 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
         %55 = OpTypeFunction %void %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %64 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %72 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %81 = OpTypeFunction %void %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %92 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
        %104 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
        %116 = OpTypeFunction %void %_ptr_Function_int
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
        %125 = OpTypeFunction %void %_ptr_Function_v2int
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
        %135 = OpTypeFunction %void %_ptr_Function_v3int
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %145 = OpTypeFunction %void %_ptr_Function_v4int
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
        %205 = OpTypeFunction %void %_ptr_Function_bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
        %214 = OpTypeFunction %void %_ptr_Function_v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
        %224 = OpTypeFunction %void %_ptr_Function_v3bool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
        %234 = OpTypeFunction %void %_ptr_Function_v4bool
        %242 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %38

         %39 = OpLabel
         %43 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %43 %42
         %45 =   OpFunctionCall %v4float %main %43
                 OpStore %sk_FragColor %45
                 OpReturn
               OpFunctionEnd


               ; Function out_half_vh
%out_half_vh = OpFunction %void None %47
         %48 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %49 = OpLabel
         %50 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %53 =   OpLoad %v4float %50                ; RelaxedPrecision
         %54 =   OpCompositeExtract %float %53 0    ; RelaxedPrecision
                 OpStore %48 %54
                 OpReturn
               OpFunctionEnd


               ; Function out_half2_vh2
%out_half2_vh2 = OpFunction %void None %55
         %56 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %57 = OpLabel
         %58 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %59 =   OpLoad %v4float %58                ; RelaxedPrecision
         %60 =   OpCompositeExtract %float %59 1    ; RelaxedPrecision
         %61 =   OpCompositeConstruct %v2float %60 %60  ; RelaxedPrecision
                 OpStore %56 %61
                 OpReturn
               OpFunctionEnd


               ; Function out_half3_vh3
%out_half3_vh3 = OpFunction %void None %64
         %65 = OpFunctionParameter %_ptr_Function_v3float   ; RelaxedPrecision

         %66 = OpLabel
         %67 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %68 =   OpLoad %v4float %67                ; RelaxedPrecision
         %69 =   OpCompositeExtract %float %68 2    ; RelaxedPrecision
         %70 =   OpCompositeConstruct %v3float %69 %69 %69  ; RelaxedPrecision
                 OpStore %65 %70
                 OpReturn
               OpFunctionEnd


               ; Function out_half4_vh4
%out_half4_vh4 = OpFunction %void None %72
         %73 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %74 = OpLabel
         %75 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %76 =   OpLoad %v4float %75                ; RelaxedPrecision
         %77 =   OpCompositeExtract %float %76 3    ; RelaxedPrecision
         %78 =   OpCompositeConstruct %v4float %77 %77 %77 %77  ; RelaxedPrecision
                 OpStore %73 %78
                 OpReturn
               OpFunctionEnd


               ; Function out_half2x2_vh22
%out_half2x2_vh22 = OpFunction %void None %81
         %82 = OpFunctionParameter %_ptr_Function_mat2v2float   ; RelaxedPrecision

         %83 = OpLabel
         %84 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %85 =   OpLoad %v4float %84                ; RelaxedPrecision
         %86 =   OpCompositeExtract %float %85 0    ; RelaxedPrecision
         %87 =   OpCompositeConstruct %v2float %86 %float_0     ; RelaxedPrecision
         %88 =   OpCompositeConstruct %v2float %float_0 %86     ; RelaxedPrecision
         %89 =   OpCompositeConstruct %mat2v2float %87 %88      ; RelaxedPrecision
                 OpStore %82 %89
                 OpReturn
               OpFunctionEnd


               ; Function out_half3x3_vh33
%out_half3x3_vh33 = OpFunction %void None %92
         %93 = OpFunctionParameter %_ptr_Function_mat3v3float   ; RelaxedPrecision

         %94 = OpLabel
         %95 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
         %96 =   OpLoad %v4float %95                ; RelaxedPrecision
         %97 =   OpCompositeExtract %float %96 1    ; RelaxedPrecision
         %98 =   OpCompositeConstruct %v3float %97 %float_0 %float_0    ; RelaxedPrecision
         %99 =   OpCompositeConstruct %v3float %float_0 %97 %float_0    ; RelaxedPrecision
        %100 =   OpCompositeConstruct %v3float %float_0 %float_0 %97    ; RelaxedPrecision
        %101 =   OpCompositeConstruct %mat3v3float %98 %99 %100         ; RelaxedPrecision
                 OpStore %93 %101
                 OpReturn
               OpFunctionEnd


               ; Function out_half4x4_vh44
%out_half4x4_vh44 = OpFunction %void None %104
        %105 = OpFunctionParameter %_ptr_Function_mat4v4float   ; RelaxedPrecision

        %106 = OpLabel
        %107 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %108 =   OpLoad %v4float %107               ; RelaxedPrecision
        %109 =   OpCompositeExtract %float %108 2   ; RelaxedPrecision
        %110 =   OpCompositeConstruct %v4float %109 %float_0 %float_0 %float_0  ; RelaxedPrecision
        %111 =   OpCompositeConstruct %v4float %float_0 %109 %float_0 %float_0  ; RelaxedPrecision
        %112 =   OpCompositeConstruct %v4float %float_0 %float_0 %109 %float_0  ; RelaxedPrecision
        %113 =   OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %109  ; RelaxedPrecision
        %114 =   OpCompositeConstruct %mat4v4float %110 %111 %112 %113          ; RelaxedPrecision
                 OpStore %105 %114
                 OpReturn
               OpFunctionEnd


               ; Function out_int_vi
 %out_int_vi = OpFunction %void None %116
        %117 = OpFunctionParameter %_ptr_Function_int

        %118 = OpLabel
        %119 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %120 =   OpLoad %v4float %119               ; RelaxedPrecision
        %121 =   OpCompositeExtract %float %120 0   ; RelaxedPrecision
        %122 =   OpConvertFToS %int %121
                 OpStore %117 %122
                 OpReturn
               OpFunctionEnd


               ; Function out_int2_vi2
%out_int2_vi2 = OpFunction %void None %125
        %126 = OpFunctionParameter %_ptr_Function_v2int

        %127 = OpLabel
        %128 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %129 =   OpLoad %v4float %128               ; RelaxedPrecision
        %130 =   OpCompositeExtract %float %129 1   ; RelaxedPrecision
        %131 =   OpConvertFToS %int %130
        %132 =   OpCompositeConstruct %v2int %131 %131
                 OpStore %126 %132
                 OpReturn
               OpFunctionEnd


               ; Function out_int3_vi3
%out_int3_vi3 = OpFunction %void None %135
        %136 = OpFunctionParameter %_ptr_Function_v3int

        %137 = OpLabel
        %138 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %139 =   OpLoad %v4float %138               ; RelaxedPrecision
        %140 =   OpCompositeExtract %float %139 2   ; RelaxedPrecision
        %141 =   OpConvertFToS %int %140
        %142 =   OpCompositeConstruct %v3int %141 %141 %141
                 OpStore %136 %142
                 OpReturn
               OpFunctionEnd


               ; Function out_int4_vi4
%out_int4_vi4 = OpFunction %void None %145
        %146 = OpFunctionParameter %_ptr_Function_v4int

        %147 = OpLabel
        %148 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %149 =   OpLoad %v4float %148               ; RelaxedPrecision
        %150 =   OpCompositeExtract %float %149 3   ; RelaxedPrecision
        %151 =   OpConvertFToS %int %150
        %152 =   OpCompositeConstruct %v4int %151 %151 %151 %151
                 OpStore %146 %152
                 OpReturn
               OpFunctionEnd


               ; Function out_float_vf
%out_float_vf = OpFunction %void None %47
        %153 = OpFunctionParameter %_ptr_Function_float

        %154 = OpLabel
        %155 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %156 =   OpLoad %v4float %155               ; RelaxedPrecision
        %157 =   OpCompositeExtract %float %156 0   ; RelaxedPrecision
                 OpStore %153 %157
                 OpReturn
               OpFunctionEnd


               ; Function out_float2_vf2
%out_float2_vf2 = OpFunction %void None %55
        %158 = OpFunctionParameter %_ptr_Function_v2float

        %159 = OpLabel
        %160 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %161 =   OpLoad %v4float %160               ; RelaxedPrecision
        %162 =   OpCompositeExtract %float %161 1   ; RelaxedPrecision
        %163 =   OpCompositeConstruct %v2float %162 %162
                 OpStore %158 %163
                 OpReturn
               OpFunctionEnd


               ; Function out_float3_vf3
%out_float3_vf3 = OpFunction %void None %64
        %164 = OpFunctionParameter %_ptr_Function_v3float

        %165 = OpLabel
        %166 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %167 =   OpLoad %v4float %166               ; RelaxedPrecision
        %168 =   OpCompositeExtract %float %167 2   ; RelaxedPrecision
        %169 =   OpCompositeConstruct %v3float %168 %168 %168
                 OpStore %164 %169
                 OpReturn
               OpFunctionEnd


               ; Function out_float4_vf4
%out_float4_vf4 = OpFunction %void None %72
        %170 = OpFunctionParameter %_ptr_Function_v4float

        %171 = OpLabel
        %172 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %173 =   OpLoad %v4float %172               ; RelaxedPrecision
        %174 =   OpCompositeExtract %float %173 3   ; RelaxedPrecision
        %175 =   OpCompositeConstruct %v4float %174 %174 %174 %174
                 OpStore %170 %175
                 OpReturn
               OpFunctionEnd


               ; Function out_float2x2_vf22
%out_float2x2_vf22 = OpFunction %void None %81
        %176 = OpFunctionParameter %_ptr_Function_mat2v2float

        %177 = OpLabel
        %178 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %179 =   OpLoad %v4float %178               ; RelaxedPrecision
        %180 =   OpCompositeExtract %float %179 0   ; RelaxedPrecision
        %181 =   OpCompositeConstruct %v2float %180 %float_0
        %182 =   OpCompositeConstruct %v2float %float_0 %180
        %183 =   OpCompositeConstruct %mat2v2float %181 %182
                 OpStore %176 %183
                 OpReturn
               OpFunctionEnd


               ; Function out_float3x3_vf33
%out_float3x3_vf33 = OpFunction %void None %92
        %184 = OpFunctionParameter %_ptr_Function_mat3v3float

        %185 = OpLabel
        %186 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %187 =   OpLoad %v4float %186               ; RelaxedPrecision
        %188 =   OpCompositeExtract %float %187 1   ; RelaxedPrecision
        %189 =   OpCompositeConstruct %v3float %188 %float_0 %float_0
        %190 =   OpCompositeConstruct %v3float %float_0 %188 %float_0
        %191 =   OpCompositeConstruct %v3float %float_0 %float_0 %188
        %192 =   OpCompositeConstruct %mat3v3float %189 %190 %191
                 OpStore %184 %192
                 OpReturn
               OpFunctionEnd


               ; Function out_float4x4_vf44
%out_float4x4_vf44 = OpFunction %void None %104
        %193 = OpFunctionParameter %_ptr_Function_mat4v4float

        %194 = OpLabel
        %195 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %196 =   OpLoad %v4float %195               ; RelaxedPrecision
        %197 =   OpCompositeExtract %float %196 2   ; RelaxedPrecision
        %198 =   OpCompositeConstruct %v4float %197 %float_0 %float_0 %float_0
        %199 =   OpCompositeConstruct %v4float %float_0 %197 %float_0 %float_0
        %200 =   OpCompositeConstruct %v4float %float_0 %float_0 %197 %float_0
        %201 =   OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %197
        %202 =   OpCompositeConstruct %mat4v4float %198 %199 %200 %201
                 OpStore %193 %202
                 OpReturn
               OpFunctionEnd


               ; Function out_bool_vb
%out_bool_vb = OpFunction %void None %205
        %206 = OpFunctionParameter %_ptr_Function_bool

        %207 = OpLabel
        %208 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %209 =   OpLoad %v4float %208               ; RelaxedPrecision
        %210 =   OpCompositeExtract %float %209 0   ; RelaxedPrecision
        %211 =   OpFUnordNotEqual %bool %210 %float_0
                 OpStore %206 %211
                 OpReturn
               OpFunctionEnd


               ; Function out_bool2_vb2
%out_bool2_vb2 = OpFunction %void None %214
        %215 = OpFunctionParameter %_ptr_Function_v2bool

        %216 = OpLabel
        %217 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %218 =   OpLoad %v4float %217               ; RelaxedPrecision
        %219 =   OpCompositeExtract %float %218 1   ; RelaxedPrecision
        %220 =   OpFUnordNotEqual %bool %219 %float_0
        %221 =   OpCompositeConstruct %v2bool %220 %220
                 OpStore %215 %221
                 OpReturn
               OpFunctionEnd


               ; Function out_bool3_vb3
%out_bool3_vb3 = OpFunction %void None %224
        %225 = OpFunctionParameter %_ptr_Function_v3bool

        %226 = OpLabel
        %227 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %228 =   OpLoad %v4float %227               ; RelaxedPrecision
        %229 =   OpCompositeExtract %float %228 2   ; RelaxedPrecision
        %230 =   OpFUnordNotEqual %bool %229 %float_0
        %231 =   OpCompositeConstruct %v3bool %230 %230 %230
                 OpStore %225 %231
                 OpReturn
               OpFunctionEnd


               ; Function out_bool4_vb4
%out_bool4_vb4 = OpFunction %void None %234
        %235 = OpFunctionParameter %_ptr_Function_v4bool

        %236 = OpLabel
        %237 =   OpAccessChain %_ptr_Uniform_v4float %33 %int_2
        %238 =   OpLoad %v4float %237               ; RelaxedPrecision
        %239 =   OpCompositeExtract %float %238 3   ; RelaxedPrecision
        %240 =   OpFUnordNotEqual %bool %239 %float_0
        %241 =   OpCompositeConstruct %v4bool %240 %240 %240 %240
                 OpStore %235 %241
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %242        ; RelaxedPrecision
        %243 = OpFunctionParameter %_ptr_Function_v2float

        %244 = OpLabel
          %h =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
        %246 =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %h2 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
        %250 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
         %h3 =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
        %254 =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %h4 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %258 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %263 =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
        %266 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
        %271 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %h2x2 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %277 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
       %h3x3 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %281 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
       %h4x4 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %285 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %289 =   OpVariable %_ptr_Function_v3float Function         ; RelaxedPrecision
        %295 =   OpVariable %_ptr_Function_float Function           ; RelaxedPrecision
        %301 =   OpVariable %_ptr_Function_float Function           ; RelaxedPrecision
          %i =   OpVariable %_ptr_Function_int Function
        %305 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_v2int Function
        %309 =   OpVariable %_ptr_Function_v2int Function
         %i3 =   OpVariable %_ptr_Function_v3int Function
        %313 =   OpVariable %_ptr_Function_v3int Function
         %i4 =   OpVariable %_ptr_Function_v4int Function
        %317 =   OpVariable %_ptr_Function_v4int Function
        %320 =   OpVariable %_ptr_Function_v3int Function
        %326 =   OpVariable %_ptr_Function_int Function
          %f =   OpVariable %_ptr_Function_float Function
        %330 =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_v2float Function
        %334 =   OpVariable %_ptr_Function_v2float Function
         %f3 =   OpVariable %_ptr_Function_v3float Function
        %338 =   OpVariable %_ptr_Function_v3float Function
         %f4 =   OpVariable %_ptr_Function_v4float Function
        %342 =   OpVariable %_ptr_Function_v4float Function
        %345 =   OpVariable %_ptr_Function_v2float Function
        %351 =   OpVariable %_ptr_Function_float Function
       %f2x2 =   OpVariable %_ptr_Function_mat2v2float Function
        %355 =   OpVariable %_ptr_Function_mat2v2float Function
       %f3x3 =   OpVariable %_ptr_Function_mat3v3float Function
        %359 =   OpVariable %_ptr_Function_mat3v3float Function
       %f4x4 =   OpVariable %_ptr_Function_mat4v4float Function
        %363 =   OpVariable %_ptr_Function_mat4v4float Function
        %368 =   OpVariable %_ptr_Function_float Function
          %b =   OpVariable %_ptr_Function_bool Function
        %372 =   OpVariable %_ptr_Function_bool Function
         %b2 =   OpVariable %_ptr_Function_v2bool Function
        %376 =   OpVariable %_ptr_Function_v2bool Function
         %b3 =   OpVariable %_ptr_Function_v3bool Function
        %380 =   OpVariable %_ptr_Function_v3bool Function
         %b4 =   OpVariable %_ptr_Function_v4bool Function
        %384 =   OpVariable %_ptr_Function_v4bool Function
        %387 =   OpVariable %_ptr_Function_v2bool Function
        %393 =   OpVariable %_ptr_Function_bool Function
         %ok =   OpVariable %_ptr_Function_bool Function
        %485 =   OpVariable %_ptr_Function_v4float Function
        %247 =   OpFunctionCall %void %out_half_vh %246
        %248 =   OpLoad %float %246                 ; RelaxedPrecision
                 OpStore %h %248
        %251 =   OpFunctionCall %void %out_half2_vh2 %250
        %252 =   OpLoad %v2float %250               ; RelaxedPrecision
                 OpStore %h2 %252
        %255 =   OpFunctionCall %void %out_half3_vh3 %254
        %256 =   OpLoad %v3float %254               ; RelaxedPrecision
                 OpStore %h3 %256
        %259 =   OpFunctionCall %void %out_half4_vh4 %258
        %260 =   OpLoad %v4float %258               ; RelaxedPrecision
                 OpStore %h4 %260
        %261 =   OpAccessChain %_ptr_Function_float %h3 %int_1
        %264 =   OpFunctionCall %void %out_half_vh %263
        %265 =   OpLoad %float %263                 ; RelaxedPrecision
                 OpStore %261 %265
        %267 =   OpFunctionCall %void %out_half2_vh2 %266
        %268 =   OpLoad %v2float %266               ; RelaxedPrecision
        %269 =   OpLoad %v3float %h3                ; RelaxedPrecision
        %270 =   OpVectorShuffle %v3float %269 %268 3 1 4   ; RelaxedPrecision
                 OpStore %h3 %270
        %272 =   OpFunctionCall %void %out_half4_vh4 %271
        %273 =   OpLoad %v4float %271               ; RelaxedPrecision
        %274 =   OpLoad %v4float %h4                ; RelaxedPrecision
        %275 =   OpVectorShuffle %v4float %274 %273 6 7 4 5     ; RelaxedPrecision
                 OpStore %h4 %275
        %278 =   OpFunctionCall %void %out_half2x2_vh22 %277
        %279 =   OpLoad %mat2v2float %277           ; RelaxedPrecision
                 OpStore %h2x2 %279
        %282 =   OpFunctionCall %void %out_half3x3_vh33 %281
        %283 =   OpLoad %mat3v3float %281           ; RelaxedPrecision
                 OpStore %h3x3 %283
        %286 =   OpFunctionCall %void %out_half4x4_vh44 %285
        %287 =   OpLoad %mat4v4float %285           ; RelaxedPrecision
                 OpStore %h4x4 %287
        %288 =   OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
        %290 =   OpFunctionCall %void %out_half3_vh3 %289
        %291 =   OpLoad %v3float %289               ; RelaxedPrecision
                 OpStore %288 %291
        %293 =   OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
        %294 =   OpAccessChain %_ptr_Function_float %293 %int_3
        %296 =   OpFunctionCall %void %out_half_vh %295
        %297 =   OpLoad %float %295                 ; RelaxedPrecision
                 OpStore %294 %297
        %299 =   OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
        %300 =   OpAccessChain %_ptr_Function_float %299 %int_0
        %302 =   OpFunctionCall %void %out_half_vh %301
        %303 =   OpLoad %float %301                 ; RelaxedPrecision
                 OpStore %300 %303
        %306 =   OpFunctionCall %void %out_int_vi %305
        %307 =   OpLoad %int %305
                 OpStore %i %307
        %310 =   OpFunctionCall %void %out_int2_vi2 %309
        %311 =   OpLoad %v2int %309
                 OpStore %i2 %311
        %314 =   OpFunctionCall %void %out_int3_vi3 %313
        %315 =   OpLoad %v3int %313
                 OpStore %i3 %315
        %318 =   OpFunctionCall %void %out_int4_vi4 %317
        %319 =   OpLoad %v4int %317
                 OpStore %i4 %319
        %321 =   OpFunctionCall %void %out_int3_vi3 %320
        %322 =   OpLoad %v3int %320
        %323 =   OpLoad %v4int %i4
        %324 =   OpVectorShuffle %v4int %323 %322 4 5 6 3
                 OpStore %i4 %324
        %325 =   OpAccessChain %_ptr_Function_int %i2 %int_1
        %327 =   OpFunctionCall %void %out_int_vi %326
        %328 =   OpLoad %int %326
                 OpStore %325 %328
        %331 =   OpFunctionCall %void %out_float_vf %330
        %332 =   OpLoad %float %330
                 OpStore %f %332
        %335 =   OpFunctionCall %void %out_float2_vf2 %334
        %336 =   OpLoad %v2float %334
                 OpStore %f2 %336
        %339 =   OpFunctionCall %void %out_float3_vf3 %338
        %340 =   OpLoad %v3float %338
                 OpStore %f3 %340
        %343 =   OpFunctionCall %void %out_float4_vf4 %342
        %344 =   OpLoad %v4float %342
                 OpStore %f4 %344
        %346 =   OpFunctionCall %void %out_float2_vf2 %345
        %347 =   OpLoad %v2float %345
        %348 =   OpLoad %v3float %f3
        %349 =   OpVectorShuffle %v3float %348 %347 3 4 2
                 OpStore %f3 %349
        %350 =   OpAccessChain %_ptr_Function_float %f2 %int_0
        %352 =   OpFunctionCall %void %out_float_vf %351
        %353 =   OpLoad %float %351
                 OpStore %350 %353
        %356 =   OpFunctionCall %void %out_float2x2_vf22 %355
        %357 =   OpLoad %mat2v2float %355
                 OpStore %f2x2 %357
        %360 =   OpFunctionCall %void %out_float3x3_vf33 %359
        %361 =   OpLoad %mat3v3float %359
                 OpStore %f3x3 %361
        %364 =   OpFunctionCall %void %out_float4x4_vf44 %363
        %365 =   OpLoad %mat4v4float %363
                 OpStore %f4x4 %365
        %366 =   OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
        %367 =   OpAccessChain %_ptr_Function_float %366 %int_0
        %369 =   OpFunctionCall %void %out_float_vf %368
        %370 =   OpLoad %float %368
                 OpStore %367 %370
        %373 =   OpFunctionCall %void %out_bool_vb %372
        %374 =   OpLoad %bool %372
                 OpStore %b %374
        %377 =   OpFunctionCall %void %out_bool2_vb2 %376
        %378 =   OpLoad %v2bool %376
                 OpStore %b2 %378
        %381 =   OpFunctionCall %void %out_bool3_vb3 %380
        %382 =   OpLoad %v3bool %380
                 OpStore %b3 %382
        %385 =   OpFunctionCall %void %out_bool4_vb4 %384
        %386 =   OpLoad %v4bool %384
                 OpStore %b4 %386
        %388 =   OpFunctionCall %void %out_bool2_vb2 %387
        %389 =   OpLoad %v2bool %387
        %390 =   OpLoad %v4bool %b4
        %391 =   OpVectorShuffle %v4bool %390 %389 4 1 2 5
                 OpStore %b4 %391
        %392 =   OpAccessChain %_ptr_Function_bool %b3 %int_2
        %394 =   OpFunctionCall %void %out_bool_vb %393
        %395 =   OpLoad %bool %393
                 OpStore %392 %395
                 OpStore %ok %true
                 OpSelectionMerge %400 None
                 OpBranchConditional %true %399 %400

        %399 =     OpLabel
        %402 =       OpLoad %float %h               ; RelaxedPrecision
        %403 =       OpLoad %v2float %h2            ; RelaxedPrecision
        %404 =       OpCompositeExtract %float %403 0   ; RelaxedPrecision
        %405 =       OpFMul %float %402 %404            ; RelaxedPrecision
        %406 =       OpLoad %v3float %h3                ; RelaxedPrecision
        %407 =       OpCompositeExtract %float %406 0   ; RelaxedPrecision
        %408 =       OpFMul %float %405 %407            ; RelaxedPrecision
        %409 =       OpLoad %v4float %h4                ; RelaxedPrecision
        %410 =       OpCompositeExtract %float %409 0   ; RelaxedPrecision
        %411 =       OpFMul %float %408 %410            ; RelaxedPrecision
        %412 =       OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
        %413 =       OpLoad %v2float %412           ; RelaxedPrecision
        %414 =       OpCompositeExtract %float %413 0   ; RelaxedPrecision
        %415 =       OpFMul %float %411 %414            ; RelaxedPrecision
        %416 =       OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
        %417 =       OpLoad %v3float %416           ; RelaxedPrecision
        %418 =       OpCompositeExtract %float %417 0   ; RelaxedPrecision
        %419 =       OpFMul %float %415 %418            ; RelaxedPrecision
        %420 =       OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
        %421 =       OpLoad %v4float %420           ; RelaxedPrecision
        %422 =       OpCompositeExtract %float %421 0   ; RelaxedPrecision
        %423 =       OpFMul %float %419 %422            ; RelaxedPrecision
        %424 =       OpFOrdEqual %bool %float_1 %423
                     OpBranch %400

        %400 = OpLabel
        %425 =   OpPhi %bool %false %244 %424 %399
                 OpStore %ok %425
                 OpSelectionMerge %427 None
                 OpBranchConditional %425 %426 %427

        %426 =     OpLabel
        %428 =       OpLoad %float %f
        %429 =       OpLoad %v2float %f2
        %430 =       OpCompositeExtract %float %429 0
        %431 =       OpFMul %float %428 %430
        %432 =       OpLoad %v3float %f3
        %433 =       OpCompositeExtract %float %432 0
        %434 =       OpFMul %float %431 %433
        %435 =       OpLoad %v4float %f4
        %436 =       OpCompositeExtract %float %435 0
        %437 =       OpFMul %float %434 %436
        %438 =       OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
        %439 =       OpLoad %v2float %438
        %440 =       OpCompositeExtract %float %439 0
        %441 =       OpFMul %float %437 %440
        %442 =       OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
        %443 =       OpLoad %v3float %442
        %444 =       OpCompositeExtract %float %443 0
        %445 =       OpFMul %float %441 %444
        %446 =       OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
        %447 =       OpLoad %v4float %446
        %448 =       OpCompositeExtract %float %447 0
        %449 =       OpFMul %float %445 %448
        %450 =       OpFOrdEqual %bool %float_1 %449
                     OpBranch %427

        %427 = OpLabel
        %451 =   OpPhi %bool %false %400 %450 %426
                 OpStore %ok %451
                 OpSelectionMerge %453 None
                 OpBranchConditional %451 %452 %453

        %452 =     OpLabel
        %454 =       OpLoad %int %i
        %455 =       OpLoad %v2int %i2
        %456 =       OpCompositeExtract %int %455 0
        %457 =       OpIMul %int %454 %456
        %458 =       OpLoad %v3int %i3
        %459 =       OpCompositeExtract %int %458 0
        %460 =       OpIMul %int %457 %459
        %461 =       OpLoad %v4int %i4
        %462 =       OpCompositeExtract %int %461 0
        %463 =       OpIMul %int %460 %462
        %464 =       OpIEqual %bool %int_1 %463
                     OpBranch %453

        %453 = OpLabel
        %465 =   OpPhi %bool %false %427 %464 %452
                 OpStore %ok %465
                 OpSelectionMerge %467 None
                 OpBranchConditional %465 %466 %467

        %466 =     OpLabel
        %468 =       OpLoad %bool %b                ; RelaxedPrecision
                     OpSelectionMerge %470 None
                     OpBranchConditional %468 %469 %470

        %469 =         OpLabel
        %471 =           OpLoad %v2bool %b2         ; RelaxedPrecision
        %472 =           OpCompositeExtract %bool %471 0
                         OpBranch %470

        %470 =     OpLabel
        %473 =       OpPhi %bool %false %466 %472 %469
                     OpSelectionMerge %475 None
                     OpBranchConditional %473 %474 %475

        %474 =         OpLabel
        %476 =           OpLoad %v3bool %b3         ; RelaxedPrecision
        %477 =           OpCompositeExtract %bool %476 0
                         OpBranch %475

        %475 =     OpLabel
        %478 =       OpPhi %bool %false %470 %477 %474
                     OpSelectionMerge %480 None
                     OpBranchConditional %478 %479 %480

        %479 =         OpLabel
        %481 =           OpLoad %v4bool %b4         ; RelaxedPrecision
        %482 =           OpCompositeExtract %bool %481 0
                         OpBranch %480

        %480 =     OpLabel
        %483 =       OpPhi %bool %false %475 %482 %479
                     OpBranch %467

        %467 = OpLabel
        %484 =   OpPhi %bool %false %453 %483 %480
                 OpStore %ok %484
                 OpSelectionMerge %488 None
                 OpBranchConditional %484 %486 %487

        %486 =     OpLabel
        %489 =       OpAccessChain %_ptr_Uniform_v4float %33 %int_0
        %490 =       OpLoad %v4float %489           ; RelaxedPrecision
                     OpStore %485 %490
                     OpBranch %488

        %487 =     OpLabel
        %491 =       OpAccessChain %_ptr_Uniform_v4float %33 %int_1
        %492 =       OpLoad %v4float %491           ; RelaxedPrecision
                     OpStore %485 %492
                     OpBranch %488

        %488 = OpLabel
        %493 =   OpLoad %v4float %485               ; RelaxedPrecision
                 OpReturnValue %493
               OpFunctionEnd
