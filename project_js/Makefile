
PROJECT = js

OBJECTS += main.o retarget.o ../v7/v7.o
INCLUDE_PATHS += -I../v7

WORKSPACE_PATH = ../

PROJECT_PATH := $(patsubst %/Makefile,%,$(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_PATH := $(patsubst %/makefile,%,$(PROJECT_PATH))


include $(WORKSPACE_PATH)Makefile
