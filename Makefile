BOOST_PATH ?= /usr/
OBJDIR = bin
SRCDIR = src

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/ -g -Wall -std=c++11 -fopenmp

LD_FLAGS = 

all: $(OBJDIR)/estimate_weights $(OBJDIR)/editdist $(OBJDIR)/closest_word

clean:
	rm -rf $(OBJDIR)

$(OBJDIR)/%: $(SRCDIR)/%.cpp $(SRCDIR)/Util.hpp $(SRCDIR)/Levenshtein.hpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) -o $@
