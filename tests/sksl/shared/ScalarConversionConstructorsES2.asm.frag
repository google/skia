               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f "f"
               OpName %i "i"
               OpName %b "b"
               OpName %f1 "f1"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %i1 "i1"
               OpName %i2 "i2"
               OpName %i3 "i3"
               OpName %b1 "b1"
               OpName %b2 "b2"
               OpName %b3 "b3"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
          %f = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_bool Function
         %f1 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_float Function
         %f3 = OpVariable %_ptr_Function_float Function
         %i1 = OpVariable %_ptr_Function_int Function
         %i2 = OpVariable %_ptr_Function_int Function
         %i3 = OpVariable %_ptr_Function_int Function
         %b1 = OpVariable %_ptr_Function_bool Function
         %b2 = OpVariable %_ptr_Function_bool Function
         %b3 = OpVariable %_ptr_Function_bool Function
         %79 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpCompositeExtract %float %32 1
               OpStore %f %33
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %37 = OpLoad %v4float %36
         %38 = OpCompositeExtract %float %37 1
         %39 = OpConvertFToS %int %38
               OpStore %i %39
         %42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpCompositeExtract %float %43 1
         %45 = OpFUnordNotEqual %bool %44 %float_0
               OpStore %b %45
               OpStore %f1 %33
         %48 = OpConvertSToF %float %39
               OpStore %f2 %48
         %50 = OpSelect %float %45 %float_1 %float_0
               OpStore %f3 %50
         %53 = OpConvertFToS %int %33
               OpStore %i1 %53
               OpStore %i2 %39
         %56 = OpSelect %int %45 %int_1 %int_0
               OpStore %i3 %56
         %59 = OpFUnordNotEqual %bool %33 %float_0
               OpStore %b1 %59
         %61 = OpINotEqual %bool %39 %int_0
               OpStore %b2 %61
               OpStore %b3 %45
         %63 = OpFAdd %float %33 %48
         %64 = OpFAdd %float %63 %50
         %65 = OpConvertSToF %float %53
         %66 = OpFAdd %float %64 %65
         %67 = OpConvertSToF %float %39
         %68 = OpFAdd %float %66 %67
         %69 = OpConvertSToF %float %56
         %70 = OpFAdd %float %68 %69
         %71 = OpSelect %float %59 %float_1 %float_0
         %72 = OpFAdd %float %70 %71
         %73 = OpSelect %float %61 %float_1 %float_0
         %74 = OpFAdd %float %72 %73
         %75 = OpSelect %float %45 %float_1 %float_0
         %76 = OpFAdd %float %74 %75
         %78 = OpFOrdEqual %bool %76 %float_9
               OpSelectionMerge %83 None
               OpBranchConditional %78 %81 %82
         %81 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %85 = OpLoad %v4float %84
               OpStore %79 %85
               OpBranch %83
         %82 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %87 = OpLoad %v4float %86
               OpStore %79 %87
               OpBranch %83
         %83 = OpLabel
         %88 = OpLoad %v4float %79
               OpReturnValue %88
               OpFunctionEnd
