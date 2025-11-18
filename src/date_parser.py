import sys
import json
import regex as re
import nltk
from dateparser import parse as parse_date


def read_file_content(filepath):
    # Simple file reader to get text from the file path passed by C++
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.read()

if __name__ == "__main__":
    # This block allows C++ to run this script via command line
    if len(sys.argv) < 2:
        print(json.dumps({"error": "No filename provided"}))
        sys.exit(1)
        
    filename = sys.argv[1]
    try:
        # 1. Read the text from the file
        text = read_file_content(filename)
        
        # 2. Parse the text
        events = extract_schedule_events(text)
        
        # 3. Print the result as JSON to stdout (so C++ can read it)
        print(json.dumps(events, default=str)) 
    except Exception as e:
        print(json.dumps({"error": str(e)}))