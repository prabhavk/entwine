CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC = src/main.cpp
TARGET = entwine

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
