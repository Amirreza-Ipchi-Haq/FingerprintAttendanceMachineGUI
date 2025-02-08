CC=gcc
CCLOCATION=/mingw32/bin
DOCKER=fingerprintattendancemachineguilinux
EDIT=vi
ICON=icon.ico
REQUIREMENTS=Requirements.txt
SRC=main.c
TARGET=FingerprintAttendanceMachineGUI
WINDRES=windres
help:
	echo "Makefile commands:\n\tcommit [COMMITMESSAGE=<message>]    Commit to the repository using\n\t                                    <message> as the commit message\n\t                                    (Leave empty to enter description as well)\n\tcompileLinux                        Compile for Linux\n\tcompileOSX                          Compile for OSX\n\tcompileWindows [CCLOCATION=<dlls>]  Compile for Windows (With <dlls> as its\n\t                                    dependency location)\n\tedit [EDIT=<editor>]                Edit the source code (using Vim by default)\n\tfetch                               Fetch updates from the repository\n\thelp                                Show help\n\tpush                                Push updates to the repository\n\trun                                 Run on Linux"
commit:
	git add .
	test -n "${COMMITMESSAGE}"&&git commit -m "${COMMITMESSAGE}"||git commit
compileLinux:
	docker image ls -a|grep i386|grep debian|grep -q stable-slim||docker pull i386/debian:stable-slim
	docker container ls -a|grep -q ${DOCKER}||(docker build --network host --platform=linux/386 -t ${DOCKER} .&&docker run -d --rm --name ${DOCKER} ${DOCKER})
	docker exec -it ${DOCKER} bash -c "(test \`cat /progress\` -gt 0||(cd /build&&/glibc/configure --enable-static-nss --prefix=/libc&&make&&make install&&echo 1 >/progress))&&export LIBRARY_PATH='/libc/lib:/usr/lib/i386-linux-gnu:/usr/lib' CC='gcc -w -s -static -L/libc/lib -I/libc/include'&&(test \`cat /progress\` -gt 1||(cd /libglvnd/*/&&./autogen.sh&&./configure --build=i386-pc-linux-gnu --disable-shared CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32&&make&&make install-strip&&echo 2 >/progress))&&(test \`cat /progress\` -gt 2||(cd /util-linux/*/&&./autogen.sh&&./configure --disable-shared&&make&&make install-strip&&echo 3 >/progress))&&(test \`cat /progress\` -gt 3||(commands=();cd /wayland/*/;ls progress &>/dev/null||echo 0 >progress;while rm -rf build&&while ! ls build &>/dev/null||grep -iq \"downloading\|cloning\" build/meson-logs/meson-log.txt;do (meson wrap update --force||exit 1)&&! meson setup --reconfigure --wipe --prefix /usr --default-library static --prefer-static --strip -Ddebug=false -Dtests=false -Ddocumentation=false -Ddtd_validation=false . build&&(! grep -q 'Git command failed' build/meson-logs/meson-log.txt||exit 1);done;do test \`cat progress\` -lt \$${#commands[@]}&&eval \$${commands[\`cat progress\`]}&&expr \`cat progress\` + 1 >progress||exit 1;done&&cd build&&meson compile --ninja-args '-k 0'&&meson install&&echo 4 >/progress))&&(test \`cat /progress\` -gt 4||(commands=();cd /libxkbcommon/*/;ls progress &>/dev/null||echo 0 >progress;while rm -rf build&&while ! ls build &>/dev/null||grep -iq \"downloading\|cloning\" build/meson-logs/meson-log.txt;do (meson wrap update --force||exit 1)&&! meson setup --reconfigure --wipe --default-library static --prefer-static --strip -Ddebug=false -Denable-docs=false -Denable-xkbregistry=false . build&&(! grep -q 'Git command failed' build/meson-logs/meson-log.txt||exit 1);done;do test \`cat progress\` -lt \$${#commands[@]}&&eval \$${commands[\`cat progress\`]}&&expr \`cat progress\` + 1 >progress||exit 1;done&&cd build&&meson compile --ninja-args '-k 0'&&meson install&&echo 5 >/progress))&&(test \`cat /progress\` -gt 5||(commands=();cd /gtk4/*/;ls progress &>/dev/null||echo 0 >progress;while rm -rf build&&while ! ls build &>/dev/null||grep -iq \"downloading\|cloning\" build/meson-logs/meson-log.txt;do (meson wrap update --force||exit 1)&&! meson setup --reconfigure --wipe --prefix /usr --default-library static --prefer-static --strip -Ddebug=false -Dmedia-gstreamer=disabled -Dvulkan=disabled -Dbuild-examples=false -Dbuild-tests=false -Ddemos=false -Dbuild-examples=false -Dbuild-tests=false -Dprint-cups=disabled -Df16c=disabled -Dintrospection=disabled -Dglib:tests=false -Dlibepoxy:tests=false . build&&(! grep -q 'Git command failed' build/meson-logs/meson-log.txt||exit 1);done;do test \`cat progress\` -lt \$${#commands[@]}&&eval \$${commands[\`cat progress\`]}&&expr \`cat progress\` + 1 >progress||exit 1;done&&sed -i /g_module_close/d subprojects/glib-2.82.4/gio/gio-querymodules.c&&cd build&&meson compile --ninja-args '-k 0'&&meson install&&echo 6 >/progress))&&cd /&&\`printenv CC\` main.c -o ${TARGET}Linux \`find /gtk4/*/build /libxkbcommon/*/build -type d -not -name '*.a.p' -exec echo '-L{} ' \\;\` \`pkg-config --static --cflags --libs-only-other gtk4\` -lgtk -lgtk_css -lxkbcommon -lgdk -lwayland+cursor -lgsk -lpango-1.0 -lpangocairo-1.0 -lpangoft2-1.0 -lgio-2.0 -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lgdk_pixbuf-2.0 -lharfbuzz -lgraphene-1.0 -lcairo -lcairo-script-interpreter -lcairo-gobject -lz -lpng16 -lXrender -ltiff -llzma -ljpeg -ljpeg12 -ljpeg16 -lpixman-1 -lepoxy -lfribidi -lfontconfig -lfreetype -lexpat -lffi -lpcre2-8 -lpcre2-16 -lpcre2-32 -lpcre2-posix -lwayland-client -lwayland-egl -lXfixes -lXrandr -lXdamage -lXcursor -lXrender -lX11 -lxcb -lXau -lXdmcp -lXi -lXinerama -lXext -lmount -lblkid -lm -ldl&&strip -s ${TARGET}Linux"
	docker cp ${DOCKER}:/${TARGET}Linux .
	docker stop ${DOCKER}
	test -n "`docker image ls -aq ${DOCKER}`"&&docker rmi -f `docker image ls -aq ${DOCKER}`
