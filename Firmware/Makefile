
#-------------------------------------------------------------------------------
# Configure variable
#-------------------------------------------------------------------------------
TOP_DIR:=$(TOP_DIR)
ROOT_DIR:=$(TOP_DIR)/..
MAKEFILE_PATH :=$(subst $(strip \),/,$(abspath $(lastword $(MAKEFILE_LIST))))


#-------------------------------------------------------------------------------
# Configure source code files
#-------------------------------------------------------------------------------
SRC_FILES+= \
	main.c\
  src\CircularBuffer.c \
  src\virtualCom.c \
  src\dht11.c \
  src\sds011.c \
  src\mq135.c \
  src\mainProcess.c \
  src\demo\backup.c \
  src\demo\di_counter.c \
  src\demo\mqtt_demo.c \
  src\demo\flash_ota.c \
  src\demo\sms_demo.c \
  src\demo\uart_debug.c \
  src\simcom_demo\cus_urc.c \
  #src\uart.c \
  #src\sgp30.c \
  
  #src\uart.c \
  src\config.c \
  src\gpio.c \
  src\urcProcess.c \
  src\network.c \
  src\timer.c \
  src\rtc.c \
  src\sms.c \
  src\call.c \
  src\mqtt.c \
  src\ssl.c \
  src\tcpip.c \
  src\i2c.c \
  src\spi.c \
  src\lcd\i2c_lcd.c \
  src\fota.c \
  src\internalFlash.c \
  src\http.c \
  src\watchdog.c \
  src\adc.c \
  
#-------------------------------------------------------------------------------
# the include directories
#-------------------------------------------------------------------------------
INC_DIRS+= \
  -I${TOP_DIR}/simcom_lib/inc \
  -I${TOP_DIR}/user_source/inc \
  -I${TOP_DIR}/user_source/src/lcd \
  -I${TOP_DIR}/user_source/src/simcom_demo \
#-------------------------------------------------------------------------------
# Configure compile flag
#-------------------------------------------------------------------------------
U_FLAGS:=

#-------------------------------------------------------------------------------
# Configure default macro define
#-------------------------------------------------------------------------------
DFLAGS+=


#-------------------------------------------------------------------------------
# include mk file
#-------------------------------------------------------------------------------
include ${TOP_DIR}\script\package.mk
