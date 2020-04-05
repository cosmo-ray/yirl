#Make the image:

```
docker build -v $PWD:/yirl -t yirl-build ./docker
```

#Build YIRL

```
docker run -v $PWD:/yirl yirl-build scl enable devtoolset-9 ./build/docker/make.sh
```
you can choose the gcc version between 8-9 by changing the devtool used