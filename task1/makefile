CC = gcc

CFLAGS = -I/usr/local/include -O3 -march=native -mavx2 -mfma -funroll-loops -fopenmp
LDFLAGS = /usr/local/lib/libblis.a -lm -lpthread -fopenmp

TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)