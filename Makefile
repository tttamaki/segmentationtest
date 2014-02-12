OPENGM_EXTSRC = /Users/tamaki/dev/opengm/src/external
OPENGM_EXTLIB = /Users/tamaki/dev/opengm/build/src/external
OPENGM_INC    = /Users/tamaki/dev/opengm/include
EXTLIBS = -lboost_program_options-mt


EXTLIBS += -I$(OPENGM_EXTSRC)/MaxFlow-v3.02.src-patched/ -lexternal-library-maxflow -DWITH_MAXFLOW


OPTIONS =  -I$(OPENGM_INC) \
			`pkg-config --cflags opencv eigen3` `pkg-config --libs opencv eigen3` \
			-L$(OPENGM_EXTLIB) $(EXTLIBS) $(CFLAGS) -Wall -std=c++11 \
            -L/opt/local/lib -I/usr/local/include
ifdef DEBUG
  OPTIONS+= -O0 -g -DDEBUG
else
  OPTIONS+= -O3 -DNDEBUG
endif


all:  segmentation


segmentation: segmentation.cxx readCSV.cxx
	g++ -o segmentation segmentation.cxx readCSV.cxx \
	$(OPTIONS)



clean:
	rm -f segmentation

