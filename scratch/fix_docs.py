import os
import re

docs_dir = r"c:\Users\abhin\heavensgate\docs"

def fix_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Remove YouTube Episode Concept lines
    content = re.sub(r'> \*\*YouTube Episode Concept\*\*:[^\n]*\n+', '', content)

    # 2. Remove Visualizations for YouTube and YouTube Narrative Script sections
    content = re.sub(r'## \d*\.?\s*Visualizations for YouTube[\s\S]*?(?=##|\Z)', '', content)
    content = re.sub(r'## \d*\.?\s*YouTube Narrative Script Concept[\s\S]*?(?=##|\Z)', '', content)

    # 3. Fix LaTeX bracket notation: \[ ... \] to $$ ... $$
    content = re.sub(r'\\\[\s*', '\n$$\n', content)
    content = re.sub(r'\s*\\\]', '\n$$\n', content)

    # 4. Fix inline LaTeX parentheses: \( ... \) to $ ... $
    content = re.sub(r'\\\(\s*', '$', content)
    content = re.sub(r'\s*\\\)', '$', content)

    # 5. Clean up any trailing double newlines before headers
    content = re.sub(r'\n{3,}', '\n\n', content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content.strip() + '\n')
    print(f"Fixed {os.path.basename(filepath)}")

for filename in sorted(os.listdir(docs_dir)):
    if filename.endswith(".md"):
        fix_file(os.path.join(docs_dir, filename))

print("All docs files fixed successfully.")
