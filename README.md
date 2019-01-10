# fasthttp
fasthttp is a multithreaded, asynchronous HTTP server written in C++.

### Getting Started
1. Clone
2. Set up vagrant
3. make 
4. run

### Benchmarking
[TODO]

### Architecture
[TODO]

### Features
* Asynchronous request handling
* HTTP persistent connection (for HTTP/1.1)

### References
__HTTP__
1. https://www.jmarshall.com/easy/http/#httpspec
2. http://www.ntu.edu.sg/home/ehchua/programming/webprogramming/http_basics.html
3. https://github.com/h2o/picohttpparser

__Multithreading__
1. https://vichargrave.github.io/multithreaded-work-queue-based-server-in-cpp/

__Epoll__
1. https://xdecroc.wordpress.com/2016/03/16/using-epoll-io-event-notification-to-implement-an-asynchronous-server-unfinished/
2. https://jvns.ca/blog/2017/06/03/async-io-on-linux--select--poll--and-epoll/
3. https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/

__Misc__
1. http://berb.github.io/diploma-thesis/original/043_threadsevents.html
2. https://www.digitalocean.com/community/tutorials/how-to-benchmark-http-latency-with-wrk-on-ubuntu-14-04
3. https://medium.freecodecamp.org/million-requests-per-second-with-python-95c137af319
4. https://docs.oracle.com/cd/E23389_01/doc.11116/e21036/perf002.htm
5. https://stackoverflow.com/questions/17371462/most-efficient-high-performance-server-socket-thread-design
6. https://williamedwardscoder.tumblr.com/post/13590981677/performance-lessons-for-http-sockets
7. http://pl.atyp.us/content/tech/servers.html
8. https://stackoverflow.com/questions/17371462/most-efficient-high-performance-server-socket-thread-design