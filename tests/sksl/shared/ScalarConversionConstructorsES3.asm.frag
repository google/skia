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
               OpName %u "u"
               OpName %b "b"
               OpName %f1 "f1"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %f4 "f4"
               OpName %i1 "i1"
               OpName %i2 "i2"
               OpName %i3 "i3"
               OpName %i4 "i4"
               OpName %u1 "u1"
               OpName %u2 "u2"
               OpName %u3 "u3"
               OpName %u4 "u4"
               OpName %b1 "b1"
               OpName %b2 "b2"
               OpName %b3 "b3"
               OpName %b4 "b4"
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
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
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
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
   %float_16 = OpConstant %float 16
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
          %u = OpVariable %_ptr_Function_uint Function
          %b = OpVariable %_ptr_Function_bool Function
         %f1 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_float Function
         %f3 = OpVariable %_ptr_Function_float Function
         %f4 = OpVariable %_ptr_Function_float Function
         %i1 = OpVariable %_ptr_Function_int Function
         %i2 = OpVariable %_ptr_Function_int Function
         %i3 = OpVariable %_ptr_Function_int Function
         %i4 = OpVariable %_ptr_Function_int Function
         %u1 = OpVariable %_ptr_Function_uint Function
         %u2 = OpVariable %_ptr_Function_uint Function
         %u3 = OpVariable %_ptr_Function_uint Function
         %u4 = OpVariable %_ptr_Function_uint Function
         %b1 = OpVariable %_ptr_Function_bool Function
         %b2 = OpVariable %_ptr_Function_bool Function
         %b3 = OpVariable %_ptr_Function_bool Function
         %b4 = OpVariable %_ptr_Function_bool Function
        %114 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpCompositeExtract %float %32 1
               OpStore %f %33
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %37 = OpLoad %v4float %36
         %38 = OpCompositeExtract %float %37 1
         %39 = OpConvertFToS %int %38
               OpStore %i %39
         %43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpCompositeExtract %float %44 1
         %46 = OpConvertFToU %uint %45
               OpStore %u %46
         %49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %50 = OpLoad %v4float %49
         %51 = OpCompositeExtract %float %50 1
         %52 = OpFUnordNotEqual %bool %51 %float_0
               OpStore %b %52
               OpStore %f1 %33
         %55 = OpConvertSToF %float %39
               OpStore %f2 %55
         %57 = OpConvertUToF %float %46
               OpStore %f3 %57
         %59 = OpSelect %float %52 %float_1 %float_0
               OpStore %f4 %59
         %62 = OpConvertFToS %int %33
               OpStore %i1 %62
               OpStore %i2 %39
         %65 = OpBitcast %int %46
               OpStore %i3 %65
         %67 = OpSelect %int %52 %int_1 %int_0
               OpStore %i4 %67
         %70 = OpConvertFToU %uint %33
               OpStore %u1 %70
         %72 = OpBitcast %uint %39
               OpStore %u2 %72
               OpStore %u3 %46
         %75 = OpSelect %uint %52 %uint_1 %uint_0
               OpStore %u4 %75
         %79 = OpFUnordNotEqual %bool %33 %float_0
               OpStore %b1 %79
         %81 = OpINotEqual %bool %39 %int_0
               OpStore %b2 %81
         %83 = OpINotEqual %bool %46 %uint_0
               OpStore %b3 %83
               OpStore %b4 %52
         %85 = OpFAdd %float %33 %55
         %86 = OpFAdd %float %85 %57
         %87 = OpFAdd %float %86 %59
         %88 = OpConvertSToF %float %62
         %89 = OpFAdd %float %87 %88
         %90 = OpConvertSToF %float %39
         %91 = OpFAdd %float %89 %90
         %92 = OpConvertSToF %float %65
         %93 = OpFAdd %float %91 %92
         %94 = OpConvertSToF %float %67
         %95 = OpFAdd %float %93 %94
         %96 = OpConvertUToF %float %70
         %97 = OpFAdd %float %95 %96
         %98 = OpConvertUToF %float %72
         %99 = OpFAdd %float %97 %98
        %100 = OpConvertUToF %float %46
        %101 = OpFAdd %float %99 %100
        %102 = OpConvertUToF %float %75
        %103 = OpFAdd %float %101 %102
        %104 = OpSelect %float %79 %float_1 %float_0
        %105 = OpFAdd %float %103 %104
        %106 = OpSelect %float %81 %float_1 %float_0
        %107 = OpFAdd %float %105 %106
        %108 = OpSelect %float %83 %float_1 %float_0
        %109 = OpFAdd %float %107 %108
        %110 = OpSelect %float %52 %float_1 %float_0
        %111 = OpFAdd %float %109 %110
        %113 = OpFOrdEqual %bool %111 %float_16
               OpSelectionMerge %118 None
               OpBranchConditional %113 %116 %117
        %116 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %120 = OpLoad %v4float %119
               OpStore %114 %120
               OpBranch %118
        %117 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %122 = OpLoad %v4float %121
               OpStore %114 %122
               OpBranch %118
        %118 = OpLabel
        %123 = OpLoad %v4float %114
               OpReturnValue %123
               OpFunctionEnd
