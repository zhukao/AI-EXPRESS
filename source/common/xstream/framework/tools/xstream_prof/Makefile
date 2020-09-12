CFLAGS += -I./library -g

sample/sample: library/time_record.o sample/sample.o
	$(CC) -o $@ $^ -lpthread

.PHONY: clean

clean:
	$(RM) library/time_record.o
	$(RM) sample/sample.o
	$(RM) sample/sample