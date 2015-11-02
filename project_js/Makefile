
PROJECT = js

OBJECTS += main.o retarget.o v7.o sj_prompt.o gpio.o audio.o gsm.o
INCLUDE_PATHS += -I.
CC_SYMBOLS += -DNO_BSD_SOCKETS

WORKSPACE_PATH = ../

PROJECT_PATH := $(patsubst %/Makefile,%,$(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_PATH := $(patsubst %/makefile,%,$(PROJECT_PATH))

include $(WORKSPACE_PATH)Makefile
