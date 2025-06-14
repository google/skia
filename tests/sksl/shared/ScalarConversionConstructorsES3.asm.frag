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
               OpName %f "f"                            ; id %27
               OpName %i "i"                            ; id %34
               OpName %u "u"                            ; id %40
               OpName %b "b"                            ; id %47
               OpName %f1 "f1"                          ; id %54
               OpName %f2 "f2"                          ; id %55
               OpName %f3 "f3"                          ; id %57
               OpName %f4 "f4"                          ; id %59
               OpName %i1 "i1"                          ; id %62
               OpName %i2 "i2"                          ; id %64
               OpName %i3 "i3"                          ; id %65
               OpName %i4 "i4"                          ; id %67
               OpName %u1 "u1"                          ; id %70
               OpName %u2 "u2"                          ; id %72
               OpName %u3 "u3"                          ; id %74
               OpName %u4 "u4"                          ; id %75
               OpName %b1 "b1"                          ; id %79
               OpName %b2 "b2"                          ; id %81
               OpName %b3 "b3"                          ; id %83
               OpName %b4 "b4"                          ; id %85

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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision

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
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
   %float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float


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
          %f =   OpVariable %_ptr_Function_float Function
          %i =   OpVariable %_ptr_Function_int Function
          %u =   OpVariable %_ptr_Function_uint Function
          %b =   OpVariable %_ptr_Function_bool Function
         %f1 =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_float Function
         %f3 =   OpVariable %_ptr_Function_float Function
         %f4 =   OpVariable %_ptr_Function_float Function
         %i1 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_int Function
         %i3 =   OpVariable %_ptr_Function_int Function
         %i4 =   OpVariable %_ptr_Function_int Function
         %u1 =   OpVariable %_ptr_Function_uint Function
         %u2 =   OpVariable %_ptr_Function_uint Function
         %u3 =   OpVariable %_ptr_Function_uint Function
         %u4 =   OpVariable %_ptr_Function_uint Function
         %b1 =   OpVariable %_ptr_Function_bool Function
         %b2 =   OpVariable %_ptr_Function_bool Function
         %b3 =   OpVariable %_ptr_Function_bool Function
         %b4 =   OpVariable %_ptr_Function_bool Function
        %115 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
                 OpStore %f %33
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %37 =   OpLoad %v4float %36                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %39 =   OpConvertFToS %int %38
                 OpStore %i %39
         %43 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =   OpLoad %v4float %43                ; RelaxedPrecision
         %45 =   OpCompositeExtract %float %44 1    ; RelaxedPrecision
         %46 =   OpConvertFToU %uint %45
                 OpStore %u %46
         %50 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %51 =   OpLoad %v4float %50                ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 1    ; RelaxedPrecision
         %53 =   OpFUnordNotEqual %bool %52 %float_0
                 OpStore %b %53
                 OpStore %f1 %33
         %56 =   OpConvertSToF %float %39
                 OpStore %f2 %56
         %58 =   OpConvertUToF %float %46
                 OpStore %f3 %58
         %60 =   OpSelect %float %53 %float_1 %float_0
                 OpStore %f4 %60
         %63 =   OpConvertFToS %int %33
                 OpStore %i1 %63
                 OpStore %i2 %39
         %66 =   OpBitcast %int %46
                 OpStore %i3 %66
         %68 =   OpSelect %int %53 %int_1 %int_0
                 OpStore %i4 %68
         %71 =   OpConvertFToU %uint %33
                 OpStore %u1 %71
         %73 =   OpBitcast %uint %39
                 OpStore %u2 %73
                 OpStore %u3 %46
         %76 =   OpSelect %uint %53 %uint_1 %uint_0
                 OpStore %u4 %76
         %80 =   OpFUnordNotEqual %bool %33 %float_0
                 OpStore %b1 %80
         %82 =   OpINotEqual %bool %39 %int_0
                 OpStore %b2 %82
         %84 =   OpINotEqual %bool %46 %uint_0
                 OpStore %b3 %84
                 OpStore %b4 %53
         %86 =   OpFAdd %float %33 %56              ; RelaxedPrecision
         %87 =   OpFAdd %float %86 %58              ; RelaxedPrecision
         %88 =   OpFAdd %float %87 %60              ; RelaxedPrecision
         %89 =   OpConvertSToF %float %63           ; RelaxedPrecision
         %90 =   OpFAdd %float %88 %89              ; RelaxedPrecision
         %91 =   OpConvertSToF %float %39           ; RelaxedPrecision
         %92 =   OpFAdd %float %90 %91              ; RelaxedPrecision
         %93 =   OpConvertSToF %float %66           ; RelaxedPrecision
         %94 =   OpFAdd %float %92 %93              ; RelaxedPrecision
         %95 =   OpConvertSToF %float %68           ; RelaxedPrecision
         %96 =   OpFAdd %float %94 %95              ; RelaxedPrecision
         %97 =   OpConvertUToF %float %71           ; RelaxedPrecision
         %98 =   OpFAdd %float %96 %97              ; RelaxedPrecision
         %99 =   OpConvertUToF %float %73           ; RelaxedPrecision
        %100 =   OpFAdd %float %98 %99              ; RelaxedPrecision
        %101 =   OpConvertUToF %float %46           ; RelaxedPrecision
        %102 =   OpFAdd %float %100 %101            ; RelaxedPrecision
        %103 =   OpConvertUToF %float %76           ; RelaxedPrecision
        %104 =   OpFAdd %float %102 %103            ; RelaxedPrecision
        %105 =   OpSelect %float %80 %float_1 %float_0  ; RelaxedPrecision
        %106 =   OpFAdd %float %104 %105                ; RelaxedPrecision
        %107 =   OpSelect %float %82 %float_1 %float_0  ; RelaxedPrecision
        %108 =   OpFAdd %float %106 %107                ; RelaxedPrecision
        %109 =   OpSelect %float %84 %float_1 %float_0  ; RelaxedPrecision
        %110 =   OpFAdd %float %108 %109                ; RelaxedPrecision
        %111 =   OpSelect %float %53 %float_1 %float_0  ; RelaxedPrecision
        %112 =   OpFAdd %float %110 %111                ; RelaxedPrecision
        %114 =   OpFOrdEqual %bool %112 %float_16
                 OpSelectionMerge %119 None
                 OpBranchConditional %114 %117 %118

        %117 =     OpLabel
        %120 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %121 =       OpLoad %v4float %120           ; RelaxedPrecision
                     OpStore %115 %121
                     OpBranch %119

        %118 =     OpLabel
        %122 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %123 =       OpLoad %v4float %122           ; RelaxedPrecision
                     OpStore %115 %123
                     OpBranch %119

        %119 = OpLabel
        %124 =   OpLoad %v4float %115               ; RelaxedPrecision
                 OpReturnValue %124
               OpFunctionEnd
