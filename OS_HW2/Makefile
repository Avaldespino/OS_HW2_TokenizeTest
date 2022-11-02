OBJS = uiMain.o tokenizer.o history.o

CFLAGS = -g -O3

all: uiMain

uiMain:$(OBJS)

	cc -o uiMain $(CFLAGS) $(OBJS)
	
	cc -fPIC -Wall -g -O0 -c implementation.c
	

$(OBJS): tokenizer.h history.h

clean:
	rm -f *.o uiMain
	rm -f *~

run:uiMain
	./uiMain
