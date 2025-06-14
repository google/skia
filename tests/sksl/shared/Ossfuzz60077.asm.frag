               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %d_vi "d_vi"                      ; id %6
               OpName %b "b"                            ; id %29
               OpName %c_vi "c_vi"                      ; id %7
               OpName %b_vi "b_vi"                      ; id %8
               OpName %a_vi "a_vi"                      ; id %9
               OpName %main "main"                      ; id %10
               OpName %i "i"                            ; id %52

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_int = OpTypePointer Function %int
         %26 = OpTypeFunction %void %_ptr_Function_int
      %int_4 = OpConstant %int 4
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float
         %56 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function d_vi
       %d_vi = OpFunction %void None %26
         %27 = OpFunctionParameter %_ptr_Function_int

         %28 = OpLabel
          %b =   OpVariable %_ptr_Function_int Function
                 OpStore %b %int_4
                 OpReturn
               OpFunctionEnd


               ; Function c_vi
       %c_vi = OpFunction %void None %26
         %31 = OpFunctionParameter %_ptr_Function_int

         %32 = OpLabel
         %34 =   OpVariable %_ptr_Function_int Function
         %33 =   OpLoad %int %31
                 OpStore %34 %33
         %35 =   OpFunctionCall %void %d_vi %34
                 OpReturn
               OpFunctionEnd


               ; Function b_vi
       %b_vi = OpFunction %void None %26
         %36 = OpFunctionParameter %_ptr_Function_int

         %37 = OpLabel
         %39 =   OpVariable %_ptr_Function_int Function
         %38 =   OpLoad %int %36
                 OpStore %39 %38
         %40 =   OpFunctionCall %void %c_vi %39
                 OpReturn
               OpFunctionEnd


               ; Function a_vi
       %a_vi = OpFunction %void None %26
         %41 = OpFunctionParameter %_ptr_Function_int

         %42 = OpLabel
         %44 =   OpVariable %_ptr_Function_int Function
         %47 =   OpVariable %_ptr_Function_int Function
         %43 =   OpLoad %int %41
                 OpStore %44 %43
         %45 =   OpFunctionCall %void %b_vi %44
         %46 =   OpLoad %int %41
                 OpStore %47 %46
         %48 =   OpFunctionCall %void %b_vi %47
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %49         ; RelaxedPrecision
         %50 = OpFunctionParameter %_ptr_Function_v2float

         %51 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
         %54 =   OpVariable %_ptr_Function_int Function
         %53 =   OpLoad %int %i
                 OpStore %54 %53
         %55 =   OpFunctionCall %void %a_vi %54
                 OpReturnValue %56
               OpFunctionEnd
