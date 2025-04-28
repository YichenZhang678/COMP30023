EXE=fetchmail

$(EXE): main.c
	cc -Wall -o $(EXE) $<

format:
	clang-format -style=file -i *.c

clean:
	rm -f $(EXE)
