#!/usr/bin/env python3
import os
import sys
from datetime import datetime, timedelta
from urllib.parse import parse_qs

def get_query_params():
    """Parse query string parameters"""
    query_string = os.environ.get('QUERY_STRING', '')
    params = parse_qs(query_string)
    return params

def get_cookies():
    """Parse cookies from HTTP_COOKIE environment variable"""
    cookie_string = os.environ.get('HTTP_COOKIE', '')
    cookies = {}
    if cookie_string:
        for cookie in cookie_string.split('; '):
            if '=' in cookie:
                name, value = cookie.split('=', 1)
                cookies[name] = value
    return cookies

def set_cookie_action():
    """Set a test cookie"""
    expires = datetime.utcnow() + timedelta(days=1)
    expires_str = expires.strftime('%a, %d %b %Y %H:%M:%S GMT')
    
    # Send headers
    print("Content-Type: text/plain")
    print(f"Set-Cookie: serverCookie=ServerValue123; Expires={expires_str}; Path=/")
    print(f"Set-Cookie: timestamp={datetime.now().isoformat()}; Path=/")
    print()  # Empty line to separate headers from body
    
    # Send body
    print("Cookie Set Successfully!")
    print(f"Cookie name: serverCookie")
    print(f"Cookie value: ServerValue123")
    print(f"Expires: {expires_str}")

def read_cookies_action():
    """Read and display all cookies"""
    cookies = get_cookies()
    
    # Send headers
    print("Content-Type: text/plain")
    print()  # Empty line to separate headers from body
    
    # Send body
    if cookies:
        print("Cookies received by server:")
        print("-" * 40)
        for name, value in cookies.items():
            print(f"{name} = {value}")
        print("-" * 40)
        print(f"Total cookies: {len(cookies)}")
    else:
        print("No cookies found!")
        print("HTTP_COOKIE env:", os.environ.get('HTTP_COOKIE', 'NOT SET'))

def delete_cookie_action():
    """Delete the test cookie by setting it to expire in the past"""
    print("Content-Type: text/plain")
    print("Set-Cookie: serverCookie=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/")
    print("Set-Cookie: timestamp=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/")
    print()  # Empty line to separate headers from body
    
    print("Cookie Deletion Headers Sent!")
    print("The following cookies have been marked for deletion:")
    print("- serverCookie")
    print("- timestamp")

def main():
    try:
        # Get action from query parameters
        params = get_query_params()
        action = params.get('action', ['read'])[0]
        
        if action == 'set':
            set_cookie_action()
        elif action == 'delete':
            delete_cookie_action()
        else:  # default to read
            read_cookies_action()
            
    except Exception as e:
        print("Content-Type: text/plain")
        print()
        print(f"Error: {str(e)}")
        print(f"Type: {type(e).__name__}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