compileOSX:
	${CC} ${SRC} -o ${TARGET}OSX `pkg-config --cflags --libs gtk4` -Wall -Wextra
	strip ${TARGET}OSX
	tar xzf Template.tar.gz
	cp `otool -L ${TARGET}OSX|cut -d ' ' -f 1|tail -n +2|sed /libSystem.B.dylib/d` ${TARGET}OSX.app/Contents/Frameworks
	mv ${TARGET}OSX ${TARGET}OSX.app/Contents/MacOS
compileWindows:
	echo 'MAINICON ICON "${ICON}"'>resource.rc
	${WINDRES} -O coff -o resource.res resource.rc
	${CC} ${SRC} -o ${TARGET}Windows.exe resource.res `pkg-config --cflags --libs gtk4` -mwindows -static-libgcc -Wall -Wextra -s
	rm resource.rc resource.res
	mkdir Program
	mv ${TARGET}Windows.exe Program
	for i in `cat ${REQUIREMENTS}`;\
		do cp ${CCLOCATION}/$$i Program;\
	done
	strip -s Program/*
edit:
	${EDIT} ${SRC}
fetch:
	git fetch
	curl https://raw.githubusercontent.com/Amirreza-Ipchi-Haq/dynastr/refs/heads/main/dynastr.h>dynastr.h
push:
	git push
run:
	./${TARGET}Linux
