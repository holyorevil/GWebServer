source := $(wildcard *.cc)
source += $(wildcard ../*.cpp)
Source += $(filter-out ../test.cpp,$(source))
CFLAGS := -std=c++11 -g -Wall -lpthread -I /home/gmq/桌面/FILE/WebServer-master/MyWeb4/base/test_log
target := run
ALL:$(target)

$(target):$(Source)
	g++ $^ -o $@ $(CFLAGS)

clean:
	-rm -rf $(target) *.log

log:
	-rm -rf *.log

.PHONY: ALL