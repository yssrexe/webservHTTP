#!/usr/bin/env python3
import os
import sys

# Print HTTP headers
print("Content-Type: text/html")
print("Status: 200 OK")
print("Set-Cookie: session=abc145523; HttpOnly; theme=dark")
print("\r\n\r\n")


# Start HTML response
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Data Test</title></head>")
print("<body>")
print("<h1>CGI Data Reception Test</h1>")

# Display request method
request_method = os.environ.get('REQUEST_METHOD')
print(f"<h2>Request Method: {request_method}</h2>")

# Display query string (GET parameters)
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print(f"<h3>Query String (GET):</h3>")
    print(f"<pre>{query_string}</pre>")

# Display POST data
if request_method == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH', '')
    if content_length:
        try:
            length = int(content_length)
            post_data = sys.stdin.read(length)
            print(f"<h3>POST Data:</h3>")
            print(f"<pre>{post_data}</pre>")
        except:
            print("<p>Error reading POST data</p>")

# Display all environment variables
print("<h3>Environment Variables:</h3>")
print("<table border='1' style='border-collapse: collapse;'>")
print("<tr><th>Variable</th><th>Value</th></tr>")
for key, value in sorted(os.environ.items()):
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")
print("</table>")

print("</body>")
print("</html>")
