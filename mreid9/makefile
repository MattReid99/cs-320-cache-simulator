CC = g++
CFLAGS = -Wall -Wextra -DDEBUG -g -std=c++14
OBJFILES = cache-sim.o
TARGET = cache-sim

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
