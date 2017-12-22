all:main.exe


main.exe:main.c parser.c token.c scanner.c
	cl /c /TP /EHsc main.c
	cl /c /TP /EHsc parser.c
	cl /c /TP /EHsc scanner.c
	cl /c /TP /EHsc token.c
	cl /c /TP /EHsc temp.c
	link /subsystem:console main.obj parser.obj scanner.obj token.obj temp.obj /out:main.exe
clean:
	rm main.exe 
	rm main.obj 
	rm parser.obj 
	rm scanner.obj
	rm token.obj
	rm temp.obj
