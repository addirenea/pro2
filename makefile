TARGETS=

all: $(TARGETS)

clean:
	rm $(TARGETS)

% : %.c
	gcc -Wall -g -o $@ $<
