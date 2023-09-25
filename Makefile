CXX := g++-13 -std=c++20 -O3 -fcoroutines
CXXFLAGS := -Wall -g  
RM := del

CL := clang++-16 -std=c++20 -O3

GLSLC := glslc.exe

SRCPATH := ./src
BINPATH := ./bin
OBJPATH := $(BINPATH)/obj
LIBPATHS := ./dep/lib
LIBFLAGS := 
INCLUDEPATH := ./dep/include
MAKEDEPSPATH := ./etc/make-deps

EXE := program.exe

SRCS := $(wildcard $(SRCPATH)/*.cpp)
OBJS := $(patsubst $(SRCPATH)/%.cpp, $(OBJPATH)/%.o, $(SRCS))

DEPENDS := $(patsubst $(SRCPATH)/%.cpp, $(MAKEDEPSPATH)/%.d, $(SRCS))


.PHONY: all run clean

all: $(EXE)
	
$(EXE): $(OBJS)
	$(CL) $(CXXFLAGS) $^ -o $@ -lpthread -L$(LIBPATHS) $(LIBFLAGS)

shaders: $(SHADEROBJS)
	
-include $(DEPENDS)

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp Makefile | $(OBJPATH) $(MAKEDEPSPATH)
	$(CL) $(CXXFLAGS) -MMD -MP -MF $(MAKEDEPSPATH)/$*.d -I$(INCLUDEPATH) -c $< -o $@ 

$(SHADERPATH)/%.spv: $(SHADERPATH)/%
	$(GLSLC) $< -o $@
	
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
