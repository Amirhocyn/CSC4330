import sys
import json
import os
import regex as re
import nltk
from dateparser import parse as parse_date
from datetime import timedelta

# Regex patterns
DATE_PATTERN = r"(\b(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z.]*\s+\d{1,2}(?:,\s*\d{4})?|\d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4})"
# Matches time ranges like "5-7:30pm"
TIME_RANGE_PATTERN = r"(\d{1,2}(?::\d{2})?)\s*(?:-|–|to)\s*(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)?"
# Matches single times like "5pm"
SINGLE_TIME_PATTERN = r"(?:at|@)?\s*\b(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)"

def merge_date_and_time(base_date_obj, time_str, is_pm_context=False):
    if not base_date_obj or not time_str:
        return base_date_obj
    
    # FIX 1: Clean up the time string
    # If it's just a number like "5", make it "5:00"
    if time_str.isdigit():
        time_str += ":00"
        
    # FIX 2: Apply PM context explicitly to the string BEFORE parsing
    # This ensures "5" becomes "5:00 PM" if the context implies it
    if is_pm_context:
        # Only add PM if it's not already there
        if "pm" not in time_str.lower() and "am" not in time_str.lower():
            time_str += " PM"

    # Parse
    full_str = f"{base_date_obj.strftime('%Y-%m-%d')} {time_str}"
    dt = parse_date(full_str)
    
    return dt if dt else base_date_obj

def extract_schedule_events(text):
    try:
        nltk.data.find('tokenizers/punkt')
        nltk.data.find('tokenizers/punkt_tab')
    except LookupError:
        nltk.download('punkt', quiet=True)
        nltk.download('punkt_tab', quiet=True)

    clean_text = text.replace('\n', ' ').replace('\r', ' ')
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
            start_raw = range_match.group("start")
            end_raw = range_match.group("end")

            start_date = parse_date(start_raw)
            end_date = parse_date(end_raw)

            if start_date:
                # Event title = sentence with dates removed
                cleaned = sent.replace(range_match.group(0), "").strip(" ---,;:.")
                time_match = re.search(TIME_PATTERN, sent)
                time_str = time_match.group(1) if time_match else None

                events.append({
                    "title": cleaned if cleaned else "Event",
                    "start_date": start_date.date().isoformat(),
                    "end_date": end_date.date().isoformat() if end_date else start_date.date().isoformat(),
                    "time": time_str,
                    "raw": sent.strip()
                })
            continue

        # 2) Single date case
        date_match = re.search(SINGLE_DATE_PATTERN, sent, flags=re.IGNORECASE | re.VERBOSE)
        if not date_match:
            continue

        raw_date = date_match.group(1)
        parsed_date = parse_date(raw_date)
        if not parsed_date:
            continue

        time_match = re.search(TIME_PATTERN, sent)
        time_str = time_match.group(1) if time_match else None

        cleaned = sent.replace(raw_date, "").replace(time_str if time_str else "", "").strip(" -–-,;:.")

        events.append({
            "title": cleaned if cleaned else "Event",
            "start_date": parsed_date.date().isoformat(),
            "end_date": parsed_date.date().isoformat(),  # single day event
            "time": time_str,
            "raw": sent.strip()
        })

    return events

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"error": "No filename provided"}))
        sys.exit(1)
    filepath = sys.argv[1]
    if os.path.exists(filepath):
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            print(json.dumps(extract_schedule_events(f.read()), indent=4, default=str))
    else:
        print(json.dumps({"error": "File not found"}))