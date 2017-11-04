CC = g++

CFLAGS = -L /usr/local/lib `pkg-config --libs opencv` `pkg-config --cflags --libs protobuf`
TARGET = kcftracker

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) -v $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)
