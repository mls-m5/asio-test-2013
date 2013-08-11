CXXFLAGS =	 -g -Wall -fmessage-length=0 -std=c++0x `pkg-config --cflags freetype2 sdl ftgl` # -O2
OBJS = main.o
OBJS2 = server.o
LIBS = -L/opt/local/lib -lpthread 
TARGET = net
TARGET2 = server


$(TARGET):	$(OBJS) $(OBJS2)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -lboost_system
	$(CXX) -o $(TARGET2) $(OBJS2) $(LIBS) -lboost_system

all:	$(TARGET)

%.o: %.h

clean:
	rm -f $(OBJS) $(TARGET)
