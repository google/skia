               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %fnGreen_h4bf2 "fnGreen_h4bf2"
               OpName %S "S"
               OpMemberName %S 0 "i"
               OpName %fnRed_h4ifS "fnRed_h4ifS"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %44 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
         %24 = OpTypeFunction %v4float %_ptr_Function_bool %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
          %S = OpTypeStruct %int
%_ptr_Function_S = OpTypePointer Function %S
         %37 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_float %_ptr_Function_S
      %int_1 = OpConstant %int 1
         %45 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %true = OpConstantTrue %bool
    %int_123 = OpConstant %int 123
%float_3_1400001 = OpConstant %float 3.1400001
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%fnGreen_h4bf2 = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_bool
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
         %28 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %32 = OpLoad %v4float %28
               OpReturnValue %32
               OpFunctionEnd
%fnRed_h4ifS = OpFunction %v4float None %37
         %38 = OpFunctionParameter %_ptr_Function_int
         %39 = OpFunctionParameter %_ptr_Function_float
         %40 = OpFunctionParameter %_ptr_Function_S
         %41 = OpLabel
         %42 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %44 = OpLoad %v4float %42
               OpReturnValue %44
               OpFunctionEnd
       %main = OpFunction %v4float None %45
         %46 = OpFunctionParameter %_ptr_Function_v2float
         %47 = OpLabel
         %52 = OpVariable %_ptr_Function_v4float Function
         %58 = OpVariable %_ptr_Function_bool Function
         %60 = OpVariable %_ptr_Function_v2float Function
         %63 = OpVariable %_ptr_Function_int Function
         %65 = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_S Function
         %48 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpCompositeExtract %float %49 1
         %51 = OpFUnordNotEqual %bool %50 %float_0
               OpSelectionMerge %56 None
               OpBranchConditional %51 %54 %55
         %54 = OpLabel
               OpStore %58 %true
         %59 = OpLoad %v2float %46
               OpStore %60 %59
         %61 = OpFunctionCall %v4float %fnGreen_h4bf2 %58 %60
               OpStore %52 %61
               OpBranch %56
         %55 = OpLabel
               OpStore %63 %int_123
               OpStore %65 %float_3_1400001
         %66 = OpCompositeConstruct %S %int_0
               OpStore %67 %66
         %68 = OpFunctionCall %v4float %fnRed_h4ifS %63 %65 %67
               OpStore %52 %68
               OpBranch %56
         %56 = OpLabel
         %69 = OpLoad %v4float %52
               OpReturnValue %69
               OpFunctionEnd
