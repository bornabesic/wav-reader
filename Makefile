
all:
	gcc -s -Os wav_reader.c -o wav_reader

clean:
	rm -rf wav_reader
