<?php

$valid_username = "admin";
$valid_password = "password123";


$stdin_data = file_get_contents("php://stdin");

// Parse the data
$parsed_data = [];
parse_str($stdin_data, $parsed_data);


$username = isset($parsed_data['username']) ? $parsed_data['username'] : '';
$password = isset($parsed_data['password']) ? $parsed_data['password'] : '';


echo "Content-Type: text/html\n\n";

if ($username === $valid_username && $password === $valid_password) {


    $file = fopen('./db-cgi.txt', 'a');
    if ($file) {
        fwrite($file, "Hello, this is a new file!");
        fclose($file);
    } else {
        echo "<p>Failed to create the file.</p>";
    }
    
    echo "<h3>Welcome, " . htmlspecialchars($username) . "!</h3>";
    echo "<p>Username: '" . htmlspecialchars($username) . "'</p>";
    echo "<p>Password: '" . htmlspecialchars($password) . "'</p>";
} else {
    echo "<p>Invalid login. Username: '" . htmlspecialchars($username) . 
         "', Password: '" . htmlspecialchars($password) . "'</p>";
}
