# makefile by Matheus Ferreira (github.com/mfbsouza)

# App Settings
CXX = gcc
EXE = app
OBJ_DIR = obj

# Driver Settings
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
obj-m := altera_driver.o
PWD := $(shell pwd)

# Flags
CXXFLAGS += -Wall -fopenmp
LDFLAGS += -L/usr/local/lib -lSDL2 -lSDL2_mixer -fopenmp
#LDLIBS += -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ_DIR)/app.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/app.o: app.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Driver building
#driver:
#	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
#
#move:
#	mv *.o .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers $(OBJ_DIR)/
#
#rm -rf $(OBJ_DIR)/.tmp_versions $(OBJ_DIR)/modules.order $(OBJ_DIR)/Module.symvers
#rm -rf $(OBJ_DIR)/.altera_driver.ko.cmd $(OBJ_DIR)/.altera_driver.mod.o.cmd $(OBJ_DIR)/.altera_driver.o.cmd
clean:
	rm -rf $(OBJ_DIR)/*
	rm $(EXE)
