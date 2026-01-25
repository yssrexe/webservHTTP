#!/usr/bin/python3

import time

print("Content-Type: text/html")
print("Status: 200 OK")
print("Set-Cookie: session=abc123; HttpOnly")
print("\r\n\r\n")


def add(a, b):
    return a + b

print("<html><body>")
print("<br>")
time.sleep(5)
print("hello Ayoub, Welcome to webserv project")
print("<br>")
print("\r\n", end='')
print(f"your age is {add(10, 12)}")
print("</body></html>")
