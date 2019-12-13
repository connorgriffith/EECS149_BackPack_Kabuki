PROJECT_NAME = $(shell basename "$(realpath ./)")

# Configurations
NRF_IC = nrf52832
SDK_VERSION = 15
SOFTDEVICE_MODEL = s132
CONFIGURATION_VARS += INTERFACE_NUMBER=1 

# Source and header files
APP_HEADER_PATHS += .
APP_HEADER_PATHS += $(NRF_BASE_DIR)/sdk/nrf5_sdk_15.3.0/modules/nrfx/
APP_HEADER_PATHS += $(NRF_BASE_DIR)/sdk/nrf5_sdk_15.3.0/modules/nrfx/hal/
APP_HEADER_PATHS += $(NRF_BASE_DIR)/sdk/nrf5_sdk_15.3.0/modules/nrfx/drivers/include/
APP_SOURCE_PATHS += .
APP_SOURCE_PATHS +=  $(NRF_BASE_DIR)/sdk/nrf5_sdk_15.3.0/modules/nrfx/drivers/src/


APP_SOURCES = $(notdir $(wildcard ./*.c))
APP_SOURCES += nrfx_spi.c

# Include board Makefile (if any)
NRF_BASE_DIR ?= ../../nrf52x-base/
include ../../boards/buckler_revB/Board.mk

# Include main Makefile
include $(NRF_BASE_DIR)/make/AppMakefile.mk


