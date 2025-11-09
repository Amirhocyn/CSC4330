import regex as re
import nltk
from dateparser import parse as parse_date
from typing import List, Dict, Any

nltk.download('punkt', quiet=True)

DATE_RANGE_PATTERN = r"""
    (
        (?P<start>
            \d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4} |
            \d{4}[\/\-]\d{1,2}[\/\-]\d{1,2}   |
            \b(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z]*\s+\d{1,2}(?:,\s*\d{4})?
        )
        \s*(?:-|to|through|–|—)\s*
        (?P<end>
            \d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4} |
            \d{4}[\/\-]\d{1,2}[\/\-]\d{1,2}   |
            \b\d{1,2}(?:st|nd|rd|th)?(?:\s+(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z]*)?(?:,\s*\d{4})?
        )
    )
"""

SINGLE_DATE_PATTERN = r"""
    (
        \d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4}      |
        \d{4}[\/\-]\d{1,2}[\/\-]\d{1,2}        |
        \b(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z]*\s+\d{1,2}(?:,\s*\d{4})? |
        \b\d{1,2}\s+(?:jan|feb|mar|apr|may|jun|jul|aug|sep|sept|oct|nov|dec)[a-z]*(?:\s+\d{4})?
    )
"""

TIME_PATTERN = r"\b(\d{1,2}(:\d{2})?\s?(AM|PM|am|pm)?)\b"


def extract_schedule_events(text: str) -> List[Dict[str, Any]]:
    sentences = nltk.sent_tokenize(text)
    events = []

    for sent in sentences:
        # 1) Check for date ranges first
        range_match = re.search(DATE_RANGE_PATTERN, sent, flags=re.IGNORECASE | re.VERBOSE)
        if range_match:
            start_raw = range_match.group("start")
            end_raw = range_match.group("end")

            start_date = parse_date(start_raw)
            end_date = parse_date(end_raw)

            if start_date:
                # Event title = sentence with dates removed
                cleaned = sent.replace(range_match.group(0), "").strip(" -–—,;:.")
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

        cleaned = sent.replace(raw_date, "").replace(time_str if time_str else "", "").strip(" -–—,;:.")

        events.append({
            "title": cleaned if cleaned else "Event",
            "start_date": parsed_date.date().isoformat(),
            "end_date": parsed_date.date().isoformat(),  # single day event
            "time": time_str,
            "raw": sent.strip()
        })

    return events
