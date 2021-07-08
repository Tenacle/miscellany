#!/usr/bin/python
import sys
from os import environ
import json

# let's fetch some cookies
# check for current user
cookies = {}
currUser = ''
if environ.has_key('HTTP_COOKIE'):
    for cookie in environ['HTTP_COOKIE'].split(';'):
        (key, value) = cookie.strip().split('=', 1)
        cookies[key] = value
if cookies:
    if cookies.has_key('currUser'):
        currUser = cookies['currUser'] 

# fetch userListData.json
# parse userListData
userList = ''
jsonData = {}
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
            
# header and some styling
print """Content-Type: text/html\n
<html><head><title>View Events</title>
<style>             
fieldset legend {   
font-size:30px      
}                   
label, input {      
display:inline-block
}                   
label {             
width:100px;        
}                   
select {            
width:125px;        
}                   
</style></head>     
"""

# body
print"""
<body>
<div align="left" style="width:400px">
<fieldset><legend>View User Events</legend>
<form action="./add-event_a2q1.cgi"><label style="text-align:right">User name:</label>
<select id="users" onchange="processUsersReq(event)">
    <option value="">--</option>
"""
print userList
print"""
</select><br/>
<label style="vertical-align:top;text-align:right">User Events:</label>
<select size="6" id="events" onchange="processEventReq(event)" style="width:264px">
    <option value="" selected>--</option>
</select>
<label style="width:100%">Event details:</label>
<div id="eventDetails" style="border:1px solid grey;width:100%;height:200px;padding:1px">Please select an event</div><br/>
<input type="submit" value="New Event" style="padding:5px">
</form></fieldset></div>

<script type="text/javascript">

var oReq = new XMLHttpRequest();
var currUser = '';
var eventList = [];
onloadEvent();

// do some early checking
function onloadEvent() {
    elem = document.getElementById('users');
    if( elem ){
        try {
            if( elem.selectedIndex > 0 ) {
                loadXMLDoc(elem.options[elem.selectedIndex].value);
            }
        }
        catch(e) {
            var msg = (typeof e == "string") ? e : ((e.message) ? e.message : "Unknown Error");
            alert("Unable to get JSON data:" + msg);
            return;
        }
    }
};

function parseXMLDoc(oReq) {
    //document.getElementByID
    var response = JSON.parse(oReq.responseText);
    eventList = response.events;
    var output = '';
            
    for( var i = 0; i < eventList.length; i++){
        thisEvent = eventList[i];
        thisEvent.eventDate = new Date(thisEvent.eventDate);
        output += "<option value=" + i + ">" + "(" + thisEvent.eventDate.toDateString() + ") " + thisEvent.eventName + "</option>";
    }

    console.log(eventList);
    document.getElementById('events').innerHTML = output;

}

function loadXMLDoc(url) {
    //oReq.addEventListener("load", reqListener);
    oReq.onreadystatechange = function() {
        if( this.readyState == 4 && this.status == 200 ) {
            parseXMLDoc(oReq);
        }
    };
    oReq.open("GET", url);
    oReq.send();
};

function processUsersReq(evt) {
    console.log(evt);
    if( evt ) {
        var elem = (evt.target) ? evt.target : ((evt.srcElement) ? evt.srcElement : null);
        if( elem ){
            try {
                if( elem.selectedIndex > 0 ) {
                    document.cookie = "currUser=" + elem.options[elem.selectedIndex].text;
                    loadXMLDoc(elem.options[elem.selectedIndex].value);
                }
            }
            catch(e) {
                var msg = (typeof e == "string") ? e : ((e.message) ? e.message : "Unknown Error");
                alert("Unable to get JSON data:" + msg);
                return;
            }
        }
    }
};

function displayEventDetails(index) {
    thisEvent = eventList[index];
    output = "<strong>" + thisEvent.eventName + "</strong><br/><em>" + 
        thisEvent.eventDate.toDateString() + "</em><br/><br/>" + thisEvent.eventDesc;

    document.getElementById('eventDetails').innerHTML = output;
}

function processEventReq(evt) {
    console.log(evt);
    if( evt ) {
        var elem = (evt.target) ? evt.target : ((evt.srcElement) ? evt.srcElement : null);
        if( elem ){
            try {
                if( elem.selectedIndex >= 0 ) {
                    displayEventDetails(elem.options[elem.selectedIndex].value);
                }
            }
            catch(e) {
                var msg = (typeof e == "string") ? e : ((e.message) ? e.message : "Unknown Error");
                alert("Unable to get JSON data:" + msg);
                return;
            }
        }
    }
};

</script>

</body></html>
"""

