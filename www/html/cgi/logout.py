#!/usr/bin/env python3
import os

SESSIONS_FILE = "/tmp/webserv_sessions.txt"

def parse_cookies():
    """Parse cookies from HTTP_COOKIE environment variable"""
    cookies = {}
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    
    if cookie_header:
        for cookie in cookie_header.split(';'):
            cookie = cookie.strip()
            if '=' in cookie:
                name, value = cookie.split('=', 1)
                cookies[name.strip()] = value.strip()
    
    return cookies

def remove_session(session_id):
    """Remove session from file"""
    try:
        if not os.path.exists(SESSIONS_FILE):
            return
        
        with open(SESSIONS_FILE, 'r') as f:
            lines = f.readlines()
        
        with open(SESSIONS_FILE, 'w') as f:
            for line in lines:
                if not line.startswith(session_id + ':'):
                    f.write(line)
    except:
        pass

def main():
    cookies = parse_cookies()
    session_id = cookies.get('session_id', '')
    
    if session_id:
        remove_session(session_id)

    print("Status: 200 OK")
    print("Content-Type: text/html")
    print("Set-Cookie: session_id=; Max-Age=0; Path=/")
    print("\r\n\r\n")
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Logged Out</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
        }

        .navbar {
            background-color: #333;
            padding: 15px 20px;
            display: flex;
            justify-content: center;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }

        .navbar h2 {
            color: white;
            font-size: 20px;
        }

        .content {
            max-width: 600px;
            margin: 40px auto;
            padding: 40px;
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }

        .icon {
            text-align: center;
            font-size: 64px;
            color: #22c55e;
            margin-bottom: 20px;
        }

        h1 {
            color: #333;
            margin-bottom: 10px;
            text-align: center;
        }

        .message {
            color: #666;
            text-align: center;
            margin: 20px 0;
            line-height: 1.6;
        }

        .info-box {
            background-color: #f9f9f9;
            padding: 20px;
            border-radius: 4px;
            border-left: 4px solid #555;
            margin: 20px 0;
        }

        .info-box p {
            color: #666;
            margin: 8px 0;
            line-height: 1.6;
        }

        .info-box strong {
            color: #333;
        }

        .btn {
            background-color: #555;
            color: white;
            border: none;
            padding: 12px 24px;
            font-size: 16px;
            cursor: pointer;
            border-radius: 4px;
            text-decoration: none;
            display: inline-block;
            transition: background-color 0.3s ease;
            margin-top: 20px;
        }

        .btn:hover {
            background-color: #777;
        }

        .btn:active {
            background-color: #999;
        }

        .button-group {
            text-align: center;
        }
    </style>
</head>
<body>
    <nav class="navbar">
        <h2>Logout</h2>
    </nav>

    <div class="content">
        <div class="icon">✓</div>
        
        <h1>Logged Out Successfully</h1>
        <p class="message">You have been successfully logged out. Your session has been terminated and cookies have been cleared.</p>
        
        <div class="info-box">
            <p><strong>✓ Session removed</strong></p>
            <p><strong>✓ Cookies cleared</strong></p>
            <p><strong>✓ All authentication data deleted</strong></p>
        </div>
        
        <p class="message">Thank you for using our cookie and session management system!</p>
        
        <div class="button-group">
            <a href="/login.html" class="btn">Login Again</a>
        </div>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
