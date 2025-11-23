import sys
import json
import os
import regex as re
import nltk
from dateparser import parse as parse_date
from paddleocr import PaddleOCR
import logging
import warnings
import cv2  # Add this import for image preprocessing (install with pip install opencv-python if needed)
import numpy as np  # Add this for the sharpen kernel

# --- DEBUG MARKER ---
print("PYTHON: I AM THE CORRECT FILE WITH .ocr()", file=sys.stderr)

warnings.filterwarnings("ignore", category=DeprecationWarning)
logging.getLogger("ppocr").setLevel(logging.ERROR)

DATE_PATTERN = r"(\b(?:(?:mon|tue|wed|thu|fri|sat|sun)[a-z]*\.?,?\s*)?(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z.]*[-.–—\s]*\d{1,2}(?:st|nd|rd|th)?(?:,?\s*'?\d{2,4})?)"
TIME_RANGE_PATTERN = r"(\d{1,2}(?::\d{2})?)\s*(?:-|–|to)\s*(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)?"
SINGLE_TIME_PATTERN = r"(?:at|@)?\s*\b(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)"

try:
    ocr = PaddleOCR(use_textline_orientation=True, lang='en',
                    det_db_thresh=0.3,
                    det_db_box_thresh=0.5,
                    det_db_unclip_ratio=2.0,
                    use_dilation=True,
                    det_db_score_mode='slow')
except:
    ocr = PaddleOCR(lang='en')

def merge_date_and_time(base_date_obj, time_str, is_pm_context=False):
    if not base_date_obj or not time_str: return base_date_obj
    if time_str.isdigit(): time_str += ":00"
    if is_pm_context and "pm" not in time_str.lower() and "am" not in time_str.lower():
        time_str += " PM"
    time_str = time_str.replace("at", "").replace("@", "").strip()
    full_str = f"{base_date_obj.strftime('%Y-%m-%d')} {time_str}"
    dt = parse_date(full_str)
    return dt if dt else base_date_obj

def extract_schedule_events(filepath):
    print(f"PYTHON: Processing {filepath}...", file=sys.stderr)
    try:
        nltk.data.find('tokenizers/punkt')
        nltk.data.find('tokenizers/punkt_tab')
    except LookupError:
        nltk.download('punkt', quiet=True)
        nltk.download('punkt_tab', quiet=True)

    # Preprocess the image to improve OCR accuracy
    temp_path = None
    try:
        img = cv2.imread(filepath)
        if img is not None:
            # Convert to grayscale and enhance contrast
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            enhanced = cv2.equalizeHist(gray)
            # Sharpen the image
            sharpen_kernel = np.array([[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]])
            sharpened = cv2.filter2D(enhanced, -1, sharpen_kernel)
            # Optional: binary threshold for cleaner text
            # _, binary = cv2.threshold(sharpened, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
            temp_path = os.path.join(os.getcwd(), 'temp_preprocessed.png')
            cv2.imwrite(temp_path, sharpened)
            filepath = temp_path  # Use preprocessed image for OCR
    except Exception as e:
        print(f"PYTHON WARNING: Preprocessing failed - {str(e)}", file=sys.stderr)

    # OCR with tuned parameters (moved to init)
    text_content = ""
    try:
        result = ocr.predict(filepath)  # Use predict() without 'cls' or other runtime args
        print(f"PYTHON DEBUG: Full OCR result: {result}", file=sys.stderr)  # For debugging
        if result:
            for res in result:  # Handle list of dicts
                for text, conf in zip(res.get('rec_texts', []), res.get('rec_scores', [])):
                    if conf > 0.5:
                        text_content += text + "\n"
    except Exception as e:
        print(f"PYTHON ERROR: OCR Failed - {str(e)}", file=sys.stderr)

    # Clean up temp file
    if temp_path and os.path.exists(temp_path):
        os.remove(temp_path)

    # Save Raw
    output_path = os.path.join(os.getcwd(), "last_scan_raw.txt")
    try:
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(text_content if text_content else "No text found.")
    except: pass

    # Parse Dates
    clean_text = text_content.replace('\r', ' ')
    sentences = nltk.sent_tokenize(clean_text)
    events = []
    last_seen_date = None

    for sent in sentences:
        date_match = re.search(DATE_PATTERN, sent, flags=re.IGNORECASE)
        base_date = None
        if date_match:
            base_date = parse_date(date_match.group(1))
            if base_date: last_seen_date = base_date
        if not base_date: base_date = last_seen_date
        if not base_date: continue

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
    if os.path.exists(filepath):
        results = extract_schedule_events(filepath)
        print(json.dumps(results, indent=4, default=str))
    else:
        print(json.dumps({"error": "File not found"}))