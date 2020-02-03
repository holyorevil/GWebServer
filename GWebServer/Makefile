#source = $(wildcard *.cpp)
source := testclient.cpp
allSource := $(wildcard *.cpp)  
test1Source += $(filter-out testclient.cpp noblock_client.cpp,$(allSource))
baseSource := $(wildcard base/*.cpp)
BaseSource := $(filter-out base/test.cpp,$(baseSource))
test1Obj := $(patsubst %.cpp, ./obj/%.o, $(test1Source))
BaseObj := $(patsubst base/%.cpp, ./obj/%.o, $(BaseSource))
test1Target := run
target := $(patsubst %.cpp, %, $(source))
CFLAGS := -std=c++11 -g -Wall -lpthread
ALL:$(target) $(test1Target)

#模式规则就是生成目标有多个，并且每一个生成目标依赖条件不一样，所以其实一条模式规则是把多个语句合并到一起
#比如下面这个模式规则，就是生成目标有多个，所以使用
$(target):%:%.cpp
	g++ $< -o $@ -g
test1:
	g++ $(test1Obj) $(BaseObj) -o test1 $(CFLAGS)

$(test1Target):$(test1Obj) $(BaseObj)
	g++ $^ -o $@ $(CFLAGS)

$(test1Obj):./obj/%.o:%.cpp
	g++ -c $< -o $@ $(CFLAGS)

$(BaseObj):./obj/%.o:./base/%.cpp
	g++ -c $< -o $@ $(CFLAGS)


clean:
	-rm -rf $(target) $(test1Obj) $(baseObj) test1

logClean:
	-rm -rf log/*.log

debug:
	@echo $(test1Source)
#这个的作用是如果重复编译一个目标的情况下，如果不把这个目标加入到.PHONY中，就会停止编译，除非删除，所以加入到下面比较省事
.PHONY: ALL test1
