# variable
CC     = gcc
DEPS   = intel_hex.h
OBJ    = main.o intel_hex.o
BIN    = application.bin
TARGET = application.exe

# include path
INC := .

# resources path
SRC := main.c intel_hex.c

%.o: %.c $(DEPS)
	$(CC) -I$(INC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

.PHONY: clean build

# command build .hex -> .bin
build: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ) $(BIN)
run: 
	./$(TARGET)

# read .bin
read:
	hexdump -C $(BIN)