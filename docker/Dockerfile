# YIRL build env
FROM centos:7

RUN yum search epel-release
RUN yum info epel-release
RUN yum install -y  epel-release wget
RUN wget http://www.nosuchhost.net/~cheese/fedora/packages/epel-7/x86_64/cheese-release-7-1.noarch.rpm
RUN rpm -Uvh cheese-release*rpm
RUN yum update -y

RUN yum install -y lua-devel git autoconf make cmake json-c-devel ncurses-devel centos-release-scl-rh mpg123-devel
RUN yum install -y glib2-devel SDL2-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_image-devel devtoolset-9-gcc-c++ devtoolset-8-gcc-c++

RUN yum clean all
