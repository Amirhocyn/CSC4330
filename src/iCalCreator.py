#!/usr/bin/env python3
"""
iCalCreator.py - Streamlined calendar file generator using calendar-link library

Creates .ics files and direct calendar links for Google Calendar and Microsoft 365.
Uses the calendar-link library for proper iCalendar formatting.
"""

from typing import List, Optional
from datetime import datetime, date, time
import calendar_link


def create_ical_file(events: List, filename: str = "events.ics") -> bool:
    """
    Creates a .ics file from event objects using calendar-link library.
    
    Works with any objects that have these getter methods:
    - getTitle()
    - getDate() or getStartDate()      # YYYYMMDD or YYYY-MM-DD
    - getStartTime()                    # HHMMSS
    - getEndTime() (optional)           # HHMMSS
    - getLocation() (optional)
    - getDescription() or getDetails() (optional)
    
    Args:
        events: List of event objects with getter methods
        filename: Output filename (will be saved as .ics)
    
    Returns:
        True if file created successfully, False otherwise
    
    Example:
        >>> events = [event1, event2, event3]
        >>> create_ical_file(events, "my_calendar.ics")
        ✓ Successfully created my_calendar.ics with 3 event(s)
        True
    """
    
    if not events:
        print("Error: No events provided")
        return False
    
    # Ensure filename ends with .ics
    if not filename.endswith('.ics'):
        filename += '.ics'
    
    try:
        # Convert event objects to calendar-link Event objects
        calendar_events = []
        
        for event in events:
            # Convert to calendar-link Event
            cal_event = _convert_to_calendar_event(event)
            if cal_event:
                calendar_events.append(cal_event)
        
        if not calendar_events:
            print("Error: No valid events to export")
            return False
        
        # Generate ICS content using calendar-link
        ics_content = _generate_ics_content(calendar_events)
        
        # Write to file
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(ics_content)
        
        print(f"✓ Successfully created {filename} with {len(calendar_events)} event(s)")
        return True
        
    except Exception as e:
        print(f"✗ Error creating .ics file: {e}")
        return False


def _convert_to_calendar_event(event) -> Optional[calendar_link.CalendarEvent]:
    """
    Convert an event object to calendar-link CalendarEvent.
    
    Args:
        event: Event object with getter methods
        
    Returns:
        CalendarEvent object or None if conversion fails
    """
    try:
        # Get title (required)
        title = event.getTitle() if hasattr(event, 'getTitle') else "Event"
        
        # Get date (required)
        event_date = None
        if hasattr(event, 'getDate'):
            date_str = event.getDate()  # YYYYMMDD format
            if date_str and len(date_str) == 8:
                # Parse YYYYMMDD
                year = int(date_str[:4])
                month = int(date_str[4:6])
                day = int(date_str[6:8])
                event_date = date(year, month, day)
        
        if not event_date and hasattr(event, 'getStartDate'):
            date_str = event.getStartDate()  # YYYY-MM-DD format
            if date_str:
                # Parse ISO format
                parts = date_str.split('-')
                if len(parts) == 3:
                    event_date = date(int(parts[0]), int(parts[1]), int(parts[2]))
        
        if not event_date:
            print(f"Warning: Event '{title}' missing date, skipping")
            return None
        
        # Get start time (optional, default 9:00 AM)
        start_time_obj = time(9, 0)  # Default
        if hasattr(event, 'getStartTime'):
            time_str = event.getStartTime()  # HHMMSS format
            if time_str and len(time_str) >= 4:
                hour = int(time_str[:2])
                minute = int(time_str[2:4])
                start_time_obj = time(hour, minute)
        
        # Combine date and time
        begin = datetime.combine(event_date, start_time_obj)
        
        # Get end time (optional, default 1 hour after start)
        end_time_obj = None
        if hasattr(event, 'getEndTime'):
            time_str = event.getEndTime()
            if time_str and len(time_str) >= 4:
                hour = int(time_str[:2])
                minute = int(time_str[2:4])
                end_time_obj = time(hour, minute)
        
        # Calculate end datetime
        if end_time_obj:
            end = datetime.combine(event_date, end_time_obj)
        else:
            # Default to 1 hour after start
            from datetime import timedelta
            end = begin + timedelta(hours=1)
        
        # Get optional fields
        location = ""
        if hasattr(event, 'getLocation'):
            location = event.getLocation() or ""
        
        description = ""
        if hasattr(event, 'getDescription'):
            description = event.getDescription() or ""
        elif hasattr(event, 'getDetails'):
            description = event.getDetails() or ""
        
        # Create calendar-link Event
        cal_event = calendar_link.CalendarEvent(
            summary=title,
            begin=begin,
            end=end,
            location=location,
            description=description
        )
        
        return cal_event
        
    except Exception as e:
        print(f"Warning: Failed to convert event: {e}")
        return None


def _generate_ics_content(events: List[calendar_link.CalendarEvent]) -> str:
    """
    Generate ICS file content from calendar events.
    
    Args:
        events: List of CalendarEvent objects
        
    Returns:
        Complete ICS file content as string
    """
    # Build ICS content
    ics_lines = []
    
    # Header
    ics_lines.append("BEGIN:VCALENDAR")
    ics_lines.append("VERSION:2.0")
    ics_lines.append("PRODID:-//TimeKeeper//Event Extractor//EN")
    ics_lines.append("CALSCALE:GREGORIAN")
    ics_lines.append("METHOD:PUBLISH")
    
    # Add each event
    for event in events:
        # Use calendar-link's ICS generator
        event_ics = event.to_ics()
        
        # Extract just the VEVENT portion (remove VCALENDAR wrapper)
        lines = event_ics.strip().split('\n')
        in_event = False
        for line in lines:
            if line.startswith('BEGIN:VEVENT'):
                in_event = True
            if in_event:
                ics_lines.append(line)
            if line.startswith('END:VEVENT'):
                in_event = False
    
    # Footer
    ics_lines.append("END:VCALENDAR")
    
    return '\n'.join(ics_lines)


