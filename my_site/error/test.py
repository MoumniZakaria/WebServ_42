import sys
import secrets



html_content = f"""\
<html>
<head>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f9;
            color: #333;
            text-align: center;
            margin: 10%;
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
    <p>Welcome, khalid!</p>
    <p>Your credentials are correct, and a cookie has been set.</p>
</body>
</html>
"""

print("HTTP/1.1 200 OK")
print("Content-Type: text/html")
print(f"Content-Length: {len(html_content.encode('utf-8'))}")
print()  


print(html_content)
