#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");

echo "<h1>PHP POST Result</h1>";
echo "<h2>POST Data:</h2>";

if (!empty($_POST)) {
    echo "<ul>";
    foreach ($_POST as $key => $value) {
        echo "<li><b>$key:</b> $value</li>";
    }
    echo "</ul>";
} else {
    echo "<p style='color:red'>No POST data received!</p>";
}

echo "<h2>Environment:</h2>";
echo "<p>REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>CONTENT_LENGTH: " . ($_SERVER['CONTENT_LENGTH'] ?? '0') . "</p>";
echo "<p>CONTENT_TYPE: " . ($_SERVER['CONTENT_TYPE'] ?? 'N/A') . "</p>";

echo "<h2>Raw Input:</h2>";
$raw = file_get_contents('php://input');
echo "<pre>" . htmlspecialchars($raw) . "</pre>";

echo "<br><a href='/test-post.html'>Back</a>";
?>
