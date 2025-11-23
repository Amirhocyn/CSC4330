import sys
import json
import os
import regex as re
import nltk
from dateparser import parse as parse_date
from datetime import timedelta
from google.cloud import vision
import io

# Set Google Application Credentials
os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "credentials.json"

# --- REGEX PATTERNS ---
DATE_PATTERN = r"(\b(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z.]*\s+\d{1,2}(?:,\s*\d{4})?|\d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4})"
TIME_RANGE_PATTERN = r"(\d{1,2}(?::\d{2})?)\s*(?:-|–|to)\s*(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)?"
SINGLE_TIME_PATTERN = r"(?:at|@)?\s*\b(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)"

def merge_date_and_time(base_date_obj, time_str, is_pm_context=False):
    if not base_date_obj or not time_str: return base_date_obj
    if time_str.isdigit(): time_str += ":00"
    if is_pm_context and "pm" not in time_str.lower() and "am" not in time_str.lower():
        time_str += " PM"
    
    # Simple clean to remove "at" if present
    time_str = time_str.replace("at", "").replace("@", "").strip()
    
    full_str = f"{base_date_obj.strftime('%Y-%m-%d')} {time_str}"
    dt = parse_date(full_str)
    return dt if dt else base_date_obj

def get_text_from_google(path):
    """Sends image to Google Cloud and returns the text string."""
    client = vision.ImageAnnotatorClient()
    
    with io.open(path, 'rb') as image_file:
        content = image_file.read()
        
    image = vision.Image(content=content)
    response = client.text_detection(image=image)
    
    if response.error.message:
        raise Exception(f"Google API Error: {response.error.message}")

    if response.text_annotations:
        # The first annotation is the full text block
        return response.text_annotations[0].description
    return ""

def extract_schedule_events(filepath):
    # 1. DETERMINE INPUT TYPE
    # If it's an image, use Google. If it's text, read it directly.
    ext = os.path.splitext(filepath)[1].lower()
    if ext in ['.jpg', '.jpeg', '.png', '.bmp']:
        text_content = get_text_from_google(filepath)
    else:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            text_content = f.read()

    # 2. RUN PARSING LOGIC (Same as before)
    try:
        nltk.data.find('tokenizers/punkt')
        nltk.data.find('tokenizers/punkt_tab')
    except LookupError:
        nltk.download('punkt', quiet=True)
        nltk.download('punkt_tab', quiet=True)

    clean_text = text_content.replace('\n', ' ').replace('\r', ' ')
    sentences = nltk.sent_tokenize(clean_text)
    events = []
    last_seen_date = None

    for sent in sentences:
        # A. Date Finding
        date_match = re.search(DATE_PATTERN, sent, flags=re.IGNORECASE)
        base_date = None
        if date_match:
            base_date = parse_date(date_match.group(1))
            if base_date: last_seen_date = base_date
        if not base_date: base_date = last_seen_date
        if not base_date: continue

        # B. Time Finding
        range_match = re.search(TIME_RANGE_PATTERN, sent, flags=re.IGNORECASE)
        start_dt = base_date
        end_dt = None
        found_time = False
        
        if range_match:
            found_time = True
            t_start = range_match.group(1)
            t_end = range_match.group(2)
            meridiem = range_match.group(3) if range_match.group(3) else ""
            is_pm = meridiem.lower() == 'pm'
            end_dt = merge_date_and_time(base_date, f"{t_end}{meridiem}")
            start_dt = merge_date_and_time(base_date, t_start, is_pm_context=is_pm)
        else:
            time_match = re.search(SINGLE_TIME_PATTERN, sent, flags=re.IGNORECASE)
            if time_match:
                found_time = True
                t_raw = time_match.group(1) + (time_match.group(2) if time_match.group(2) else "")
                start_dt = merge_date_and_time(base_date, t_raw)
        
        if found_time or (date_match and not found_time):
             events.append({
                "title": "Scanned Event",
                "start_date_pretty": start_dt.strftime("%Y-%m-%d %H:%M:%S"),
                "end_date_pretty": end_dt.strftime("%Y-%m-%d %H:%M:%S") if end_dt else "",
                "raw": sent.strip()
            })

    return events

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"error": "No filename provided"}))
        sys.exit(1)
    
    filepath = sys.argv[1]
    
    try:
        # Check if file exists
        if not os.path.exists(filepath):
             print(json.dumps({"error": "File not found"}))
             sys.exit(1)
             
        # CALL THE MAIN FUNCTION
        results = extract_schedule_events(filepath)
        print(json.dumps(results, indent=4, default=str))

    except Exception as e:
        print(json.dumps({"error": str(e)}))