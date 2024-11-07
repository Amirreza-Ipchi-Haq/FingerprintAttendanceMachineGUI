CC=gcc
EDIT=vi
ICON=icon.ico
REQUIREMENTS=requirements.txt
SRC=main.c
TARGET=FingerprintAttendanceMachineGUI
WINDRES=windres
edit:
	${EDIT} ${SRC}
compileLinux:
	${CC} ${SRC} -o ${TARGET}Linux `pkg-config --cflags --libs gtk4` -static-libgcc -Wall -Wextra
compileWindows:
	echo 'MAINICON ICON "${ICON}"'>resource.rc
	${WINDRES} -O coff -o resource.res resource.rc
	${CC} ${SRC} -o ${TARGET}Windows.exe resource.res `pkg-config --cflags --libs gtk4` -mwindows -static-libgcc -Wall -Wextra
	rm resource.rc resource.res
	mkdir Program
	mv ${TARGET}Windows.exe Program
	for i in `cat ${REQUIREMENTS}`;\
		do cp /mingw32/bin/$$i Program;\
	done
run:
	./${TARGET}Linux
