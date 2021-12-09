# httpserver
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/qdslovelife/httpserver.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/qdslovelife/httpserver/context:cpp)

A simple HTTP server written in C that runs on GNU/Linux.

- [httpserver](#httpserver)
  - [Feature](#feature)
  - [Build](#build)
  - [Run](#run)
    - [Static page](#static-page)
    - [CGI & POST](#cgi--post)
    - [Log](#log)
  - [Bench](#bench)

## Feature

- [√] HTTP/1.1 support only
- [√] Using epoll for concurrency
- [√] Support GET, HEAD, POST
- [√] Keep-Alive support
- [√] Support many status codes, including 200, 400, 404, 408, 500, 501, 505
- [√] Can handle timeout connections
- [√] CGI support
- [√] Simple Logging
  - The log format: [Apache Log](https://httpd.apache.org/docs/2.4/logs.html)

## Build

``` bash
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug
cd build && make
```

## Run

``` bash
cd build
./server --http=9999 --log=test.log --www=../static_site --cgi=../cgi
```

### Static page

![运行截图](./image/运行截图.png)

### CGI & POST

TODO(dashuai): Long strings will cause memory leak.

**URL: 127.0.0.1:9999/cgi/?text=asciiart**

![asciiart](./image/asciiart.png)

### Log

![日志截图](./image/日志截图.png)

## Bench

Apache Bench(Short Link)

![](./image/ab短连接.png)

Apache Bench(Long Link)

![](./image/ab长连接.png)
