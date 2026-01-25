#!/usr/bin/python3
import sys
import os

# Read POST data
content_length = os.environ.get('CONTENT_LENGTH', '')
content_length = int(content_length) if content_length and content_length.strip() else 0
post_data = sys.stdin.read(content_length) if content_length > 0 else ""

# Output
print("Content-Type: text/html")
print()
print("<h1>Python POST Result</h1>")
print("<h2>POST Data:</h2>")

if post_data:
    print("<pre>" + post_data + "</pre>")
else:
    print("<p style='color:red'>No POST data received!</p>")

print("<h2>Environment:</h2>")
print(f"<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"<p>CONTENT_LENGTH: {content_length}</p>")
print(f"<p>CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'N/A')}</p>")

print("<br><a href='/test-post.html'>Back</a>")
