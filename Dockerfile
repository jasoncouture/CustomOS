FROM ubuntu:20.10

RUN apt update
RUN apt install nasm -y
RUN apt install mtools -y
RUN apt install build-essential -y
RUN apt install dosfstools -y

WORKDIR /root/src
COPY . .
VOLUME "/data"
RUN chmod +x ./export.sh
RUN cd /root/src && make clean
ENTRYPOINT "./export.sh"