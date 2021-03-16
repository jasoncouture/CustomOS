@echo off
docker build -t kernel-build-image . 
docker run --name kernel-build kernel-build-image
docker cp kernel-build:/data bin
move bin\data\*.* bin\
rmdir bin\data
docker rm kernel-build -f