#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := panel
PROJECT_PATH := $(abspath .)
PROJECT_BOARD := evb
export PROJECT_PATH PROJECT_BOARD
#CONFIG_TOOLPREFIX :=

-include ./proj_config.mk

ifeq ($(origin BL60X_SDK_PATH), undefined)
BL60X_SDK_PATH_GUESS ?= $(shell pwd)
BL60X_SDK_PATH ?= $(BL60X_SDK_PATH_GUESS)/../../..
endif

COMPONENTS_BLSYS   := bltime blfdt blmtd bloop loopadc looprt loopset blsync_ble blestack blecontroller_602_std
COMPONENTS_VFS     := romfs  
COMPONENTS_LVGL    := lvgl lv_device lv_srcs draw  extra extra_themes extra_layouts extra_libs extra_others extra_widgets disp_device

INCLUDE_COMPONENTS += freertos_riscv_ram bl602 bl602_std newlibc wifi wifi_manager wpa_supplicant hosal mbedtls_lts lwip lwip_dhcpd vfs yloop utils cli httpc netutils blog blog_testc blsync_ble cjson blcrypto_suite bl_os_adapter wifi_hosal sntp dns_server
INCLUDE_COMPONENTS += $(COMPONENTS_NETWORK)
INCLUDE_COMPONENTS += $(COMPONENTS_BLSYS)
INCLUDE_COMPONENTS += $(COMPONENTS_VFS)
INCLUDE_COMPONENTS += $(PROJECT_NAME)
INCLUDE_COMPONENTS += $(COMPONENTS_LVGL)
include $(BL60X_SDK_PATH)/make_scripts_riscv/project.mk
