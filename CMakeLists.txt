#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Adapted and extended by Michael Neises
#

cmake_minimum_required(VERSION 3.8.2)

project(vm-measure C)

include(${CAMKES_ARM_VM_HELPERS_PATH})

# Create our CPP Flags based on ARM VM config variables
find_package(camkes-vm-linux REQUIRED)
include(${CAMKES_VM_LINUX_HELPERS_PATH})
set(cpp_flags "-DKERNELARMPLATFORM_QEMU-ARM-VIRT")

# Add our custom linux kernel
AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/newLinux")

# Grab the default rootfs
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/rootfs.cpio.gz")

include(${CAMKES_VM_LINUX_SOURCE_HELPERS_PATH})
include(${CAMKES_VM_LINUX_MODULE_HELPERS_PATH})
include(ExternalProject)
include(external-project-helpers)

# Setup and configure linux sources
set(linux_config "${CAMKES_VM_LINUX_DIR}/linux_configs/4.9.y/64/config")
set(linux_symvers "${CAMKES_VM_LINUX_DIR}/linux_configs/4.9.y/64/Module.symvers")

# Set the directory of our custom linux kernel
set(linux_dir "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/linux-stable")

# Add a kernel module
ExternalProject_Add(good-module
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/good-module
    BUILD_ALWAYS ON
    STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/good-module-stamp
    EXCLUDE_FROM_ALL
    INSTALL_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${LINUX_64BIT_TOOLCHAIN}
        -DLINUX_KERNEL_DIR=${linux_dir}
        -DMODULE_HELPERS_FILE=${CAMKES_VM_LINUX_DIR}/linux-module-helpers.cmake
)
# Add our module binary to the overlay
# The number at this time happens to be 275.
AddExternalProjFilesToOverlay(
    good-module
    ${CMAKE_CURRENT_BINARY_DIR}/good-module
    vm-overlay
    "lib/modules/4.9.275/kernel/drivers/vmm"
    FILES
    good.ko
)

# Add another module
ExternalProject_Add(poison-module
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/poison-module
    BUILD_ALWAYS ON
    STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/poison-module-stamp
    EXCLUDE_FROM_ALL
    INSTALL_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${LINUX_64BIT_TOOLCHAIN}
        -DLINUX_KERNEL_DIR=${linux_dir}
        -DMODULE_HELPERS_FILE=${CAMKES_VM_LINUX_DIR}/linux-module-helpers.cmake
)
AddExternalProjFilesToOverlay(
    poison-module
    ${CMAKE_CURRENT_BINARY_DIR}/poison-module
    vm-overlay
    "lib/modules/4.9.275/kernel/drivers/vmm"
    FILES
    poison.ko
)

# Add another module
ExternalProject_Add(measurement-module
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/measurement-module
    BUILD_ALWAYS ON
    STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/measurement-module-stamp
    EXCLUDE_FROM_ALL
    INSTALL_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${LINUX_64BIT_TOOLCHAIN}
        -DLINUX_KERNEL_DIR=${linux_dir}
        -DMODULE_HELPERS_FILE=${CAMKES_VM_LINUX_DIR}/linux-module-helpers.cmake
)
AddExternalProjFilesToOverlay(
    measurement-module
    ${CMAKE_CURRENT_BINARY_DIR}/measurement-module
    vm-overlay
    "lib/modules/4.9.275/kernel/drivers/vmm"
    FILES
    measurement.ko
)



    # Complile CrossVM Dataport Apps
    ExternalProject_Add(
        dataport-apps
        URL
        file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/dataport
        BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/dataport_apps
        INSTALL_COMMAND
        ""
        BUILD_ALWAYS
        ON
        EXCLUDE_FROM_ALL
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    AddExternalProjFilesToOverlay(
        dataport-apps
        ${CMAKE_CURRENT_BINARY_DIR}/dataport_apps
        vm-overlay
        "usr/bin"
        FILES
        dataport_read
        dataport_write
    )

    # Complile CrossVM Event Apps
    ExternalProject_Add(
        event-apps
        URL
        file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/emits_event
        BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/emits_event_apps
        INSTALL_COMMAND
        ""
        BUILD_ALWAYS
        ON
        EXCLUDE_FROM_ALL
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    AddExternalProjFilesToOverlay(
        event-apps
        ${CMAKE_CURRENT_BINARY_DIR}/emits_event_apps
        vm-overlay
        "usr/bin"
        FILES
        emits_event_emit
    )

    # Complile CrossVM Consume Event Apps
    ExternalProject_Add(
        consume-event-apps
        URL
        file:///${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-apps/camkes-connector-apps/pkgs/consumes_event
        BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/consume_event_apps
        INSTALL_COMMAND
        ""
        BUILD_ALWAYS
        ON
        EXCLUDE_FROM_ALL
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    AddExternalProjFilesToOverlay(
        consume-event-apps
        ${CMAKE_CURRENT_BINARY_DIR}/consume_event_apps
        vm-overlay
        "usr/bin"
        FILES
        consumes_event_wait
    )

    # Add script to initialise dataport module
    AddFileToOverlayDir(
        "S90crossvm_module_init"
        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/cross_vm_module_init
        "etc/init.d"
        vm-overlay
    )


    # Add script to test dataport module
#    AddFileToOverlayDir(
#        "S91crossvm_test"
#        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/cross_vm_test
#        "etc/init.d"
#        vm-overlay
#    )





# Add our poison module's init script- just an insmod call
AddFileToOverlayDir(
    "mike_modules_init"
    ${CMAKE_CURRENT_SOURCE_DIR}/modules_init
    "etc/init.d"
    vm-overlay
)


# Generate overlayed rootfs
AddOverlayDirToRootfs(
    vm-overlay 
    ${rootfs_file} 
    "buildroot" 
    "rootfs_install" 
    output_overlayed_rootfs_location 
    rootfs_target 
    GZIP 
)

# Add the newly built rootfs
AddToFileServer(
    "linux-initrd"
    ${output_overlayed_rootfs_location}
    DEPENDS
    rootfs_target
)

# Updated dtb based on initrd
UpdateDtbFromInitrd(
    "${CAMKES_VM_IMAGES_DIR}/qemu-arm-virt/linux-dtb"
    ${rootfs_file}
    "0x4d700000"
    dtb_gen_target
    output_dtb_location
)
AddToFileServer("linux-dtb" "${output_dtb_location}" DEPENDS dtb_gen_target)
include(simulation)
set(SIMULATION ON CACHE BOOL "Generate simulation script to run qemu with the proper arguments")
if(SIMULATION)
    GenerateSimulateScript()
endif()

AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB Tk1DeviceFwd Tk1Insecure)

DefineCAmkESVMFileServer()

CAmkESAddImportPath(${KernelARMPlatform})

# Define our VM Component with out cross vm dataports glue code
DeclareCAmkESComponent(VM SOURCES src/cross_vm_connections.c)

# Define our demo component that shares a dataport with the VM
DeclareCAmkESComponent(Measurer SOURCES components/Measurer/measurer.c)

# Declare root server
DeclareCAmkESRootserver(
    vm_measure.camkes
    CPP_FLAGS
    ${cpp_flags}
    CPP_INCLUDES
    ${CAMKES_VM_DIR}/components/VM_Arm
)