def create_google_calendar_link(event) -> Optional[str]:
    """
    Create a direct Google Calendar link for a single event.
    
    Args:
        event: Event object with getter methods
        
    Returns:
        Google Calendar URL or None if conversion fails
        
    Example:
        >>> link = create_google_calendar_link(event)
        >>> print(link)
        https://calendar.google.com/calendar/render?action=TEMPLATE&text=...
    """
    try:
        cal_event = _convert_to_calendar_event(event)
        if not cal_event:
            return None
        
        # Generate Google Calendar link
        return cal_event.to_google()
        
    except Exception as e:
        print(f"Error creating Google Calendar link: {e}")
        return None


def create_microsoft_calendar_link(event) -> Optional[str]:
    """
    Create a direct Microsoft 365 Calendar link for a single event.
    
    Args:
        event: Event object with getter methods
        
    Returns:
        Microsoft 365 Calendar URL or None if conversion fails
        
    Example:
        >>> link = create_microsoft_calendar_link(event)
        >>> print(link)
        https://outlook.live.com/calendar/0/deeplink/compose?path=...
    """
    try:
        cal_event = _convert_to_calendar_event(event)
        if not cal_event:
            return None
        
        # Generate Microsoft 365 link
        return cal_event.to_microsoft()
        
    except Exception as e:
        print(f"Error creating Microsoft Calendar link: {e}")
        return None


def create_calendar_links(events: List) -> dict:
    """
    Create calendar links for all events.
    
    Args:
        events: List of event objects
        
    Returns:
        Dictionary with 'google' and 'microsoft' keys containing lists of URLs
        
    Example:
        >>> links = create_calendar_links([event1, event2])
        >>> print(links['google'][0])
        >>> print(links['microsoft'][0])
    """
    google_links = []
    microsoft_links = []
    
    for event in events:
        google_link = create_google_calendar_link(event)
        if google_link:
            google_links.append(google_link)
        
        microsoft_link = create_microsoft_calendar_link(event)
        if microsoft_link:
            microsoft_links.append(microsoft_link)
    
    return {
        'google': google_links,
        'microsoft': microsoft_links
    }


def open_calendar_file(filename: str):
    """
    Open the calendar file in the system's default calendar application.
    
    Args:
        filename: Path to .ics file
    """
    import os
    import sys
    
    if sys.platform == 'win32':
        os.startfile(filename)
    elif sys.platform == 'darwin':  # macOS
        os.system(f'open "{filename}"')
    else:  # Linux
        os.system(f'xdg-open "{filename}"')


def main():
    """
    Example usage demonstrating the streamlined API.
    """
    print("=" * 70)
    print("iCalCreator.py - Streamlined Calendar Generator")
    print("Using calendar-link library")
    print("=" * 70)
    
    # Create a mock event for demonstration
    class MockEvent:
        def __init__(self, title, date, start_time, end_time="", location="", description=""):
            self._title = title
            self._date = date
            self._start_time = start_time
            self._end_time = end_time
            self._location = location
            self._description = description
        
        def getTitle(self): return self._title
        def getDate(self): return self._date
        def getStartTime(self): return self._start_time
        def getEndTime(self): return self._end_time
        def getLocation(self): return self._location
        def getDescription(self): return self._description
    
    # Create test events
    events = [
        MockEvent(
            title="CSC 4402 Final Exam",
            date="20250515",
            start_time="140000",
            end_time="170000",
            location="Patrick F. Taylor Hall Room 3305",
            description="Comprehensive final exam covering all course material"
        ),
        MockEvent(
            title="Midterm Exam",
            date="20250310",
            start_time="140000",
            end_time="153000",
            location="PFT Hall",
            description="Midterm covering chapters 1-6"
        ),
        MockEvent(
            title="Project Presentation",
            date="20250420",
            start_time="100000",
            end_time="120000",
            location="Engineering Building",
            description="Final project presentation"
        ),
    ]
    
    print("\n1. Creating .ics file...")
    success = create_ical_file(events, "test_calendar.ics")
    
    if success:
        print("\n2. Generating calendar links...")
        links = create_calendar_links(events)
        
        print(f"\n   Google Calendar links generated: {len(links['google'])}")
        print(f"   Microsoft 365 links generated: {len(links['microsoft'])}")
        
        print("\n   Example Google Calendar link:")
        if links['google']:
            print(f"   {links['google'][0][:80]}...")
        
        print("\n   Example Microsoft Calendar link:")
        if links['microsoft']:
            print(f"   {links['microsoft'][0][:80]}...")
        
        print("\n" + "=" * 70)
        print("✓ SUCCESS!")
        print("=" * 70)
        print("\nFiles created:")
        print("  • test_calendar.ics")
        print("\nTo use:")
        print("  1. Double-click test_calendar.ics to import all events")
        print("  2. Or use the generated links to add events individually")
        print("\nOpening calendar file...")
        
        try:
            open_calendar_file("test_calendar.ics")
        except:
            print("  (Please open test_calendar.ics manually)")
    else:
        print("\n✗ Failed to create calendar file")


if __name__ == "__main__":
    main()