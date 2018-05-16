TARGET = fade_leds
CFLAGS = -Wall
RPI_LIB = ../rpi_ws281x

default: $(TARGET)

$(TARGET): $(TARGET).c
	gcc $< -o $(TARGET) -I $(RPI_LIB) $(RPI_LIB)/libws2811.a

clean:
	rm -f $(TARGET)

install:
	chown root $(TARGET)
	chmod u+s $(TARGET)
