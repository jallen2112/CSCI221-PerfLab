##
##

CXX	=g++
CXXFLAGS= -static 
CXXOPT = -O3 -fopenmp 

goals: judge
	echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc
	$(CXX) $(CXXFLAGS) $(CXXOPT) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc 
##
## Parameters for the test run
##
FILTERS = gauss.filter vline.filter hline.filter emboss.filter ##sharpen.filter avg.filter edge.filter  
IMAGES = boats.bmp blocks-small.bmp
TRIALS = 1 2 3 4

judge: filter
	-./Judge -p ./filter -i boats.bmp
	-./Judge -p ./filter -i blocks-small.bmp

clean:
	-rm *.o
	-rm filter
	-rm filtered-*.bmp
