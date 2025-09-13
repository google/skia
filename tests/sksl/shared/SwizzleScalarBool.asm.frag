               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %b "b"                            ; id %27
               OpName %b4 "b4"                          ; id %35

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision

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
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
    %float_1 = OpConstant %float 1


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
          %b =   OpVariable %_ptr_Function_bool Function
         %b4 =   OpVariable %_ptr_Function_v4bool Function
         %30 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %33 =   OpLoad %float %30                  ; RelaxedPrecision
         %34 =   OpFUnordNotEqual %bool %33 %float_0
                 OpStore %b %34
         %38 =   OpCompositeConstruct %v4bool %34 %34 %34 %34
                 OpStore %b4 %38
         %40 =   OpCompositeConstruct %v2bool %34 %34
         %41 =   OpCompositeExtract %bool %40 0
         %42 =   OpCompositeExtract %bool %40 1
         %45 =   OpCompositeConstruct %v4bool %41 %42 %false %true
                 OpStore %b4 %45
         %46 =   OpCompositeConstruct %v4bool %false %34 %true %false
                 OpStore %b4 %46
         %47 =   OpCompositeConstruct %v4bool %false %34 %false %34
                 OpStore %b4 %47
         %48 =   OpSelect %float %false %float_1 %float_0   ; RelaxedPrecision
         %50 =   OpCompositeExtract %bool %47 1
         %51 =   OpSelect %float %50 %float_1 %float_0  ; RelaxedPrecision
         %52 =   OpCompositeExtract %bool %47 2
         %53 =   OpSelect %float %52 %float_1 %float_0  ; RelaxedPrecision
         %54 =   OpCompositeExtract %bool %47 3
         %55 =   OpSelect %float %54 %float_1 %float_0  ; RelaxedPrecision
         %56 =   OpCompositeConstruct %v4float %48 %51 %53 %55  ; RelaxedPrecision
                 OpReturnValue %56
               OpFunctionEnd
