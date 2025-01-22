FROM i386/debian:stable-slim
WORKDIR /
COPY dynastr.h main.c .
RUN bash -c 'echo -e "Acquire::https::Verify-Peer \"false\";\nAcquire::https::Verify-Host \"false\";"'>/etc/apt/apt.conf.d/no-ca-certificate.conf
RUN bash -c 'echo -e "deb https://deb.debian.org/debian bookworm main non-free-firmware\ndeb-src https://deb.debian.org/debian bookworm main non-free-firmware\ndeb https://security.debian.org/debian-security bookworm-security main non-free-firmware\ndeb-src https://security.debian.org/debian-security bookworm-security main non-free-firmware\ndeb https://deb.debian.org/debian bookworm-updates main non-free-firmware\ndeb-src https://deb.debian.org/debian bookworm-updates main non-free-firmware"'>/etc/apt/sources.list
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y automake autopoint bison dpkg-source-gitarchive gawk gettext libffi-dev liblzma-dev libtool libwayland-bin libx11-dev libxcb-xkb-dev libxcursor-dev libxdamage-dev libxext-dev libxi-dev libxinerama-dev libxml2-dev libxrandr-dev pkgconf python3-pip shared-mime-info wayland-protocols x11proto-gl-dev
RUN git clone --depth 1 git://sourceware.org/git/glibc.git
RUN mkdir /build /gtk4 /libc /libglvnd /libxkbcommon /util-linux /wayland
WORKDIR /gtk4
RUN apt-get source -y gtk4
WORKDIR /libglvnd
RUN apt-get source -y libglvnd
WORKDIR /libxkbcommon
RUN apt-get source -y libxkbcommon
WORKDIR /util-linux
RUN apt-get source -y util-linux
WORKDIR /wayland
RUN apt-get source -y wayland
RUN apt-get autoremove --purge -y
RUN apt-get clean
RUN pip install --break-system-packages jinja2 markdown markupsafe meson ninja packaging pygments typogrify
RUN echo 0 >/progress
ENTRYPOINT ["tail","-f","/dev/null"]
