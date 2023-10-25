CLANG := clang++
GCC := g++ -fcoroutines

CXX := $(CLANG) -std=c++20 -flto #-fsanitize=thread

CXXFLAGS := -Wall -g -O3 -march=native

SRCPATH := ./src
BINPATH := ./bin
OBJPATH := $(BINPATH)/obj

LIBPATHS := ./dep/lib
ifdef OS
	LIBFLAGS := -lSynchronization -lWs2_32
else
	LIBFLAGS := -luring
endif

INCLUDEPATH := ./dep/include
MAKEDEPSPATH := ./etc/make-deps

EXE := program.exe

SRCS := $(wildcard $(SRCPATH)/*.cpp)
OBJS := $(patsubst $(SRCPATH)/%.cpp, $(OBJPATH)/%.o, $(SRCS))

DEPENDS := $(patsubst $(SRCPATH)/%.cpp, $(MAKEDEPSPATH)/%.d, $(SRCS))


.PHONY: all run clean

all: $(EXE)
	
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread -L$(LIBPATHS) $(LIBFLAGS)

-include $(DEPENDS)

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp Makefile | $(OBJPATH) $(MAKEDEPSPATH)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(MAKEDEPSPATH)/$*.d -I$(INCLUDEPATH) -c $< -o $@ 

$(OBJPATH) $(MAKEDEPSPATH):
ifdef OS
	powershell.exe [void](New-Item -ItemType Directory -Path ./ -Name $@)
else
	mkdir -p $@
endif

clean:
ifdef OS
	powershell.exe if (Test-Path $(OBJPATH)) {Remove-Item $(OBJPATH) -Recurse}
	powershell.exe if (Test-Path $(EXE)) {Remove-Item $(EXE)}
	powershell.exe if (Test-Path $(MAKEDEPSPATH)) {Remove-Item $(MAKEDEPSPATH) -Recurse}
else
	rm -rf $(OBJPATH)
	rm -rf $(EXE)
	rm -rf $(MAKEDEPSPATH)
endif

run:
	./$(EXE)
	
