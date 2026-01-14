               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpName %main "main"                  ; id %6
               OpName %b1 "b1"                      ; id %18
               OpName %b2 "b2"                      ; id %31
               OpName %b3 "b3"                      ; id %44
               OpName %b4 "b4"                      ; id %57

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %int %uint           ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
     %v2uint = OpTypeVector %uint 2
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
     %v3uint = OpTypeVector %uint 3
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %v4uint = OpTypeVector %uint 4


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %b1 =   OpVariable %_ptr_Function_int Function
         %b2 =   OpVariable %_ptr_Function_v2int Function
         %b3 =   OpVariable %_ptr_Function_v3int Function
         %b4 =   OpVariable %_ptr_Function_v4int Function
         %21 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %24 =   OpLoad %int %21
         %20 =   OpBitCount %int %24
         %26 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %29 =   OpLoad %uint %26
         %25 =   OpBitCount %int %29
         %30 =   OpIAdd %int %20 %25
                 OpStore %b1 %30
         %35 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %36 =   OpLoad %int %35
         %37 =   OpCompositeConstruct %v2int %36 %36
         %34 =   OpBitCount %v2int %37
         %39 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %40 =   OpLoad %uint %39
         %42 =   OpCompositeConstruct %v2uint %40 %40
         %38 =   OpBitCount %v2int %42
         %43 =   OpIAdd %v2int %34 %38
                 OpStore %b2 %43
         %48 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %49 =   OpLoad %int %48
         %50 =   OpCompositeConstruct %v3int %49 %49 %49
         %47 =   OpBitCount %v3int %50
         %52 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %53 =   OpLoad %uint %52
         %55 =   OpCompositeConstruct %v3uint %53 %53 %53
         %51 =   OpBitCount %v3int %55
         %56 =   OpIAdd %v3int %47 %51
                 OpStore %b3 %56
         %61 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %62 =   OpLoad %int %61
         %63 =   OpCompositeConstruct %v4int %62 %62 %62 %62
         %60 =   OpBitCount %v4int %63
         %65 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %66 =   OpLoad %uint %65
         %68 =   OpCompositeConstruct %v4uint %66 %66 %66 %66
         %64 =   OpBitCount %v4int %68
         %69 =   OpIAdd %v4int %60 %64
                 OpStore %b4 %69
         %70 =   OpCompositeConstruct %v4int %30 %30 %30 %30
         %71 =   OpVectorShuffle %v4int %43 %43 0 1 0 1
         %72 =   OpIAdd %v4int %70 %71
         %73 =   OpCompositeExtract %int %56 0
         %74 =   OpCompositeExtract %int %56 1
         %75 =   OpCompositeExtract %int %56 2
         %76 =   OpCompositeConstruct %v4int %73 %74 %75 %int_1
         %77 =   OpIAdd %v4int %72 %76
         %78 =   OpIAdd %v4int %77 %69
         %79 =   OpCompositeExtract %int %78 0
         %80 =   OpConvertSToF %float %79           ; RelaxedPrecision
         %81 =   OpCompositeExtract %int %78 1
         %82 =   OpConvertSToF %float %81           ; RelaxedPrecision
         %83 =   OpCompositeExtract %int %78 2
         %84 =   OpConvertSToF %float %83           ; RelaxedPrecision
         %85 =   OpCompositeExtract %int %78 3
         %86 =   OpConvertSToF %float %85           ; RelaxedPrecision
         %87 =   OpCompositeConstruct %v4float %80 %82 %84 %86  ; RelaxedPrecision
                 OpStore %sk_FragColor %87
                 OpReturn
               OpFunctionEnd
