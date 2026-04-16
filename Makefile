
CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall


SOURCES = main.cpp board.cpp movegen.cpp evaluator.cpp ai_engine.cpp ids_engine.cpp maia_engine.cpp


TARGET = chess.exe


all: $(TARGET)

$(TARGET): $(SOURCES) types.h board.h movegen.h evaluator.h ai_engine.h ids_engine.h maia_engine.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)


run: $(TARGET)
	./$(TARGET)


clean:
	del /f $(TARGET) 2>nul || rm -f $(TARGET)

.PHONY: all run clean
