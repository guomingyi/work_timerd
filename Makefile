BUILD_TARGET := work_timerd

LD := g++

CCFLAGS += -Wall

CCFLAGS += -g

DEPSFLAGS += -lpthread 

BUILD_SRC := \
    $(shell find . -name '*.c' -o -name '*.cpp' -o -name '*.h')


all:$(BUILD_SRC)
	$(LD) -o $(BUILD_TARGET) $(CCFLAGS) $(DEPSFLAGS) $(BUILD_SRC)  


install: $(BUILD_TARGET)
	[ "`id -u`" = "0" ] || { echo "Must be run as root"; exit 1; }
	cp -rf $(BUILD_TARGET) /usr/local/sbin
	cp -rf init-script /etc/init.d/$(BUILD_TARGET)
	chmod 755 /etc/init.d/$(BUILD_TARGET)
	update-rc.d $(BUILD_TARGET) defaults 92 08
	/etc/init.d/$(BUILD_TARGET) start

uninstall:
	[ "`id -u`" = "0" ] || { echo "Must be run as root"; exit 1; }
	[ -e /etc/init.d/$(BUILD_TARGET) ] && /etc/init.d/$(BUILD_TARGET) stop || :
	update-rc.d -f $(BUILD_TARGET) remove
	rm -rf /usr/local/sbin/$(BUILD_TARGET)
	rm -rf /etc/init.d/$(BUILD_TARGET)

clean:  
	rm -rf *.o $(BUILD_TARGET) 


.PHONY : all install uninstall clean
