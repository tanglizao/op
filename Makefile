CP = cp
CC = g++
RM = rm -rf
MKDIR = mkdir -p

debug:DMODE = -g -rdynamic -DDEBUG
release:DMODE = -g -O2 -s -DNDEBUG

PACKDIR = $(HOME)/pack
INCLUDE = -I./ \
	  -I$(PACKDIR)/libevent/include

CFLAGS = $(DMODE) $(INCLUDE)

LIB = -lpthread -lrt \
      $(PACKDIR)/libevent/lib/libevent.a

SRC_CPP := main.cpp
CPP_OBJ := $(subst .cpp,.o,$(SRC_CPP))

SRC_CC	:=
CC_OBJ	:= $(subst .cc,.o,$(SRC_CC))

ALL_OBJ	:= $(CPP_OBJ) $(CC_OBJ)

TARGET = op

debug: $(TARGET)
release: $(TARGET)

$(TARGET): $(ALL_OBJ)
	@echo "making ......"
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIB)

$(CPP_OBJ): %.o: %.cpp
	$(CC) $(CFLAGS) -c $(subst .o,.cpp,$@)

$(CC_OBJ): %.o: %.cc
	$(CC) $(CFLAGS) -c $(subst .o,.cc,$@)

clean:
	$(RM) *.o $(TARGET) core*

install: clean release
	$(RM) *.tar.gz
	tar czvf $(TARGET)_$(shell ./$(TARGET) -h | grep version | cut -d ' ' -f 3).tar.gz \
	../$(TARGET) ../setup.txt
