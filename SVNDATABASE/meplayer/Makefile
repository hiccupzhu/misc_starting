lib = -lavformat -lavcodec -lavutil -lswscale -lavcore -lm -lz -lbz2 -lpthread
obj = meplayer.o matroska_reader_t.o ebml.o
debug = -g
all:meplayer
meplayer:$(obj)
	g++ $(debug) -o meplayer $(obj) $(lib)
	mv *.o obj/
meplayer.o:meplayer.cpp
	g++ $(debug) -c -o meplayer.o meplayer.cpp $(lib)
matroska_reader_t.o:matroska_reader_t.cpp
	g++ $(debug) -c -o matroska_reader_t.o matroska_reader_t.cpp $(lib)
ebml.o:ebml.cpp
	g++ $(debug) -c -o ebml.o ebml.cpp $(lib)
clean:
	rm $(obj)