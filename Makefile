BUILD_TARGET := work_timerd


LD := g++
CCFLAGS := -Wall
CCFLAGS += -g

DEPSFLAGS := -lpthread 
#DEPSFLAGS += -lwiringPi

BUILD_SRC := \
    $(shell find . -name '*.c' -o -name '*.cpp' -o -name '*.h')


all:$(BUILD_SRC)
	$(LD) -o $(BUILD_TARGET) $(CCFLAGS) $(DEPSFLAGS) $(BUILD_SRC)  


clean:  
	rm -rf *.o $(BUILD_TARGET) 


.PHONY : all clean
