# Pour compiler ce code, il suffit de taper `make` dans un terminal (après avoir accédé au dossier où tu as le programme + le makefile)
# Si tout ce passe bien, un exécutable du nom `programme` apparaît dans le répertoire et tu peux l'exécuter en tapant `./programme`.

CXX = clang++
CXXFLAGS = -Wall -g -O2 -std=c++11 -DNDEBUG
LDFLAGS = -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio

TARGETS = puyo

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
