# YIRL build env
FROM centos:7

RUN sed -i s/mirror.centos.org/vault.centos.org/g /etc/yum.repos.d/*.repo
RUN sed -i s/^#.*baseurl=http/baseurl=http/g /etc/yum.repos.d/*.repo
RUN sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo
RUN sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo

RUN yum search epel-release
RUN yum info epel-release
RUN yum install -y  epel-release wget
RUN wget http://www.nosuchhost.net/~cheese/fedora/packages/epel-7/x86_64/cheese-release-7-1.noarch.rpm
RUN rpm -Uvh cheese-release*rpm
RUN yum update -y

RUN yum install -y lua-devel git autoconf make cmake3 json-c-devel ncurses-devel mpg123-devel mesa-libGL-devel mesa-libGLU-devel perl-devel perl-ExtUtils-Embed.noarch
RUN yum install -y centos-release-scl

RUN sed -i s/mirror.centos.org/vault.centos.org/g /etc/yum.repos.d/*.repo
RUN sed -i s/^#.*baseurl=http/baseurl=http/g /etc/yum.repos.d/*.repo
RUN sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo
RUN sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo

RUN yum install -y glib2-devel SDL2-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_image-devel devtoolset-9-gcc-c++ devtoolset-8-gcc-c++ libglvnd-opengl mesa-libGL libglvnd-glx

RUN ln -s /usr/bin/cmake3 /usr/bin/cmake

RUN yum clean all
