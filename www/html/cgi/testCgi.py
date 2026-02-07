#!/usr/bin/env python3
"""
Simple script that generates HTML code as output with HTTP headers
Shows a colorful data visualization of programming languages
"""

# Sample data
languages = [
    {"name": "Python", "popularity": 85, "color": "#3776ab"},
    {"name": "JavaScript", "popularity": 78, "color": "#f7df1e"},
    {"name": "Java", "popularity": 65, "color": "#007396"},
    {"name": "C++", "popularity": 58, "color": "#00599c"},
    {"name": "Go", "popularity": 45, "color": "#00add8"}
]

# Print HTTP headers
print("Content-Type: text/html")
print("Status: 200 OK")
print("Cache-Control: no-cache")
print("Server: Python/3.x")
print("\r\n\r\n")  # Empty line to separate headers from body

# Build the bars HTML
bars_html = ""
for lang in languages:
    bars_html += f"""
        <div class="bar">
            <div class="bar-label">{lang['name']}</div>
            <div class="bar-graph" style="width: {lang['popularity']}%; background-color: {lang['color']};">
                {lang['popularity']}%
            </div>
        </div>"""

# Print HTML content in one print statement
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Programming Languages Popularity</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #f5f5f5;
        }}
        h1 {{
            color: #333;
            text-align: center;
        }}
        .bar-container {{
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .bar {{
            margin: 15px 0;
        }}
        .bar-label {{
            font-weight: bold;
            margin-bottom: 5px;
        }}
        .bar-graph {{
            height: 30px;
            border-radius: 5px;
            display: flex;
            align-items: center;
            padding-left: 10px;
            color: white;
            font-weight: bold;
            transition: all 0.3s ease;
        }}
        .bar-graph:hover {{
            opacity: 0.8;
            transform: scale(1.02);
        }}
    </style>
</head>
<body>
    <h1>Programming Languages Popularity</h1>
    <div class="bar-container">{bars_html}
    </div>
</body>
</html>""")
