FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libmysqlclient-dev \
    mysql-client \
    && rm -rf /var/lib/apt/lists/*

COPY . /app

RUN make clean && make

EXPOSE 8080

CMD ["./server"]

