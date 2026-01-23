#!/usr/bin/python3


print("Content-Type: text/html")
print("Status: 200 OK")
print("Set-Cookie: session=abc123; HttpOnly")
print()


def add(a, b):
    return a + b

print("<html><body>")
print("<br>")
print("hello Ayoub, Welcome to webserv project")
print("<br>")
print(f"your age is {add(10, 12)}")
print("</body></html>")
