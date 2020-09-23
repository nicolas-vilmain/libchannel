
    libchannel README 

    description: Golang channel simple and basic C implation.

    Why: Just for fun, sure.
    
    Compil test: gcc -o test test.c src/channel.c -I src/ -O2 -W -Wall -lpthread
    Change speed: in channel_read[_nonblock](struct channel *chan) change usleep value.
    Use other thread lib: Update file src/lib.h
    Use: just import channel.h
    
    Note 1: update/delete xcalloc() in channel.c with your function.
    Note 2: channel_read_nonblock() if timeout < 1, lib use channel_read()
    
    Todo:
        - channel buffering size.
        
    Thanks for reading this document.
