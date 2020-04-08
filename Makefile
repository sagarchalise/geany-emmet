all: build
	
build:	
	g++ -DLOCALEDIR=\"\" -DGETTEXT_PACKAGE=\"geany-emmet\" -c ./geany-emmet.cpp -fPIC `pkg-config --cflags geany mozjs-68`
	g++ geany-emmet.o -o geany-emmet.so -shared `pkg-config --libs geany mozjs-68`

install: uninstall startinstall

startinstall:
	mkdir -p ~/.config/geany/plugins/emmet
	cp -f ./geany-emmet.so ~/.config/geany/plugins
	chmod 755 ~/.config/geany/plugins/geany-emmet.so
	ln -s `pwd`/editor.js ~/.config/geany/plugins/emmet/
# 	ln -s `pwd`/emmet.js ~/.config/geany/plugins/emmet/

uninstall:
	rm -f ~/.config/geany/plugins/geany-emmet*
	rm -rf ~/.config/geany/plugins/emmet/editor.js

clean:
	rm -f ./geany-emmet.so
	rm -f ./geany-emmet.o
