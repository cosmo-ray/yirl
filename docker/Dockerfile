FROM rockylinux:9

RUN echo "assumeyes=1" >> /etc/yum.conf

RUN yum search epel-release
RUN yum info epel-release
RUN yum install -y  epel-release wget
RUN dnf config-manager --set-enabled crb
RUN yum update -y

RUN yum install -y lua-devel git autoconf make cmake3 json-c-devel ncurses-devel mpg123-devel mesa-libGL-devel mesa-libGLU-devel perl-devel perl-ExtUtils-Embed.noarch

RUN yum install -y glib2-devel SDL2-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_image-devel libglvnd-opengl mesa-libGL libglvnd-glx

RUN yum clean all
