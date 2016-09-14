all: CImg.h a2.cpp
	g++ a2.cpp -o a2 -lX11 -ljpeg -lpthread -I. -I/usr/local/include -L/usr/local/lib -I/usr/X11/include -L/usr/X11/lib -lX11 -Isiftpp -O3 siftpp/sift.cpp

clean:
	rm a2
