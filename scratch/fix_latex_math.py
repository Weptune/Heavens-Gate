import os
import re

docs_dir = r"c:\Users\abhin\heavensgate\docs"

def fix_content(content):
    # Convert remaining \text{...} C++ function names to clean markdown code backticks
    content = re.sub(r'\\text\{countr\\\\_zero\}\(B\)', '`countr_zero(B)`', content)
    content = re.sub(r'\\text\{countr_zero\}\(B\)', '`countr_zero(B)`', content)
    content = re.sub(r'\\text\{lsb\}\(B\)', '`lsb(B)`', content)
    content = re.sub(r'\\text\{popcount\}\(B\)', '`popcount(B)`', content)
    content = re.sub(r'\$\\text\{countr\\\\_zero\}\(B\)\$', '`countr_zero(B)`', content)
    content = re.sub(r'\$\\text\{countr_zero\}\(B\)\$', '`countr_zero(B)`', content)
    
    # Fix any double backslashes in math blocks
    content = content.replace(r'\&', r'\&')
    return content

for filename in sorted(os.listdir(docs_dir)):
    if filename.endswith(".md"):
        filepath = os.path.join(docs_dir, filename)
        with open(filepath, 'r', encoding='utf-8') as f:
            c = f.read()
        fixed_c = fix_content(c)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(fixed_c)
