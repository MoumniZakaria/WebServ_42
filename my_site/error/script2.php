#!/usr/bin/php-cgi
<?php
// CGI requires the correct Content-Type header
header("Content-Type: text/plain");

// Print all HTTP headers
echo "=== HTTP HEADERS ===\n";
foreach ($_SERVER as $key => $value) {
    if (strpos($key, 'HTTP_') === 0) {
        echo "$key: $value\n";
    }
}

// Also include other CGI-relevant server variables
echo "\n=== SERVER VARIABLES ===\n";
echo "REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "CONTENT_TYPE: " . $_SERVER['CONTENT_TYPE'] . "\n";
echo "CONTENT_LENGTH: " . $_SERVER['CONTENT_LENGTH'] . "\n";

// Read and display raw POST body
echo "\n=== POST BODY ===\n";
$body = file_get_contents('php://input');
echo $body;
?>
