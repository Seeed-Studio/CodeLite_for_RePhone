
WORKSPACE_PATH = ../
PROJECT = lua

OBJECTS += main.o retarget.o shell.o audio.o gsm.o timer.o

WORKSPACE_PATH = ../

PROJECT_PATH := $(patsubst %/Makefile,%,$(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_PATH := $(patsubst %/makefile,%,$(PROJECT_PATH))

include $(WORKSPACE_PATH)lua/Makefile
include $(WORKSPACE_PATH)Makefile
