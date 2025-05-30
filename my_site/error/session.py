#!/usr/bin/env python3
import sys
import urllib.parse


post_data = sys.stdin.read()

params = urllib.parse.parse_qs(post_data)
cookie_data = params.get('cookies', [''])[0]


session_key = ''
for pair in cookie_data.split(';'):
    if 'session_key=' in pair:
        session_key = pair.strip().split('=', 1)[1]
        break


key_found = False
username = 'Guest'

try:
    with open('db_cgi.txt', 'r') as file:
        for line in file:
            if f'key {session_key}' in line:
                key_found = True
                username = line.split()[1]
                break
except FileNotFoundError:
    pass  


if key_found:
    html_content = f"""\
<html>
<head>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f9;
            color: #333;
            text-align: center;
        }}
        h1 {{
            color: #4CAF50;
            font-size: 36px;
        }}
        p {{
            font-size: 18px;
            margin: 10px 0;
        }}
    </style>
</head>
<body>
    <h1>Key Found from session script</h1>
    <p>Welcome, {username}!</p>
    <p>Your key was found and is valid.</p>
</body>
</html>
"""

    print("HTTP/1.1 200 ok")
    print("Content-Type: text/html")
    print(f"Content-Length: {len(html_content.encode('utf-8'))}")
    print()
    print(html_content)

else:
    print("HTTP/1.1 403 forbiden")
    print("Content-Type: text/html")
    print("Content-Length: 0")
    print()
