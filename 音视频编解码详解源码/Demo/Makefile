CPP=cl
pathObj=demo1.obj max1.obj min1.obj
demo.exe: $(pathObj)
	link /OUT:demo.exe $(pathObj) 

max1.o: max1.cpp
	$(CPP) max1.cpp
min1.o: min1.cpp
	$(CPP) min1.cpp
demo1.o: demo1.cpp
	$(CPP) demo1.cpp


clean:
	rm -f *.obj
	rm -f *.exe