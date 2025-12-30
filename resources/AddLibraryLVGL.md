# Integrating LVGL into Vitis Unified IDE

This document describes how to integrate the **LVGL (Light and Versatile Graphics Library)** into a Vitis Unified IDE (2025.1) application.
 The instructions below are based on the **TankDuel_game** project but are generally applicable to other Vitis projects.

------

## Prerequisites

- Vitis Unified IDE 2025.1
- Git installed and available in the system path
- A working Vitis application project

------

## Downloading the LVGL Library

1. Open the project directory.
2. Open a command line interface in this folder.
3. Clone the LVGL repository and check out a stable version:

```
git clone https://github.com/lvgl/lvgl.git
cd lvgl
git checkout v8.3.11
```

Version `v8.3.11` is a stable and commonly used release for embedded systems.

------

## Adding LVGL to the Vitis Project

### Recommended Project Layout

The LVGL library should be placed inside the application source directory.

```
your_app/
├── src/
│   ├── main.c
│   ├── lvgl/
│   │   ├── src/
│   │   ├── lv_conf_template.h
│   │   └── lvgl.h
```

### Files to Copy

Copy only the following items from the cloned LVGL repository into your project:

```
lvgl/
 ├── src/
 ├── lvgl.h
 ├── lv_conf_template.h
```

### Files Not Required

Do **not** copy the following directories unless explicitly needed:

- `docs/`
- `tests/`
- `demos/`

------

## Creating `lv_conf.h` (Mandatory)

LVGL will not compile without a configuration file.

### Step 1: Create the Configuration File

Copy the template file and rename it:

```
lv_conf_template.h → lv_conf.h
```

Place `lv_conf.h` in the same directory as `lvgl.h`.

### Step 2: Edit `lv_conf.h`

Apply at least the following configuration options (or use the provided project configuration):

```
#define LV_COLOR_DEPTH      16
#define LV_MEM_SIZE        (64 * 1024)
#define LV_USE_LOG         1
#define LV_USE_ASSERT_NULL 1
#define LV_TICK_CUSTOM     1
```

Adjust memory size or features as required by your application.

------

## Configuring Vitis Build Settings

### Update `USERconfig.cmake`

Open `USERconfig.cmake` and update the configuration as follows:

#### Include Directories

Add the LVGL include paths to the project include directories.

![](C:\DevWorks\TankDuel_EOS\resources\pictures\Directories.jpg)

#### Source Files

Ensure that all LVGL source files are added to the build system so they are compiled together with the application.

![](C:\DevWorks\TankDuel_EOS\resources\pictures\compileSources.jpg)

------

## Adding Required Preprocessor Symbols

LVGL requires a preprocessor definition to locate the configuration file.

Add the following symbol under **Defined Symbols (-D)**:

```
LV_CONF_INCLUDE_SIMPLE
```

This tells LVGL to include `lv_conf.h` directly without a relative path.

------

## Building the Project

After completing all steps:

1. Clean the project.
2. Build the application.

If all paths and symbols are configured correctly, the project should compile successfully with LVGL integrated.

------

## Notes

- Ensure that `lv_conf.h` is visible to the compiler via the include paths.
- Compilation errors related to missing configuration options usually indicate an incorrect or incomplete `lv_conf.h`.
- Memory and feature settings should be tuned according to the target hardware.