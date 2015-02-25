all: lock barrier 

lock: lock.cc hrtimer_x86.c hrtimer_x86.h
		g++ lock.cc hrtimer_x86.c -lpthread -o lock

barrier: barrier.c hrtimer_x86.c hrtimer_x86.h
		g++ barrier.c hrtimer_x86.c -lpthread -o barrier

clean:
	-rm barrier
	-rm lock