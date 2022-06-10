vm: main.c
	gcc $< -o $@ -pthread

.PHONY: run clean

run:
	./vm

clean:
	rm vm
