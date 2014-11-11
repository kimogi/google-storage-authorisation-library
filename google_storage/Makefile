.PHONY: all clean

all: google_storage_tool.o

google_storage_tool.o: google_storage_tool.c google_storage_tool.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c google_storage_tool.c $(LIBS)

clean:
	rm -rf *.o
