# httpserver
[![CodeFactor](https://www.codefactor.io/repository/github/qdslovelife/httpserver/badge)](https://www.codefactor.io/repository/github/qdslovelife/httpserver)
![GitHub](https://img.shields.io/github/license/qdslovelife/httpserver)
![GitHub last commit](https://img.shields.io/github/last-commit/qdslovelife/httpserver)

一个用C编写的可在GNU/Linux上运行的简易HTTP服务器。

- [httpserver](#httpserver)
  - [特性](#特性)
  - [构建](#构建)
  - [运行](#运行)
    - [静态内容](#静态内容)
    - [日志](#日志)
  - [Bench](#bench)

## 特性

- [√] 仅支持HTTP/1.1
- [√] 使用epoll实现并发
- [√] 支持GET、HEAD、POST
- [√] 支持Keep-Alive
- [√] 支持多个状态码，包括200、400、404、408、500、501、505
- [√] 可以处理超时连接
- [√] 支持CGI
- [√] 简单的日志功能(TODO：Release模式下的日志有bug)
  - 日志格式为[Apache Log](https://httpd.apache.org/docs/2.4/logs.html)中的Common Log Format。

## 构建

``` cmake
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build && make
```

## 运行

``` bash
# 在build目录下
# ./server --http=9999 --log=<你的日志文件> --www=<你的静态内容的目录，末尾不加/> --cgi=<cgi目录，末尾不加/>
./server --http=9999 --log=test.log --www=static_site --cgi=../cgi
```

### 静态内容

![运行截图](./image/运行截图.png)

### 日志

![日志截图](./image/日志截图.png)

## Bench

Apache Bench短链接

![](./image/ab短连接.png)

Apache Bench长连接

![](./image/ab长连接.png)
