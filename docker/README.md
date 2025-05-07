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
docker run -v $PWD:/yirl yirl-build /yirl/docker/make.sh
```

Notes:

- I'm pretty bad at docker, so thoses files can still be improve greatly
- Also I've used podman not docker to test this
- the packages will be store in `docker-package/`
