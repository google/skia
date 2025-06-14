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
               OpName %b "b"                            ; id %40
               OpName %f1 "f1"                          ; id %47
               OpName %f2 "f2"                          ; id %48
               OpName %f3 "f3"                          ; id %50
               OpName %i1 "i1"                          ; id %53
               OpName %i2 "i2"                          ; id %55
               OpName %i3 "i3"                          ; id %56
               OpName %b1 "b1"                          ; id %59
               OpName %b2 "b2"                          ; id %61
               OpName %b3 "b3"                          ; id %63

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
               OpDecorate %77 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision

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
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %float_9 = OpConstant %float 9
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
          %b =   OpVariable %_ptr_Function_bool Function
         %f1 =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_float Function
         %f3 =   OpVariable %_ptr_Function_float Function
         %i1 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_int Function
         %i3 =   OpVariable %_ptr_Function_int Function
         %b1 =   OpVariable %_ptr_Function_bool Function
         %b2 =   OpVariable %_ptr_Function_bool Function
         %b3 =   OpVariable %_ptr_Function_bool Function
         %80 =   OpVariable %_ptr_Function_v4float Function
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
         %46 =   OpFUnordNotEqual %bool %45 %float_0
                 OpStore %b %46
                 OpStore %f1 %33
         %49 =   OpConvertSToF %float %39
                 OpStore %f2 %49
         %51 =   OpSelect %float %46 %float_1 %float_0
                 OpStore %f3 %51
         %54 =   OpConvertFToS %int %33
                 OpStore %i1 %54
                 OpStore %i2 %39
         %57 =   OpSelect %int %46 %int_1 %int_0
                 OpStore %i3 %57
         %60 =   OpFUnordNotEqual %bool %33 %float_0
                 OpStore %b1 %60
         %62 =   OpINotEqual %bool %39 %int_0
                 OpStore %b2 %62
                 OpStore %b3 %46
         %64 =   OpFAdd %float %33 %49              ; RelaxedPrecision
         %65 =   OpFAdd %float %64 %51              ; RelaxedPrecision
         %66 =   OpConvertSToF %float %54           ; RelaxedPrecision
         %67 =   OpFAdd %float %65 %66              ; RelaxedPrecision
         %68 =   OpConvertSToF %float %39           ; RelaxedPrecision
         %69 =   OpFAdd %float %67 %68              ; RelaxedPrecision
         %70 =   OpConvertSToF %float %57           ; RelaxedPrecision
         %71 =   OpFAdd %float %69 %70              ; RelaxedPrecision
         %72 =   OpSelect %float %60 %float_1 %float_0  ; RelaxedPrecision
         %73 =   OpFAdd %float %71 %72                  ; RelaxedPrecision
         %74 =   OpSelect %float %62 %float_1 %float_0  ; RelaxedPrecision
         %75 =   OpFAdd %float %73 %74                  ; RelaxedPrecision
         %76 =   OpSelect %float %46 %float_1 %float_0  ; RelaxedPrecision
         %77 =   OpFAdd %float %75 %76                  ; RelaxedPrecision
         %79 =   OpFOrdEqual %bool %77 %float_9
                 OpSelectionMerge %84 None
                 OpBranchConditional %79 %82 %83

         %82 =     OpLabel
         %85 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %86 =       OpLoad %v4float %85            ; RelaxedPrecision
                     OpStore %80 %86
                     OpBranch %84

         %83 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %80 %88
                     OpBranch %84

         %84 = OpLabel
         %89 =   OpLoad %v4float %80                ; RelaxedPrecision
                 OpReturnValue %89
               OpFunctionEnd
