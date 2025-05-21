#!/usr/bin/env python3

import sys


post_data = sys.stdin.read()

params = dict(p.split('=') for p in post_data.split('&'))
key = params.get('key', '')


key_found = False
with open('db_cgi.txt', 'r') as file:
    for line in file:
        if f'key {key}' in line:
            key_found = True
            username = line.split()[1]
            break

if key_found:
    print("Content-Type: text/html\n")
    print(f"""
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
        <h1>Key Found</h1>
        <p>Welcome, {username}!</p>
        <p>Your key was found and is valid.</p>
    </body>
    </html>
    """)
else:
    print("Content-Type: text/html\n")
    print(f"""
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
        <h1>Key Found</h1>
        <p>Welcome, walo!</p>
        <p>Your key was found and is valid.</p>
    </body>
    </html>
    """)
