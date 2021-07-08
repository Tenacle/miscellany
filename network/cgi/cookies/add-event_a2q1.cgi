#!/usr/bin/python
import sys
from os import environ
import json
import datetime

# retrieve cookie data;
# check curr user
cookies = {}; currUser = ''
if environ.has_key('HTTP_COOKIE'):
    for cookie in environ['HTTP_COOKIE'].split(';'):
        key, value = cookie.strip().split('=', 1)
        cookies[key] = value
if cookies.has_key('currUser'):
    currUser = cookies['currUser']

# fetch userListData.json
# parse userListData into selectable value
userList = ''
with open('../json/userList.json') as f:
    jsonData = json.load(f)
if jsonData:
    for data in jsonData['users']:
        if currUser == data['userID']:
            userList += "<option selected "
        else:
            userList += "<option "
        userList += "value={0}>{1}</option>\n".format(
            data['events'],
            data['userID']
        )

# header and style
print """Content-Type: text/html\n
<html><head><title>Add Event</title>
<style>
fieldset legend {
font-size:30px
}
label, input {
text-align: left;
display:inline-block
}
label {
width:100px;
padding-bottom:1px;
padding-right:5px;
text-align:right
}
select {
width:125px;
}
</style></head>
"""

# body and the forms
print """<body>
<div align="left" style="width: 400px;">
<form action="store-event_a2q1.cgi" method="POST">
    <fieldset><legend>Add User Event</legend>
    <label>User ID:</label>
    <select name="eventPath">
        <option value="">--</option>
        {0}
    </select><br/>
    <label>Event Name:</label><input type="text" name="eventName" style="width:200px"><br/>
    <label>Date:</label><input type="date" name="eventDate"><br/>
    <label style="text-align:left;width:100%">Event Description:</label>
    <textarea name="eventDesc" style="width:100%;height:125px"></textarea><br/>
    <input type="submit" value="Add Event" style="padding:5px;margin-top:5px">
    </fieldset>
</form></div>
""".format(userList)
