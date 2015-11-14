all:	 sender receiver

sender: sender.c
	gcc -Wall $< -o $@
	
receiver: sender.c
	gcc -Wall $< -o $@

clean:
	rm -f receiver sender *.o *~ core
