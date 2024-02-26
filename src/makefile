GCC = g++
TARGET = httpserver

SRC = $(shell find -name "*.cpp" -not -path "./sample_test/*")
OBJ = $(notdir $(patsubst %.cpp, %.o, $(SRC)))

lib = -lpthread
version = c++11
log = *.log

# all: $(TARGET)

$(TARGET) : $(OBJ)
	$(GCC) -o $(TARGET) sample_test/main.cpp $(OBJ) $(lib) -std=$(version)

$(OBJ) : $(SRC)
	$(GCC) -c $< -o $@ 


#清理相关文件
.PHONY : debug clean
debug:
	@echo $(SRC)
	@echo $(OBJ)
clean :
	rm -f *.o $(TARGET) 

