PROJECT_NAME = $(shell basename "$(realpath ./)")
HOST_DIR = ../../dwm1001_host_api

# Configurations
NRF_IC = nrf52832
SDK_VERSION = 15
SOFTDEVICE_MODEL = s132
CONFIGURATION_VARS += INTERFACE_NUMBER=1 

# Source and header files
APP_HEADER_PATHS += .
APP_SOURCE_PATHS += .
APP_HEADER_PATHS += $(HOST_DIR)/platform/rpi/hal\
					$(HOST_DIR)/include\
					$(HOST_DIR)/dwm_driver/lmh\


APP_SOURCE_PATHS += $(HOST_DIR)/platform/rpi/hal\
					$(HOST_DIR)/dwm_driver/dwm_api\
					$(HOST_DIR)/dwm_driver/lmh\

APP_SOURCES = $(notdir $(wildcard ./*.c))
APP_SOURCES += $(notdir $(wildcard $(HOST_DIR)/dwm_driver/dwm_api/*.c))\
         hal.c\
         hal_fprint.c\
         hal_spi.c\
			   lmh.c\
			   lmh_spirx.c\

# Include board Makefile (if any)
NRF_BASE_DIR ?= ../../nrf52x-base/
include ../../boards/buckler_revB/Board.mk

# Include main Makefile
include $(NRF_BASE_DIR)/make/AppMakefile.mk


