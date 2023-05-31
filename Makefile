sampleobjects = buffer_manager.o file_manager.o  kdbtree.o

sample_run : $(sampleobjects)
	     g++ -std=c++11 -o kdbtree $(sampleobjects)

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

kdbtree.o : kdbtree.cpp
	g++ -std=c++11 -c kdbtree.cpp

clean :
	rm -f *.o
	rm -f kdbtree
	rm -f KDBTree.txt
	rm -f 2019CS10376_output.txt
