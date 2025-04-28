CC = gcc
CFLAGS = -Wall -g
LIB = -lm
EXE=allocate

SRC = main.c linked_list.c process_utils.c

OBJ = $(SRC:.c=.o)
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LIB)

clean:
	rm -f $(OBJ) $(EXE)
