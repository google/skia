               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %b "b"
               OpName %b4 "b4"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
    %float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
          %b = OpVariable %_ptr_Function_bool Function
         %b4 = OpVariable %_ptr_Function_v4bool Function
         %26 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %30 = OpLoad %float %26
         %31 = OpFUnordNotEqual %bool %30 %float_0
               OpStore %b %31
         %35 = OpCompositeConstruct %v4bool %31 %31 %31 %31
               OpStore %b4 %35
         %37 = OpCompositeConstruct %v2bool %31 %31
         %38 = OpCompositeExtract %bool %37 0
         %39 = OpCompositeExtract %bool %37 1
         %42 = OpCompositeConstruct %v4bool %38 %39 %false %true
               OpStore %b4 %42
         %43 = OpCompositeConstruct %v4bool %false %31 %true %false
               OpStore %b4 %43
         %44 = OpCompositeConstruct %v4bool %false %31 %false %31
               OpStore %b4 %44
         %45 = OpSelect %float %false %float_1 %float_0
         %47 = OpCompositeExtract %bool %44 1
         %48 = OpSelect %float %47 %float_1 %float_0
         %49 = OpCompositeExtract %bool %44 2
         %50 = OpSelect %float %49 %float_1 %float_0
         %51 = OpCompositeExtract %bool %44 3
         %52 = OpSelect %float %51 %float_1 %float_0
         %53 = OpCompositeConstruct %v4float %45 %48 %50 %52
               OpReturnValue %53
               OpFunctionEnd
