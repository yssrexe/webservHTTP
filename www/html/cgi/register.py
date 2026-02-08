#!/usr/bin/python3
import os
import sys
from urllib.parse import parse_qs

ACCOUNTS_FILE = "/tmp/webserv_accounts.txt"

def user_exists(username):
    """Check if username already exists"""
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

def save_account(username, password):
    """Save new account to file"""
    try:
        with open(ACCOUNTS_FILE, 'a') as f:
            f.write(f"{username}:{password}\n")
        return True
    except:
        return False

def main():
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
    except ValueError:
        content_length = 0
    
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        form_data = parse_qs(post_data)
        username = form_data.get('username', [''])[0].strip()
        password = form_data.get('password', [''])[0]
        confirm_password = form_data.get('confirm_password', [''])[0]
    else:
        username = ''
        password = ''
        confirm_password = ''
    
    if not username or not password:
        send_error_response("Username and password are required.")
        return
    
    if len(username) < 3:
        send_error_response("Username must be at least 3 characters long.")
        return
    
    if len(password) < 4:
        send_error_response("Password must be at least 4 characters long.")
        return
    
    if password != confirm_password:
        send_error_response("Passwords do not match.")
        return
    
    if user_exists(username):
        send_error_response("Username already exists. Please choose a different username.")
        return
    
    if save_account(username, password):
        send_success_response(username)
    else:
        send_error_response("Failed to create account. Please try again.")

def send_success_response(username):
    """Send success response"""
    print("Status: 200 OK")
    print("Content-Type: text/html")
    print("\r\n\r\n")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Registration Successful</title>
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

        .success-icon {{
            text-align: center;
            font-size: 64px;
            color: #22c55e;
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
        <h2>Registration System</h2>
    </nav>

    <div class="content">
        <div class="success-icon">✓</div>
        <h1>Account Created Successfully!</h1>
        <p class="message">Welcome, <strong>{username}</strong>! Your account has been created.</p>
        <p class="message">You can now login with your credentials.</p>
        <div class="button-group">
            <a href="/login.html" class="btn">Go to Login</a>
        </div>
    </div>
</body>
</html>""")

def send_error_response(error_message):
    """Send error response"""
    print("Content-Type: text/html")
    print("\r\n\r\n")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Registration Failed</title>
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
        <h2>Registration System</h2>
    </nav>

    <div class="content">
        <div class="error-icon">✗</div>
        <h1>Registration Failed</h1>
        <p class="message">{error_message}</p>
        <div class="button-group">
            <a href="/register.html" class="btn">Back to Registration</a>
        </div>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
