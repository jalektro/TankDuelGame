# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\lwipopts.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\sleep.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\xemac_ieee_reg.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\xemacpsif_hw.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\xiltimer.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\xlwipconfig.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\include\\xtimer_config.h"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\lib\\liblwip220.a"
  "C:\\DevWorks_vitis\\TankDuel\\ps7_cortexa9_1\\freertos_ps7_cortexa9_1\\bsp\\lib\\libxiltimer.a"
  )
endif()
