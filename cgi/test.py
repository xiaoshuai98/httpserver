#! /usr/bin/python3
# -*- coding: UTF-8 -*-

str = input()

str_lists = str.split("&")
key = str_lists[0][4:]
value = str_lists[1][6:]

print("HTTP/1.1 200 OK")
print("Content-Type:text/html")
print("")
print("<html>")
print("<head>")
print("<meta charset=\"utf-8\">")
print("<title>CGI 测试实例</title>")
print("</head>")
print("<body>")
print("<h2>键值对 %s：%s</h2>" % (key, value))
print("</body>")
print("</html>")