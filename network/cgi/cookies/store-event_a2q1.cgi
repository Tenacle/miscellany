#!/usr/bin/python
import sys
from os import environ
import os.path
import json
import urllib
import datetime

# retrieve form data
fields = {}
queryText = sys.stdin.readline()
if queryText:
    for line in queryText.split('&'):
        line = line.strip()
        key, value = line.split('=')
        fields[key] = value

if fields:
    if fields.has_key('eventPath'):
        path = urllib.unquote(fields['eventPath'])
        with open(path) as f:
            jsonData = json.load(f)
        
        try:
            datetime.datetime.strptime(fields['eventDate'], '%Y-%m-%d')
        except ValueError:
            fields['eventDate'] = datetime.datetime.today().strftime('%Y-%m-%d')

        newEvent = {
            "eventName": urllib.unquote_plus(fields['eventName']),
            "eventDate": fields['eventDate'],
            "eventDesc": urllib.unquote_plus(fields['eventDesc'])
        }

        jsonData['events'].append(newEvent)
        jsonData['events'].sort(key=lambda x: x['eventDate'])
        with open(path, 'w') as f:
            json.dump(jsonData, f, indent=4)
            f.truncate()

        print "Location: ./view-events_a2q1.cgi"
        print "Content-Type: text/html\n\n\n<html><body></body></html>"
    else:
        print """Content-Type: text/html\n\n\n
        <html><body>Something went wrong. No eventPath found.
        {}
        </body></html>
        """.format(fields)
else:
        #//print "Location: ./view-events_a2q1.cgi"
        print "Content-Type: text/html\n\n\n<html><body>Something went wrong. No path found<br/>{0}</body></html>".format(fields)
