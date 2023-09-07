CXX = g++
TARGET = httpserver
log = run.log
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
lib = -pthread
version = c++11

$(TARGET) : $(OBJ)
	$(CXX) -o  $@ $^ $(lib) -std=$(version)	

%.o : %.cpp
	$(CXX) -c  $< -o $@

#清理相关文件
.PHONY : clean
clean :
	rm -f *.o $(TARGET) $(log)
