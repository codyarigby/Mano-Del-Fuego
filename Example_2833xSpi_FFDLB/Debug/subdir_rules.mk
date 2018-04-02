################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
AsciiLib.obj: ../AsciiLib.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="AsciiLib.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_ADC_cal.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_ADC_cal.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_ADC_cal.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_CodeStartBranch.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_CodeStartBranch.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_CodeStartBranch.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_DefaultIsr.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_DefaultIsr.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_DefaultIsr.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_GlobalVariableDefs.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/source/DSP2833x_GlobalVariableDefs.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_GlobalVariableDefs.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_PieCtrl.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_PieCtrl.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_PieCtrl.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_PieVect.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_PieVect.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_PieVect.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_Spi.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_Spi.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_Spi.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_SysCtrl.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_SysCtrl.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_SysCtrl.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP2833x_usDelay.obj: C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/source/DSP2833x_usDelay.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="DSP2833x_usDelay.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Example_2833xSpi_FFDLB.obj: ../Example_2833xSpi_FFDLB.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="Example_2833xSpi_FFDLB.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

LCD.obj: ../LCD.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-c2000_16.9.3.LTS/include" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v141/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" --define=_DEBUG --define=LARGE_MODEL -g --diag_suppress=10063 --diag_warning=225 --issue_remarks --quiet --preproc_with_compile --preproc_dependency="LCD.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


