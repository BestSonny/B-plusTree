#   Include Make.common file

include Make.common

#   Defining targets

all: $(OUTPUT)

# For compiling main file

$(OUTPUT) : $(MAIN) $(HEADER)
	$(CXX) $(CXXFLAGS) $(OFLAG) $(OUTPUT) $(MAIN)

# For cleaning .o files

clean :
	rm -f *.o
