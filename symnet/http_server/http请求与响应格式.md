## http请求格式

* 请求格式
```txt
method SP request-target SP HTTP-version CRLF
*( header-field CRLF )
CRLF
[ message-body ]
```

* 示例
```txt
GET /index.html HTTP/1.1\r\n
Host: example.com\r\n
User-Agent: Mozilla/5.0\r\n
Accept: text/html\r\n
\r\n
```

* 响应格式
```txt
GET /index.html HTTP/1.1\r\n
Host: example.com\r\n
User-Agent: Mozilla/5.0\r\n
Accept: text/html\r\n
\r\n
```

* 示例
```txt
HTTP/1.1 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 13\r\n
\r\n
Hello, World!
```
