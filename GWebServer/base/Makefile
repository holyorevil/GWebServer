source = *.cpp
target = run
CFLAGS = -std=c++11 -g -Wall 
LIBRARY = -lpthread
GCC=g++
ALL:$(target)

$(target):$(source)
	$(GCC) $^ -o $@ $(CFLAGS) $(LIBRARY)

clean:
	rm -rf $(target)

.PHONY:$(target)