# # # Makefile for Proxy Lab 
# # #
# # # You may modify this file any way you like (except for the handin
# # # rule). You instructor will type "make" on your specific Makefile to
# # # build your proxy from sources.

# # CC = gcc
# # CFLAGS = -g -Wall
# # LDFLAGS = -lpthread

# # all: proxy

# # csapp.o: csapp.c csapp.h
# # 	$(CC) $(CFLAGS) -c csapp.c

# # proxy.o: proxy.c csapp.h
# # 	$(CC) $(CFLAGS) -c proxy.c

# # proxy: proxy.o csapp.o
# # 	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

# # # Creates a tarball in ../proxylab-handin.tar that you can then
# # # hand in. DO NOT MODIFY THIS!
# # handin:
# # 	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

# # clean:
# # 	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz



# # Makefile for Proxy Lab 
# #
# # You may modify this file any way you like (except for the handin
# # rule). You instructor will type "make" on your specific Makefile to
# # build your proxy from sources.

# CC = gcc
# CFLAGS = -g -Wall
# LDFLAGS = -lpthread

# all: echo_server echo_client

# csapp.o: csapp.c csapp.h
#    $(CC) $(CFLAGS) -c csapp.c

# # 에코서버 테스트------------------------------------------------------
# echo_client: echoclient.c csapp.o
#    $(CC) $(CFLAGS) -o echo_client echoclient.c csapp.o $(LDFLAGS)

# echo_server: echoserveri.c csapp.o
#    $(CC) $(CFLAGS) -o echo_server echoserveri.c csapp.o $(LDFLAGS)

# csapp.o: csapp.c
#    $(CC) $(CFLAGS) -c csapp.c
# # 에코서버 테스트------------------------------------------------------

# proxy.o: proxy.c csapp.h
#    $(CC) $(CFLAGS) -c proxy.c

# proxy: proxy.o csapp.o
#    $(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

# # Creates a tarball in ../proxylab-handin.tar that you can then
# # hand in. DO NOT MODIFY THIS!
# handin:
#    (make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

# clean:
#    rm -f *~ *.o echo_client echo_server core *.tar *.zip *.gzip *.bzip *.gz

# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o
	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c
echoclient: echoclient.o csapp.o
	$(CC) $(CFLAGS) echoclient.o csapp.o -o echoclient $(LDFLAGS)

echoserveri.o: echoserveri.c csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c
echoserveri: echoserveri.o csapp.o
	$(CC) $(CFLAGS) echoserveri.o csapp.o -o echoserver $(LDFLAGS)

echoclean:
	rm -f *.o echoclient echoserver

# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz


