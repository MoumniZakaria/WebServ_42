<?php
$html_content = <<<HTML
<html>
<head>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f9;
            color: #333;
            text-align: center;
            margin: 10%;
        }
        h1 {
            color: #4CAF50;
            font-size: 36px;
        }
        p {
            font-size: 18px;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <h1>Login Successful</h1>
    <p>Welcome, khalid!</p>
    <p>Your credentials are correct, and a cookie has been set.</p>
</body>
</html>
HTML;

header("HTTP/1.1 200 OK");
header("Content-Type: text/html");
header("Content-Length: " . strlen($html_content));


echo $html_content;
?>

