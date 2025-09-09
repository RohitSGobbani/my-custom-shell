all: myshell

myshell: mycustomshell.c
	gcc mycustomshell.c -o myshell -lreadline
clean:
	rm -f myshell
