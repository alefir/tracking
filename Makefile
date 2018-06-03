CC = g++

CFLAGS = -L /usr/local/lib `pkg-config --libs opencv` `pkg-config --cflags --libs protobuf`
TARGET = advtracker

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)
