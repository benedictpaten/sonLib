
BIN = ./bin
SRC = ./src
INC = ./include
OBJ = ./obj

COMMON = $(OBJ)/align.o \
	$(OBJ)/cluster.o \
	$(OBJ)/distancemat.o \
	$(OBJ)/options.o \
	$(OBJ)/util.o \
	$(OBJ)/sequence.o

QUICKTREEOBJ = $(COMMON) $(OBJ)/quicktree.o $(OBJ)/buildtree.o $(OBJ)/tree.o

CFLAGS  = -c -I$(INC) -O3 -Wall

LFLAGS = -g 

all: $(BIN)/quicktree

quicktree : $(BIN)/quicktree

$(BIN)/quicktree : $(QUICKTREEOBJ)
	$(CC) $(LFLAGS) -o $@ $(QUICKTREEOBJ) -lm

$(OBJ)/quicktree.o : $(SRC)/quicktree.c 
	$(CC) $(CFLAGS) -o $(OBJ)/quicktree.o $(SRC)/quicktree.c

$(OBJ)/align.o : $(SRC)/align.c $(INC)/align.h
	$(CC) $(CFLAGS) -o $(OBJ)/align.o $(SRC)/align.c

$(OBJ)/buildtree.o : $(SRC)/buildtree.c $(INC)/buildtree.h
	$(CC) $(CFLAGS) -o $(OBJ)/buildtree.o $(SRC)/buildtree.c

$(OBJ)/cluster.o : $(SRC)/cluster.c $(INC)/cluster.h
	$(CC) $(CFLAGS) -o $(OBJ)/cluster.o $(SRC)/cluster.c

$(OBJ)/distancemat.o : $(SRC)/distancemat.c $(INC)/distancemat.h
	$(CC) $(CFLAGS) -o $(OBJ)/distancemat.o $(SRC)/distancemat.c

$(OBJ)/sequence.o : $(SRC)/sequence.c $(INC)/sequence.h
	$(CC) $(CFLAGS) -o $(OBJ)/sequence.o $(SRC)/sequence.c

$(OBJ)/tree.o : $(SRC)/tree.c $(INC)/tree.h
	$(CC) $(CFLAGS) -o $(OBJ)/tree.o $(SRC)/tree.c

$(OBJ)/util.o : $(SRC)/util.c $(INC)/util.h
	$(CC) $(CFLAGS) -o $(OBJ)/util.o $(SRC)/util.c

$(OBJ)/options.o : $(SRC)/options.c $(INC)/options.h
	$(CC) $(CFLAGS) -o $(OBJ)/options.o $(SRC)/options.c

clean:
	rm -rf $(OBJ)/*.o
