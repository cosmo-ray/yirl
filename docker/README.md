#Make the image:

```
docker build  -t yirl-build ./docker
```
or
```
docker build --no-cache -t yirl-build ./docker
```

#Build YIRL

```
docker run -v $PWD:/yirl yirl-build scl enable devtoolset-9 /yirl/docker/make.sh
```

Notes:

- You can choose the gcc version between 8-9 by changing the devtool used
- I've use Centos7 because it's old enough to be compatible with most recent distributions
- I'm pretty bad at docker, so thoses files can still be improve greatly
- Also I've used podman not docker to test this
- the packages will be store in `docker-package/`
