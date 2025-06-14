               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 "check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4"    ; id %6
               OpName %main "main"                                                                                  ; id %7
               OpName %v1 "v1"                                                                                      ; id %126
               OpName %v2 "v2"                                                                                      ; id %128
               OpName %v3 "v3"                                                                                      ; id %131
               OpName %v4 "v4"                                                                                      ; id %132
               OpName %v5 "v5"                                                                                      ; id %134
               OpName %v6 "v6"                                                                                      ; id %137
               OpName %v7 "v7"                                                                                      ; id %140
               OpName %v8 "v8"                                                                                      ; id %141
               OpName %v9 "v9"                                                                                      ; id %145
               OpName %v10 "v10"                                                                                    ; id %153
               OpName %v11 "v11"                                                                                    ; id %157
               OpName %v12 "v12"                                                                                    ; id %161
               OpName %v13 "v13"                                                                                    ; id %163
               OpName %v14 "v14"                                                                                    ; id %164
               OpName %v15 "v15"                                                                                    ; id %165
               OpName %v16 "v16"                                                                                    ; id %167
               OpName %v17 "v17"                                                                                    ; id %168
               OpName %v18 "v18"                                                                                    ; id %170

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %63 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
         %39 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool %_ptr_Function_v4int
    %float_1 = OpConstant %float 1
   %float_18 = OpConstant %float 18
        %123 = OpTypeFunction %v4float %_ptr_Function_v2float
        %127 = OpConstantComposite %v2float %float_1 %float_1
    %float_2 = OpConstant %float 2
        %130 = OpConstantComposite %v2float %float_1 %float_2
        %133 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
        %136 = OpConstantComposite %v2int %int_1 %int_1
      %int_2 = OpConstant %int 2
        %139 = OpConstantComposite %v2int %int_1 %int_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
        %160 = OpConstantComposite %v4bool %true %false %true %false
        %162 = OpConstantComposite %v2float %float_1 %float_0
        %166 = OpConstantComposite %v2bool %true %true
        %169 = OpConstantComposite %v3bool %true %true %true
        %171 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 = OpFunction %bool None %39
         %40 = OpFunctionParameter %_ptr_Function_v2float
         %41 = OpFunctionParameter %_ptr_Function_v2float
         %42 = OpFunctionParameter %_ptr_Function_v2float
         %43 = OpFunctionParameter %_ptr_Function_v3float
         %44 = OpFunctionParameter %_ptr_Function_v2int
         %45 = OpFunctionParameter %_ptr_Function_v2int
         %46 = OpFunctionParameter %_ptr_Function_v2float
         %47 = OpFunctionParameter %_ptr_Function_v2float
         %48 = OpFunctionParameter %_ptr_Function_v4float
         %49 = OpFunctionParameter %_ptr_Function_v2int
         %50 = OpFunctionParameter %_ptr_Function_v4bool
         %51 = OpFunctionParameter %_ptr_Function_v2float
         %52 = OpFunctionParameter %_ptr_Function_v2float
         %53 = OpFunctionParameter %_ptr_Function_v2float
         %54 = OpFunctionParameter %_ptr_Function_v2bool
         %55 = OpFunctionParameter %_ptr_Function_v2bool
         %56 = OpFunctionParameter %_ptr_Function_v3bool
         %57 = OpFunctionParameter %_ptr_Function_v4int

         %58 = OpLabel
         %59 =   OpLoad %v2float %40
         %60 =   OpCompositeExtract %float %59 0
         %61 =   OpLoad %v2float %41
         %62 =   OpCompositeExtract %float %61 0
         %63 =   OpFAdd %float %60 %62              ; RelaxedPrecision
         %64 =   OpLoad %v2float %42
         %65 =   OpCompositeExtract %float %64 0
         %66 =   OpFAdd %float %63 %65              ; RelaxedPrecision
         %67 =   OpLoad %v3float %43
         %68 =   OpCompositeExtract %float %67 0
         %69 =   OpFAdd %float %66 %68              ; RelaxedPrecision
         %70 =   OpLoad %v2int %44
         %71 =   OpCompositeExtract %int %70 0
         %72 =   OpConvertSToF %float %71           ; RelaxedPrecision
         %73 =   OpFAdd %float %69 %72              ; RelaxedPrecision
         %74 =   OpLoad %v2int %45
         %75 =   OpCompositeExtract %int %74 0
         %76 =   OpConvertSToF %float %75           ; RelaxedPrecision
         %77 =   OpFAdd %float %73 %76              ; RelaxedPrecision
         %78 =   OpLoad %v2float %46
         %79 =   OpCompositeExtract %float %78 0
         %80 =   OpFAdd %float %77 %79              ; RelaxedPrecision
         %81 =   OpLoad %v2float %47
         %82 =   OpCompositeExtract %float %81 0
         %83 =   OpFAdd %float %80 %82              ; RelaxedPrecision
         %84 =   OpLoad %v4float %48
         %85 =   OpCompositeExtract %float %84 0
         %86 =   OpFAdd %float %83 %85              ; RelaxedPrecision
         %87 =   OpLoad %v2int %49
         %88 =   OpCompositeExtract %int %87 0
         %89 =   OpConvertSToF %float %88           ; RelaxedPrecision
         %90 =   OpFAdd %float %86 %89              ; RelaxedPrecision
         %91 =   OpLoad %v4bool %50                 ; RelaxedPrecision
         %92 =   OpCompositeExtract %bool %91 0
         %93 =   OpSelect %float %92 %float_1 %float_0  ; RelaxedPrecision
         %95 =   OpFAdd %float %90 %93                  ; RelaxedPrecision
         %96 =   OpLoad %v2float %51
         %97 =   OpCompositeExtract %float %96 0
         %98 =   OpFAdd %float %95 %97              ; RelaxedPrecision
         %99 =   OpLoad %v2float %52
        %100 =   OpCompositeExtract %float %99 0
        %101 =   OpFAdd %float %98 %100             ; RelaxedPrecision
        %102 =   OpLoad %v2float %53
        %103 =   OpCompositeExtract %float %102 0
        %104 =   OpFAdd %float %101 %103            ; RelaxedPrecision
        %105 =   OpLoad %v2bool %54                 ; RelaxedPrecision
        %106 =   OpCompositeExtract %bool %105 0
        %107 =   OpSelect %float %106 %float_1 %float_0     ; RelaxedPrecision
        %108 =   OpFAdd %float %104 %107                    ; RelaxedPrecision
        %109 =   OpLoad %v2bool %55                         ; RelaxedPrecision
        %110 =   OpCompositeExtract %bool %109 0
        %111 =   OpSelect %float %110 %float_1 %float_0     ; RelaxedPrecision
        %112 =   OpFAdd %float %108 %111                    ; RelaxedPrecision
        %113 =   OpLoad %v3bool %56                         ; RelaxedPrecision
        %114 =   OpCompositeExtract %bool %113 0
        %115 =   OpSelect %float %114 %float_1 %float_0     ; RelaxedPrecision
        %116 =   OpFAdd %float %112 %115                    ; RelaxedPrecision
        %117 =   OpLoad %v4int %57
        %118 =   OpCompositeExtract %int %117 0
        %119 =   OpConvertSToF %float %118          ; RelaxedPrecision
        %120 =   OpFAdd %float %116 %119            ; RelaxedPrecision
        %122 =   OpFOrdEqual %bool %120 %float_18
                 OpReturnValue %122
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %123        ; RelaxedPrecision
        %124 = OpFunctionParameter %_ptr_Function_v2float

        %125 = OpLabel
         %v1 =   OpVariable %_ptr_Function_v2float Function
         %v2 =   OpVariable %_ptr_Function_v2float Function
         %v3 =   OpVariable %_ptr_Function_v2float Function
         %v4 =   OpVariable %_ptr_Function_v3float Function
         %v5 =   OpVariable %_ptr_Function_v2int Function
         %v6 =   OpVariable %_ptr_Function_v2int Function
         %v7 =   OpVariable %_ptr_Function_v2float Function
         %v8 =   OpVariable %_ptr_Function_v2float Function
         %v9 =   OpVariable %_ptr_Function_v4float Function
        %v10 =   OpVariable %_ptr_Function_v2int Function
        %v11 =   OpVariable %_ptr_Function_v4bool Function
        %v12 =   OpVariable %_ptr_Function_v2float Function
        %v13 =   OpVariable %_ptr_Function_v2float Function
        %v14 =   OpVariable %_ptr_Function_v2float Function
        %v15 =   OpVariable %_ptr_Function_v2bool Function
        %v16 =   OpVariable %_ptr_Function_v2bool Function
        %v17 =   OpVariable %_ptr_Function_v3bool Function
        %v18 =   OpVariable %_ptr_Function_v4int Function
        %172 =   OpVariable %_ptr_Function_v2float Function
        %173 =   OpVariable %_ptr_Function_v2float Function
        %174 =   OpVariable %_ptr_Function_v2float Function
        %175 =   OpVariable %_ptr_Function_v3float Function
        %176 =   OpVariable %_ptr_Function_v2int Function
        %177 =   OpVariable %_ptr_Function_v2int Function
        %178 =   OpVariable %_ptr_Function_v2float Function
        %179 =   OpVariable %_ptr_Function_v2float Function
        %180 =   OpVariable %_ptr_Function_v4float Function
        %181 =   OpVariable %_ptr_Function_v2int Function
        %182 =   OpVariable %_ptr_Function_v4bool Function
        %183 =   OpVariable %_ptr_Function_v2float Function
        %184 =   OpVariable %_ptr_Function_v2float Function
        %185 =   OpVariable %_ptr_Function_v2float Function
        %186 =   OpVariable %_ptr_Function_v2bool Function
        %187 =   OpVariable %_ptr_Function_v2bool Function
        %188 =   OpVariable %_ptr_Function_v3bool Function
        %189 =   OpVariable %_ptr_Function_v4int Function
        %191 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %v1 %127
                 OpStore %v2 %130
                 OpStore %v3 %127
                 OpStore %v4 %133
                 OpStore %v5 %136
                 OpStore %v6 %139
                 OpStore %v7 %130
        %142 =   OpConvertSToF %float %int_1
        %143 =   OpConvertSToF %float %int_1
        %144 =   OpCompositeConstruct %v2float %142 %143
                 OpStore %v8 %144
        %146 =   OpConvertSToF %float %int_1
        %147 =   OpAccessChain %_ptr_Uniform_float %12 %int_2
        %149 =   OpLoad %float %147
        %152 =   OpCompositeConstruct %v4float %146 %149 %float_3 %float_4
                 OpStore %v9 %152
        %155 =   OpConvertFToS %int %float_1
        %156 =   OpCompositeConstruct %v2int %int_3 %155
                 OpStore %v10 %156
                 OpStore %v11 %160
                 OpStore %v12 %162
                 OpStore %v13 %21
                 OpStore %v14 %21
                 OpStore %v15 %166
                 OpStore %v16 %166
                 OpStore %v17 %169
                 OpStore %v18 %171
                 OpStore %172 %127
                 OpStore %173 %130
                 OpStore %174 %127
                 OpStore %175 %133
                 OpStore %176 %136
                 OpStore %177 %139
                 OpStore %178 %130
                 OpStore %179 %144
                 OpStore %180 %152
                 OpStore %181 %156
                 OpStore %182 %160
                 OpStore %183 %162
                 OpStore %184 %21
                 OpStore %185 %21
                 OpStore %186 %166
                 OpStore %187 %166
                 OpStore %188 %169
                 OpStore %189 %171
        %190 =   OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 %172 %173 %174 %175 %176 %177 %178 %179 %180 %181 %182 %183 %184 %185 %186 %187 %188 %189
                 OpSelectionMerge %194 None
                 OpBranchConditional %190 %192 %193

        %192 =     OpLabel
        %195 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %198 =       OpLoad %v4float %195           ; RelaxedPrecision
                     OpStore %191 %198
                     OpBranch %194

        %193 =     OpLabel
        %199 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %200 =       OpLoad %v4float %199           ; RelaxedPrecision
                     OpStore %191 %200
                     OpBranch %194

        %194 = OpLabel
        %201 =   OpLoad %v4float %191               ; RelaxedPrecision
                 OpReturnValue %201
               OpFunctionEnd
