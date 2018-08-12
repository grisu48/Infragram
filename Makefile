

# Default compiler and compiler flags
CXX=g++
CC=gcc

# Default flags for all compilers
O_FLAGS=-O3 -Wall -Wextra -pedantic
# Debugging flags
#O_FLAGS=-Og -g2 -Wall -Wextra -pedantic
CXX_FLAGS=$(O_FLAGS) -std=c++11
CC_FLAGS=$(O_FLAGS) -std=c99


# Binaries, object files, libraries and stuff
LIBS=-ljpeg
INCLUDE=
OBJS=
BINS=infragram ndvi


# Default generic instructions
default:	all
all:	$(OBJS) $(BINS)
clean:	
	rm -f *.o
# Object files
%.o:	%.cpp %.hpp
	$(CXX) $(CXX_FLAGS) -c $(INCLUDE) -o $@ $< $(LIBS) 
	
infragram:	infragram.cpp jpeg.hpp ndvi.hpp
	$(CXX) $(CXX_FLAGS) $(INCLUDE) -o $@ $< $(LIBS)

ndvi:	ndvi.cpp ndvi.hpp jpeg.hpp
	$(CXX) $(CXX_FLAGS) $(INCLUDE) -o $@ $< $(LIBS)

tests:	jpeg

jpeg:	jpeg.cpp jpeg.hpp
	$(CXX) $(CXX_FLAGS) $(INCLUDE) -o $@ $< $(LIBS)
