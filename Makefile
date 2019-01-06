myShell: myShell.o parser.o command.o
	g++ -o myShell myShell.o parser.o command.o
myShell.o: myShell.cpp myShell.h
	g++ -std=gnu++98 -pedantic -Wall -Werror -c myShell.cpp 
parser.o: parser.cpp parser.h
	g++ -std=gnu++98 -pedantic -Wall -Werror -c parser.cpp
command.o: command.cpp command.h
	g++ -std=gnu++98 -pedantic -Wall -Werror -c command.cpp 

clean:
	rm myShell myShell.o parser.o command.o
