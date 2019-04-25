default:
	gcc -Wall fscheck.c -o fscheck
	./fscheck fs.img

clean:
	rm fscheck fs.img
	cp fs-clean.img fs.img

mv:
	mv fs.img fsfu/
