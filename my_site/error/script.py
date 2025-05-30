#!/usr/bin/env python3

import sys
import urllib.parse
import secrets
import string

post_data = sys.stdin.read()

params = urllib.parse.parse_qs(post_data)

db_user = "admin"
db_pass = "admin"

username = params.get('username', [''])[0]
password = params.get('password', [''])[0]


def generate_random_key(length=50):
    alphabet = string.ascii_letters + string.digits 
    return ''.join(secrets.choice(alphabet) for _ in range(length))

if username == db_user and password == db_pass:
    key = generate_random_key(50)
    with open('db_cgi.txt', 'a') as file:
        file.write(f"user {username} key {key}\n")

    html_content = f"""\
    <html>
    <head>
        <style>
            body {{
                font-family: Arial, sans-serif;
                background-color: #f4f4f9;
                color: #333;
                text-align: center;
                margin:10%;
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
        <h1>Login Successful</h1>
        <p>Welcome, {username}!</p>
        <p>Your credentials are correct, and a cookie has been set.</p>
    </body>
    </html>
    """
    
    print("HTTP/1.1 200 ok")
    print("Content-Type: text/html")
    print(f"Content-Length: {len(html_content.encode('utf-8'))}")
    print(f"Set-Cookie: session_key={key}; Path=/;")
    print()
    print(html_content)
else:
    html_content = f"""\
    <html>
    <head>
        <style>
            body {{
                font-family: Arial, sans-serif;
                background-color: #f4f4f9;
                color: #333;
                text-align: center;
                margin:10%;
            }}
            h1 {{
                color: red;
                font-size: 36px;
            }}
        </style>
    </head>
    <body>
        <h1>Invalid credentials.</h1>
    </body>
    </html>
    """

    print("HTTP/1.1 200 ok")
    print("Content-Type: text/html")
    print(f"Content-Length: {len(html_content.encode('utf-8'))}")
    print()
    print(html_content)