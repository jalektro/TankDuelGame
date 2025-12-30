# 1 "C:/DevWorks_vitis/TankDuel/ps7_cortexa9_1/freertos_ps7_cortexa9_1/bsp/lop-config.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "C:/DevWorks_vitis/TankDuel/ps7_cortexa9_1/freertos_ps7_cortexa9_1/bsp/lop-config.dts"

/dts-v1/;
/ {
        compatible = "system-device-tree-v1,lop";
        lops {
                lop_0 {
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetal_validate_comp_xlnx.py";
                };

                lop_1 {
                    compatible = "system-device-tree-v1,lop,assist-v1";
                    node = "/";
                    outdir = "C:/DevWorks_vitis/TankDuel/ps7_cortexa9_1/freertos_ps7_cortexa9_1/bsp";
                    id = "module,baremetal_validate_comp_xlnx";
                    options = "ps7_cortexa9_1 C:/Xilinx/2025.1/Vitis/data/embeddedsw/ThirdParty/bsp/freertos10_xilinx_v1_17/src C:/DevWorks_vitis/_ide/.wsdata/.repo.yaml";
                };

                lop_2 {
                    compatible = "system-device-tree-v1,lop,assist-v1";
                    node = "/";
                    outdir = "C:/DevWorks_vitis/TankDuel/ps7_cortexa9_1/freertos_ps7_cortexa9_1/bsp";
                    id = "module,baremetal_validate_comp_xlnx";
                    options = "ps7_cortexa9_1 C:/Xilinx/2025.1/Vitis/data/embeddedsw/ThirdParty/sw_services/lwip220_v1_2/src C:/DevWorks_vitis/_ide/.wsdata/.repo.yaml";
                };

        };
    };
