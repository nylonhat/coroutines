CLANG := clang++
ifdef OS
	CLANG += -femulated-tls
endif

GCC := g++ -fcoroutines

CXX := $(CLANG) -std=c++20

DEBUGFLAGS := -Wall -g -O3 -march=native -fsanitize=thread
RELEASEFLAGS := -O3 -march=native -DNDEBUG -fno-exceptions

CXXFLAGS := $(RELEASEFLAGS)
ifdef OS
	
else 
	CXXFLAGS += -flto=auto
endif

SRCPATH := ./src
BINPATH := ./bin
OBJPATH := $(BINPATH)/obj

LIBPATHS := ./dep/lib
ifdef OS
	LIBFLAGS := -lWs2_32#-lSynchronization
else
	LIBFLAGS := -luring
endif

INCLUDEPATH := ./dep/include
MAKEDEPSPATH := ./etc/make-deps

EXE := program.exe

#SRCS := $(wildcard $(SRCPATH)/*.cpp) $(wildcard $(SRCPATH)/*/*.cpp) $(wildcard $(SRCPATH)/*/*/*.cpp) $(wildcard $(SRCPATH)/*/*/*/*.cpp) 
ifdef OS
	SRCS := $(wildcard $(SRCPATH)/*.cpp) $(wildcard $(SRCPATH)/*/*.cpp) $(wildcard $(SRCPATH)/*/*/*.cpp) $(wildcard $(SRCPATH)/*/*/*/*.cpp)
else
	SRCS := $(shell find $(SRCPATH) -name '*.cpp')
endif

OBJS := $(SRCS:$(SRCPATH)/%.cpp=$(OBJPATH)/%.o)
OBJDIR := $(sort $(dir $(OBJS)))

DEPENDS := $(SRCS:$(SRCPATH)/%.cpp=$(MAKEDEPSPATH)/%.d)
DEPENDSDIR := $(sort $(dir $(DEPENDS)))

.PHONY: all run clean

all: $(EXE)
	
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread -L$(LIBPATHS) $(LIBFLAGS)

-include $(DEPENDS)

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp Makefile | $(OBJDIR) $(DEPENDSDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(MAKEDEPSPATH)/$*.d -I$(INCLUDEPATH) -c $< -o $@ 

$(OBJDIR) $(DEPENDSDIR):
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
	
