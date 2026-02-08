#!/usr/bin/python3
import hashlib
import random
import string
import os
import sys
from urllib.parse import parse_qs

SESSIONS_FILE = "/tmp/webserv_sessions.txt"
USERS_FILE = "/tmp/webserv_users.txt"
ACCOUNTS_FILE = "/tmp/webserv_accounts.txt"

def generate_session_id():
    """Generate a random session ID"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=32))

def get_user_reload_count(username):
    """Get the reload count for a specific user"""
    try:
        if os.path.exists(USERS_FILE):
            with open(USERS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split(':')
                    if len(parts) >= 2 and parts[0] == username:
                        return int(parts[1])
    except:
        pass
    return 0

def save_session(session_id, username):
    """Save session to file with user's current reload count"""
    try:
        reload_count = get_user_reload_count(username)
        with open(SESSIONS_FILE, 'a') as f:
            f.write(f"{session_id}:{username}:{reload_count}\n")
    except:
        pass

def verify_credentials(username, password):
    """Verify username and password against accounts file"""
    try:
        if os.path.exists(ACCOUNTS_FILE):
            with open(ACCOUNTS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split(':')
                    if len(parts) >= 2 and parts[0] == username and parts[1] == password:
                        return True
    except:
        pass
    return False

def user_exists(username):
    """Check if username exists in accounts file"""
    try:
        if os.path.exists(ACCOUNTS_FILE):
            with open(ACCOUNTS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split(':')
                    if len(parts) >= 2 and parts[0] == username:
                        return True
    except:
        pass
    return False

def main():
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
    except ValueError:
        content_length = 0
    
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        form_data = parse_qs(post_data)
        username = form_data.get('username', [''])[0]
        password = form_data.get('password', [''])[0]
    else:
        username = ''
        password = ''
    
    if verify_credentials(username, password):
        session_id = generate_session_id()
        
        save_session(session_id, username)

        print("Content-Type: text/html")
        print(f"Set-Cookie: session_id={session_id}; Max-Age=3600; Path=/")
        print(f"Set-Cookie: username={username}; Max-Age=3600; Path=/")
        print("\r\n\r\n")
        print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="refresh" content="0; url=/profile.html">
    <title>Login Success</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }}

        .content {{
            max-width: 600px;
            padding: 40px;
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            text-align: center;
        }}

        .success-icon {{
            font-size: 64px;
            color: #22c55e;
            margin-bottom: 20px;
        }}

        h1 {{
            color: #333;
            margin-bottom: 10px;
        }}

        .message {{
            color: #666;
            margin: 20px 0;
            line-height: 1.6;
        }}

        .spinner {{
            border: 4px solid #f3f3f3;
            border-top: 4px solid #555;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }}

        @keyframes spin {{
            0% {{ transform: rotate(0deg); }}
            100% {{ transform: rotate(360deg); }}
        }}
    </style>
</head>
<body>
    <div class="content">
        <div class="success-icon">✓</div>
        <h1>Login Successful!</h1>
        <p class="message">Welcome, <strong>{username}</strong>!</p>
        <p class="message">Redirecting to your profile...</p>
        <div class="spinner"></div>
    </div>
    <script>
        // Fallback redirect if meta refresh doesn't work
        setTimeout(function() {{
            window.location.href = '/profile.html';
        }}, 100);
    </script>
</body>
</html>""")
    elif user_exists(username):
        send_wrong_password_error()
    else:
        send_user_not_found_error(username)

def send_wrong_password_error():
    """Send error response for wrong password"""
    print("Content-Type: text/html")
    print("\r\n\r\n")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Failed</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
        }}

        .navbar {{
            background-color: #333;
            padding: 15px 20px;
            display: flex;
            justify-content: center;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }}

        .navbar h2 {{
            color: white;
            font-size: 20px;
        }}

        .content {{
            max-width: 600px;
            margin: 40px auto;
            padding: 40px;
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}

        .error-icon {{
            text-align: center;
            font-size: 64px;
            color: #ef4444;
            margin-bottom: 20px;
        }}

        h1 {{
            color: #333;
            margin-bottom: 10px;
            text-align: center;
        }}

        .message {{
            color: #666;
            text-align: center;
            margin: 20px 0;
            line-height: 1.6;
        }}

        .btn {{
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
        }}

        .btn:hover {{
            background-color: #777;
        }}

        .btn:active {{
            background-color: #999;
        }}

        .button-group {{
            text-align: center;
        }}
    </style>
</head>
<body>
    <nav class="navbar">
        <h2>Login System</h2>
    </nav>

    <div class="content">
        <div class="error-icon">✗</div>
        <h1>Login Failed</h1>
        <p class="message">Invalid password. Please try again.</p>
        <div class="button-group">
            <a href="/login.html" class="btn">Back to Login</a>
        </div>
    </div>
</body>
</html>""")

def send_user_not_found_error(username):
    """Send error response with redirect to registration"""
    print("Content-Type: text/html")
    print("\r\n\r\n")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>User Not Found</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
        }}

        .navbar {{
            background-color: #333;
            padding: 15px 20px;
            display: flex;
            justify-content: center;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }}

        .navbar h2 {{
            color: white;
            font-size: 20px;
        }}

        .content {{
            max-width: 600px;
            margin: 40px auto;
            padding: 40px;
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}

        .warning-icon {{
            text-align: center;
            font-size: 64px;
            color: #f59e0b;
            margin-bottom: 20px;
        }}

        h1 {{
            color: #333;
            margin-bottom: 10px;
            text-align: center;
        }}

        .message {{
            color: #666;
            text-align: center;
            margin: 20px 0;
            line-height: 1.6;
        }}

        .btn {{
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
            margin: 10px 5px;
        }}

        .btn:hover {{
            background-color: #777;
        }}

        .btn:active {{
            background-color: #999;
        }}

        .btn-primary {{
            background-color: #22c55e;
        }}

        .btn-primary:hover {{
            background-color: #16a34a;
        }}

        .button-group {{
            text-align: center;
            display: flex;
            gap: 10px;
            justify-content: center;
            flex-wrap: wrap;
        }}
    </style>
</head>
<body>
    <nav class="navbar">
        <h2>Login System</h2>
    </nav>

    <div class="content">
        <div class="warning-icon">⚠</div>
        <h1>Account Not Found</h1>
        <p class="message">The username "<strong>{username}</strong>" does not exist in our system.</p>
        <p class="message">Would you like to create a new account?</p>
        <div class="button-group">
            <a href="/register.html" class="btn btn-primary">Create Account</a>
            <a href="/login.html" class="btn">Back to Login</a>
        </div>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
