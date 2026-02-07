# WebServ HTTP Server - User Authentication System

## 📋 Table of Contents
1. [System Architecture Overview](#system-architecture-overview)
2. [Account Registration Flow](#account-registration-flow)
3. [Login & Authentication Flow](#login--authentication-flow)
4. [Cookie & Session Management](#cookie--session-management)
5. [Profile Display Flow](#profile-display-flow)
6. [Logout Flow](#logout-flow)
7. [Technical Implementation Details](#technical-implementation-details)

---

## 🏗️ System Architecture Overview

This is a custom HTTP web server written in C++ that supports:
- Static HTML serving
- CGI script execution (Python, PHP, Shell)
- Cookie-based session management
- User registration and authentication
- File uploads
- Auto-indexing

### Key Components:
- **C++ HTTP Server**: Handles HTTP requests, CGI execution, and routing
- **Python CGI Scripts**: Process registration, login, and logout
- **HTML/CSS/JavaScript Frontend**: User interface
- **File-based Storage**: Sessions and user accounts stored in `/tmp/` directory

---

## 👤 Account Registration Flow

### Step 1: User Opens Registration Page
**URL**: `http://127.2.15.1:8081/register.html`

**What Happens:**
1. Browser sends GET request to web server
2. Server routes through configuration: `route = /` matches with `ROOT=www/html/view`
3. Server reads and serves `www/html/view/register.html`
4. Browser displays registration form with fields:
   - Username (minimum 3 characters)
   - Password (minimum 4 characters)
   - Confirm Password

### Step 2: User Submits Registration Form
**Form Action**: `POST /cgi/register.py`

**What Happens:**
1. Browser collects form data:
   ```
   username=john
   password=mypassword123
   confirm_password=mypassword123
   ```

2. Browser sends POST request with form data in body:
   ```http
   POST /cgi/register.py HTTP/1.1
   Host: 127.2.15.1:8081
   Content-Type: application/x-www-form-urlencoded
   Content-Length: 65
   
   username=john&password=mypassword123&confirm_password=mypassword123
   ```

### Step 3: Server Processes CGI Request
**File**: `src/cgi/handleCGI.cpp`

**What Happens:**
1. Server recognizes `/cgi` route matches CGI configuration
2. Server identifies `.py` extension, sets interpreter to `/usr/bin/python3`
3. Server prepares CGI environment variables:
   ```cpp
   REQUEST_METHOD=POST
   SCRIPT_NAME=/cgi/register.py
   SCRIPT_FILENAME=www/html/cgi/register.py
   CONTENT_LENGTH=65
   CONTENT_TYPE=application/x-www-form-urlencoded
   SERVER_PROTOCOL=HTTP/1.1
   SERVER_NAME=www.Webserv.com
   SERVER_PORT=8081
   ```
4. Server creates child process and executes Python script
5. Server pipes POST data to script's stdin

### Step 4: Python CGI Script Validates & Saves Account
**File**: `www/html/cgi/register.py`

**What Happens:**
1. Script reads `CONTENT_LENGTH` from environment
2. Script reads POST data from stdin:
   ```python
   content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
   post_data = sys.stdin.read(content_length)
   ```
3. Script parses form data using `parse_qs()`
4. Script validates input:
   - ✅ Username and password not empty
   - ✅ Username at least 3 characters
   - ✅ Password at least 4 characters
   - ✅ Passwords match
   - ✅ Username doesn't already exist
5. Script checks if user exists by reading `/tmp/webserv_accounts.txt`
6. Script saves new account:
   ```python
   # Appends to /tmp/webserv_accounts.txt
   john:mypassword123
   ```
7. Script generates HTML response:
   ```python
   print("Status: 200 OK")
   print("Content-Type: text/html")
   print("\r\n\r\n")
   print("<html>...Registration Successful...</html>")
   ```

### Step 5: Server Returns Response to Browser
**What Happens:**
1. Server reads CGI script output
2. Server sends HTTP response to browser:
   ```http
   HTTP/1.1 200 OK
   Content-Type: text/html
   
   <!DOCTYPE html>...Registration Successful...
   ```
3. Browser displays success page with link to login

**✅ REGISTRATION COMPLETE - Account stored in `/tmp/webserv_accounts.txt`**

---

## 🔐 Login & Authentication Flow

### Step 1: User Opens Login Page
**URL**: `http://127.2.15.1:8081/login.html`

**What Happens:**
1. Browser sends GET request
2. Server routes to `www/html/view/login.html`
3. Browser displays login form

### Step 2: User Submits Login Credentials
**Form Action**: `POST /cgi/login.py`

**What Happens:**
1. Browser collects form data:
   ```
   username=john
   password=mypassword123
   ```
2. Browser sends POST request with credentials

### Step 3: Server Executes Login CGI Script
**File**: `www/html/cgi/login.py`

**What Happens:**
1. Script reads POST data from stdin
2. Script parses username and password
3. Script verifies credentials against `/tmp/webserv_accounts.txt`:
   ```python
   def verify_credentials(username, password):
       with open(ACCOUNTS_FILE, 'r') as f:
           for line in f:
               parts = line.strip().split(':')
               if parts[0] == username and parts[1] == password:
                   return True
       return False
   ```

### Step 4: Generate Session ID
**What Happens:**
1. If credentials valid, script generates random 32-character session ID:
   ```python
   def generate_session_id():
       return ''.join(random.choices(string.ascii_letters + string.digits, k=32))
   ```
   Example: `a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L`

### Step 5: Save Session to Storage
**File**: `/tmp/webserv_sessions.txt`

**What Happens:**
1. Script saves session with format: `session_id:username:reload_count`
   ```python
   def save_session(session_id, username):
       reload_count = get_user_reload_count(username)
       with open(SESSIONS_FILE, 'a') as f:
           f.write(f"{session_id}:{username}:{reload_count}\n")
   ```
   Example entry:
   ```
   a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L:john:0
   ```

### Step 6: Send Cookies to Browser
**What Happens:**
1. Script sends HTTP response with Set-Cookie headers:
   ```python
   print("Content-Type: text/html")
   print(f"Set-Cookie: session_id={session_id}; Max-Age=3600; Path=/")
   print(f"Set-Cookie: username={username}; Max-Age=3600; Path=/")
   print("\r\n\r\n")
   ```
2. Server sends response:
   ```http
   HTTP/1.1 200 OK
   Content-Type: text/html
   Set-Cookie: session_id=a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L; Max-Age=3600; Path=/
   Set-Cookie: username=john; Max-Age=3600; Path=/
   
   <!DOCTYPE html>...Login Successful...
   ```

**✅ LOGIN COMPLETE - Session created and cookies sent to browser**

---

## 🍪 Cookie & Session Management

### How Cookies Work

**1. Server Sets Cookies:**
```http
Set-Cookie: session_id=a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L; Max-Age=3600; Path=/
```
- `session_id`: Cookie name
- `a7Bk9m...`: Cookie value (the session ID)
- `Max-Age=3600`: Cookie expires in 1 hour (3600 seconds)
- `Path=/`: Cookie sent for all paths on this domain

**2. Browser Stores Cookies:**
Browser saves cookies in memory/disk associated with domain `127.2.15.1:8081`

**3. Browser Sends Cookies Automatically:**
For every subsequent request to the same domain, browser includes:
```http
GET /profile.html HTTP/1.1
Host: 127.2.15.1:8081
Cookie: session_id=a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L; username=john
```

### Session Storage Format

**File**: `/tmp/webserv_sessions.txt`
```
session_id:username:reload_count
a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L:john:0
b8Cm0nOqR3sYa6wZu9xVd4tEfG5hIk7M:alice:0
```

### Server-Side Session Validation

When CGI scripts need to validate sessions:
1. Read `HTTP_COOKIE` environment variable
2. Parse cookies to extract `session_id`
3. Check if `session_id` exists in `/tmp/webserv_sessions.txt`
4. Grant or deny access based on validation

**Example in CGI:**
```python
def parse_cookies():
    cookies = {}
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    for cookie in cookie_header.split(';'):
        name, value = cookie.split('=', 1)
        cookies[name.strip()] = value.strip()
    return cookies

cookies = parse_cookies()
session_id = cookies.get('session_id', '')
# Validate session_id against SESSIONS_FILE
```

---

## 👤 Profile Display Flow

### Step 1: User Navigates to Profile Page
**URL**: `http://127.2.15.1:8081/profile.html`

**What Happens:**
1. Browser sends GET request with cookies:
   ```http
   GET /profile.html HTTP/1.1
   Host: 127.2.15.1:8081
   Cookie: session_id=a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L; username=john
   ```
2. Server routes to `www/html/view/profile.html`
3. Server reads and serves static HTML file
4. Browser receives HTML page

### Step 2: HTML Page Loads
**File**: `www/html/view/profile.html`

**What Happens:**
1. Browser parses HTML
2. Browser displays "Loading profile..." message
3. JavaScript `DOMContentLoaded` event fires
4. JavaScript calls `loadProfile()` function

### Step 3: JavaScript Checks Authentication
**Client-Side JavaScript:**
```javascript
function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return parts.pop().split(';').shift();
    return null;
}

function loadProfile() {
    const sessionId = getCookie('session_id');
    const username = getCookie('username');
    
    if (sessionId && username) {
        // ✅ User is authenticated
        document.getElementById('username-display').textContent = username;
        document.getElementById('profile-content').style.display = 'block';
    } else {
        // ❌ User is not authenticated
        document.getElementById('not-authenticated').style.display = 'block';
    }
}
```

**What Happens:**
1. JavaScript reads cookies from `document.cookie`
   - `document.cookie` returns: `"session_id=a7Bk9m...; username=john"`
2. JavaScript extracts `session_id` and `username` values
3. JavaScript validates both cookies exist

### Step 4: Display Profile Information
**If Authenticated (cookies exist):**
1. JavaScript updates DOM:
   ```javascript
   document.getElementById('username-display').textContent = "john";
   ```
2. Browser displays:
   - Welcome message
   - **Username**: "john"
   - **Profile image**: `/cgi/piedPipper.jpg`
   - Logout button

**If Not Authenticated (no cookies):**
1. JavaScript displays "Not Authenticated" message
2. Shows "Go to Login" button

### Step 5: Image Display
**What Happens:**
1. HTML contains: `<img src="/cgi/piedPipper.jpg" alt="Profile Image">`
2. Browser sends GET request for image:
   ```http
   GET /cgi/piedPipper.jpg HTTP/1.1
   Host: 127.2.15.1:8081
   Cookie: session_id=a7Bk9m...; username=john
   ```
3. Server routes to `www/html/cgi/piedPipper.jpg`
4. Server sends image file with `Content-Type: image/jpeg`
5. Browser displays image on profile page

**✅ PROFILE DISPLAYED - Username shown, image loaded, authenticated session verified**

---

## 🚪 Logout Flow

### Step 1: User Clicks Logout Button
**Link**: `<a href="/cgi/logout.py">Logout</a>`

**What Happens:**
1. Browser sends GET request with cookies:
   ```http
   GET /cgi/logout.py HTTP/1.1
   Host: 127.2.15.1:8081
   Cookie: session_id=a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L; username=john
   ```
2. Server recognizes CGI request
3. Server sets environment variable: `HTTP_COOKIE=session_id=a7Bk9m...; username=john`
4. Server executes `/usr/bin/python3 www/html/cgi/logout.py`

### Step 2: Python Script Processes Logout
**File**: `www/html/cgi/logout.py`

**What Happens:**
1. Script parses cookies from environment:
   ```python
   def parse_cookies():
       cookies = {}
       cookie_header = os.environ.get('HTTP_COOKIE', '')
       for cookie in cookie_header.split(';'):
           name, value = cookie.split('=', 1)
           cookies[name.strip()] = value.strip()
       return cookies
   
   cookies = parse_cookies()
   session_id = cookies.get('session_id', '')
   ```

2. Script removes session from storage:
   ```python
   def remove_session(session_id):
       with open(SESSIONS_FILE, 'r') as f:
           lines = f.readlines()
       
       with open(SESSIONS_FILE, 'w') as f:
           for line in lines:
               if not line.startswith(session_id + ':'):
                   f.write(line)  # Keep all lines except matching session
   ```

3. Session deleted from `/tmp/webserv_sessions.txt`

### Step 3: Delete Client Cookies
**What Happens:**
1. Script sends response with expired cookies:
   ```python
   print("Status: 200 OK")
   print("Content-Type: text/html")
   print("Set-Cookie: session_id=; Max-Age=0; Path=/")
   print("Set-Cookie: username=; Max-Age=0; Path=/")
   print("\r\n\r\n")
   ```

2. Server sends to browser:
   ```http
   HTTP/1.1 200 OK
   Content-Type: text/html
   Set-Cookie: session_id=; Max-Age=0; Path=/
   Set-Cookie: username=; Max-Age=0; Path=/
   
   <!DOCTYPE html>...Logged Out Successfully...
   ```

3. Browser receives `Max-Age=0` which means "delete immediately"
4. Browser deletes `session_id` and `username` cookies
5. Browser displays logout success page

**✅ LOGOUT COMPLETE - Session deleted server-side, cookies deleted client-side**

---

## 🔧 Technical Implementation Details

### CGI Execution Process

**C++ Server Side** (`src/cgi/handleCGI.cpp`):

1. **Environment Setup:**
   ```cpp
   void Cgi::SetEnv() {
       env_strings.push_back("REQUEST_METHOD=" + getMethod());
       env_strings.push_back("SCRIPT_FILENAME=" + getScriptFileName());
       env_strings.push_back("CONTENT_LENGTH=" + getHeader("Content-length"));
       env_strings.push_back("HTTP_COOKIE=" + getHeader("cookie"));
       // ... more environment variables
   }
   ```

2. **Interpreter Selection:**
   ```cpp
   if (fExten == ".py")
       interpreter = "/usr/bin/python3";
   else if (fExten == ".php")
       interpreter = "/usr/bin/php-cgi";
   else if (fExten == ".sh")
       interpreter = "/bin/bash";
   ```

3. **Process Creation:**
   - Fork child process
   - Setup pipes for stdin/stdout
   - Execute interpreter with script
   - Pass environment variables
   - Pipe POST data to stdin
   - Read output from stdout

### Configuration Details

**Server Configuration** (`webserv.conf`):
```properties
SERVER = [
    port = 8081;
    host = 127.2.15.1;
    allowed_methods = GET, POST, DELETE;
    max_body_size = 111111;
    server_names = www.Webserv.com, www.Webserv.ma;
    
    route = / : ROOT=www/html/view, DEFAULT_FILE=index.html;
    route = /cgi : ROOT=www/html/cgi, CGI_EXTENTION=.php-.sh-.py;
]
```

### Data Storage

**File Locations:**
- **Accounts**: `/tmp/webserv_accounts.txt`
  ```
  Format: username:password
  Example: john:mypassword123
  ```

- **Sessions**: `/tmp/webserv_sessions.txt`
  ```
  Format: session_id:username:reload_count
  Example: a7Bk9mNpQ2rXz5vYt8wUc3sDeF4gHj6L:john:0
  ```

- **User Data**: `/tmp/webserv_users.txt`
  ```
  Format: username:reload_count
  Example: john:5
  ```

### Security Considerations

⚠️ **Note**: This is a demonstration implementation. Production systems should:
- Hash passwords (bcrypt, argon2)
- Use secure session storage (database, Redis)
- Implement HTTPS/TLS
- Add CSRF protection
- Sanitize all user inputs
- Implement rate limiting
- Use secure random for session IDs
- Add session expiration validation server-side

### Complete Request-Response Flow Diagram

```
┌─────────┐                ┌─────────┐               ┌───────────┐
│ Browser │                │  Server │               │ CGI Script│
└────┬────┘                └────┬────┘               └─────┬─────┘
     │                          │                          │
     │ 1. POST /cgi/login.py    │                          │
     │─────────────────────────>│                          │
     │    + Form Data           │                          │
     │    + Cookies (if any)    │                          │
     │                          │                          │
     │                          │ 2. Parse Request         │
     │                          │    Match Route           │
     │                          │    Identify CGI          │
     │                          │                          │
     │                          │ 3. Setup Environment     │
     │                          │    Fork Process          │
     │                          │─────────────────────────>│
     │                          │    Pass ENV + POST data  │
     │                          │                          │
     │                          │                          │ 4. Process Request
     │                          │                          │    Validate Creds
     │                          │                          │    Generate Session
     │                          │                          │    Save to File
     │                          │                          │
     │                          │ 5. Return HTML + Headers │
     │                          │<─────────────────────────│
     │                          │    Set-Cookie headers    │
     │                          │                          │
     │ 6. HTTP Response         │                          │
     │<─────────────────────────│                          │
     │    Set-Cookie: session_id│                          │
     │    Set-Cookie: username  │                          │
     │    HTML Content          │                          │
     │                          │                          │
     │ 7. Store Cookies         │                          │
     │    Display Page          │                          │
     │                          │                          │
     │ 8. GET /profile.html     │                          │
     │─────────────────────────>│                          │
     │    Cookie: session_id    │                          │
     │    Cookie: username      │                          │
     │                          │                          │
     │                          │ 9. Serve Static File     │
     │                          │    (profile.html)        │
     │                          │                          │
     │ 10. HTML Page            │                          │
     │<─────────────────────────│                          │
     │                          │                          │
     │ 11. JavaScript Executes  │                          │
     │     Read Cookies         │                          │
     │     Display Username     │                          │
     │                          │                          │
     │ 12. GET /cgi/image.jpg   │                          │
     │─────────────────────────>│                          │
     │    Cookie: session_id    │                          │
     │                          │                          │
     │ 13. Image File           │                          │
     │<─────────────────────────│                          │
     │                          │                          │
     └──────────────────────────┴──────────────────────────┘
```

---

## 🚀 Running the Application

### Build and Start Server:
```bash
make re
make clean
./webserv webserv.conf
```

### Access Application:
- **Homepage**: http://127.2.15.1:8081/
- **Register**: http://127.2.15.1:8081/register.html
- **Login**: http://127.2.15.1:8081/login.html
- **Profile**: http://127.2.15.1:8081/profile.html

### Test Flow:
1. Open register page → Create account
2. Login with credentials → Receive cookies
3. Navigate to profile → See username and image
4. Click logout → Cookies deleted, session ended

---

## 📊 Summary

This web application demonstrates a complete authentication system with:

✅ **User Registration**: Form validation, duplicate checking, file storage  
✅ **User Login**: Credential verification, session generation  
✅ **Session Management**: Server-side storage, secure session IDs  
✅ **Cookie Management**: Automatic sending, client-side reading, expiration  
✅ **Protected Pages**: Client-side authentication check using cookies  
✅ **User Profile Display**: Dynamic username display, profile image  
✅ **Logout**: Session termination, cookie deletion

The system uses CGI for dynamic content, cookies for client state, and file-based storage for persistence, all orchestrated by a custom C++ HTTP server.
