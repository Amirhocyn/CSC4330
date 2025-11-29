import sys
import json
import os
import regex as re
import nltk
from dateparser import parse as parse_date
from paddleocr import PaddleOCR
import logging
import warnings
import cv2  
import numpy as np  

# --- IMPORTS FOR DOCUMENTS ---
try:
    import docx
except ImportError:
    docx = None

try:
    from pypdf import PdfReader
except ImportError:
    PdfReader = None

print("PYTHON: FINAL VERSION (Images + Docx + PDF)...", file=sys.stderr)

warnings.filterwarnings("ignore", category=DeprecationWarning)
logging.getLogger("ppocr").setLevel(logging.ERROR)

# --- REGEX PATTERNS ---
DATE_PATTERN = r"(\b(?:(?:mon|tue|wed|thu|fri|sat|sun)[a-z]*\.?,?\s*)?(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z.]*[-.–—\s]*\d{1,2}(?:st|nd|rd|th)?(?:,?\s*'?\d{2,4})?)"
DATE_RANGE_PATTERN = f"({DATE_PATTERN})\s*(?:-|–|to|through)\s*({DATE_PATTERN})"
TIME_RANGE_PATTERN = r"(\d{1,2}(?::\d{2})?)\s*(?:-|–|to)\s*(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)?"
SINGLE_TIME_PATTERN = r"(?:at|@)?\s*\b(\d{1,2}(?::\d{2})?)\s*(am|pm|AM|PM)"

try:
    ocr = PaddleOCR(use_textline_orientation=True, lang='en',
                    det_db_thresh=0.3, det_db_box_thresh=0.5,
                    det_db_unclip_ratio=2.0, use_dilation=True,
                    det_db_score_mode='slow')
except:
    ocr = PaddleOCR(lang='en')

# --- HELPERS ---
def clean_ocr_date(date_str):
    if not date_str: return ""
    clean = re.sub(r'[-.–—]', ' ', date_str)
    clean = re.sub(r'([a-zA-Z])(\d)', r'\1 \2', clean)
    return clean

def merge_date_and_time(base_date_obj, time_str, is_pm_context=False):
    if not base_date_obj or not time_str: return base_date_obj
    if time_str.isdigit(): time_str += ":00"
    if is_pm_context and "pm" not in time_str.lower() and "am" not in time_str.lower():
        time_str += " PM"
    time_str = time_str.replace("at", "").replace("@", "").strip()
    full_str = f"{base_date_obj.strftime('%Y-%m-%d')} {time_str}"
    dt = parse_date(full_str, settings={'PREFER_DATES_FROM': 'future'})
    return dt if dt else base_date_obj

def get_text_from_file(filepath):
    ext = os.path.splitext(filepath)[1].lower()
    text_content = ""

    # 1. PDF FILES
    if ext == ".pdf":
        if PdfReader is None: return "Error: pypdf not installed."
        try:
            reader = PdfReader(filepath)
            for page in reader.pages:
                text_content += page.extract_text() + "\n"
        except Exception as e:
            return f"Error reading PDF: {e}"

    # 2. DOCX FILES
    elif ext == ".docx":
        if docx is None: return "Error: python-docx not installed."
        try:
            doc = docx.Document(filepath)
            return "\n".join([para.text for para in doc.paragraphs])
        except Exception as e:
            return f"Error reading docx: {e}"

    # 3. TEXT FILES
    elif ext == ".txt":
        try:
            with open(filepath, 'r', encoding='utf-8') as f: return f.read()
        except Exception as e: return f"Error reading txt: {e}"

    # 4. IMAGES (PaddleOCR)
    else:
        temp_path = None
        try:
            img = cv2.imread(filepath)
            if img is not None:
                gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
                enhanced = cv2.equalizeHist(gray)
                sharpen_kernel = np.array([[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]])
                sharpened = cv2.filter2D(enhanced, -1, sharpen_kernel)
                temp_path = os.path.join(os.getcwd(), 'temp_preprocessed.png')
                cv2.imwrite(temp_path, sharpened)
                filepath = temp_path
        except: pass

        try:
            result = ocr.predict(filepath)
            if result:
                for res in result:
                    if hasattr(res, 'get'): # Dict format
                        for text, conf in zip(res.get('rec_texts', []), res.get('rec_scores', [])):
                            if conf > 0.5: text_content += text + "\n"
                    elif isinstance(res, list): # List format
                         for line in res:
                             if len(line) > 1: text_content += line[1][0] + "\n"
        except Exception as e:
            print(f"PYTHON ERROR: OCR Failed - {str(e)}", file=sys.stderr)

        if temp_path and os.path.exists(temp_path): os.remove(temp_path)
        
    return text_content

def extract_schedule_events(filepath):
    print(f"PYTHON: Processing {filepath}...", file=sys.stderr)
    try:
        nltk.data.find('tokenizers/punkt')
        nltk.data.find('tokenizers/punkt_tab')
    except:
        nltk.download('punkt', quiet=True)
        nltk.download('punkt_tab', quiet=True)

    text_content = get_text_from_file(filepath)

    output_path = os.path.join(os.getcwd(), "last_scan_raw.txt")
    try:
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(text_content if text_content else "No text found.")
    except: pass

    # Clean & Parse
    clean_text = text_content.replace('\n', ' ').replace('\r', ' ')
    clean_text = re.sub(r'([-–—])', r' \1 ', clean_text)

    events = []
    start_base = None
    end_base = None
    
    # Range Search
    range_iter = re.finditer(DATE_RANGE_PATTERN, clean_text, flags=re.IGNORECASE)
    for match in range_iter:
        s_str = clean_ocr_date(match.group(1))
        e_str = clean_ocr_date(match.group(2))
        start_base = parse_date(s_str, settings={'PREFER_DATES_FROM': 'future'})
        end_base = parse_date(e_str, settings={'PREFER_DATES_FROM': 'future'})
        if start_base and end_base: break
            
    # Single Date Search
    if not start_base:
        date_iter = re.finditer(DATE_PATTERN, clean_text, flags=re.IGNORECASE)
        for match in date_iter:
            d_str = clean_ocr_date(match.group(1))
            start_base = parse_date(d_str, settings={'PREFER_DATES_FROM': 'future'})
            if start_base: break
    
    if not start_base: return []

    # Time Search
    time_range_match = re.search(TIME_RANGE_PATTERN, clean_text, flags=re.IGNORECASE)
    single_time_match = re.search(SINGLE_TIME_PATTERN, clean_text, flags=re.IGNORECASE)
    
    start_dt = start_base
    end_dt = end_base if end_base else None
    
    if time_range_match:
        t_start = time_range_match.group(1)
        t_end = time_range_match.group(2)
        meridiem = time_range_match.group(3) if time_range_match.group(3) else ""
        is_pm = meridiem.lower() == 'pm'
        start_dt = merge_date_and_time(start_base, t_start, is_pm_context=is_pm)
        target_end = end_base if end_base else start_base
        end_dt = merge_date_and_time(target_end, f"{t_end}{meridiem}")
    elif single_time_match:
        t_raw = single_time_match.group(1) + (single_time_match.group(2) if single_time_match.group(2) else "")
        start_dt = merge_date_and_time(start_base, t_raw)
        if end_base: end_dt = merge_date_and_time(end_base, t_raw)

    events.append({
        "title": "Scanned Event",
        "start_date_pretty": start_dt.strftime("%Y-%m-%d %H:%M:%S") if start_dt else "",
        "end_date_pretty": end_dt.strftime("%Y-%m-%d %H:%M:%S") if end_dt else "",
        "raw": clean_text[:300]
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