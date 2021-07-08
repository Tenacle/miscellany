#!/usr/bin/python
import socket
import select
import os
import sys
import random
import json
import copy
import datetime
import pprint

import time

class Node:
    
    def __init__(self, **args):

        # initialize ME. localhost:port and ID
        if args and args.has_key('me'):
            self.me = args['me']
        else:
            self.me = {
                'ID': random.randint(1, 65534),
                'hostname': socket.gethostname(),
                'port': 15048,
                }
        if args and args.has_key('myPort'):
            self.me['port'] = args['myPort']
        if args and args.has_key('myId'):
            self.me['ID'] = args['myId']
        self.myAddress = (self.me['hostname'], self.me['port'])

        # initialize BOOTSTRAP
        if args and args.has_key('bootstrap'):
            self.bootstrap = args['bootstrap']
        else:
            self.bootstrap = {
                'ID': 65535,
                'hostname': 'silicon.cs.umanitoba.ca',
                'port': 15000
                }
        if args and args.has_key('btstrpId'):
            self.bootstrap['ID'] = args['btstrpId']
        if args and args.has_key('btstrpHostname'):
            self.bootstrap['hostname'] = args['btstrpHostname']
        if args and args.has_key('btstrpPort'):
            self.bootstrap['port'] = args['btstrpPort']
        btstrpAddress = (self.bootstrap['hostname'], self.bootstrap['port'])

        # Finger Table
        self.fingerTable = {}
        self.useFingerTable = False
            
        # initialize timeout, sockets, etc
        if args.has_key('timeout'):
            self.timeout = args['timeout']
        else:
            self.timeout = 2
        self.done = False

        self.theSucc = self.bootstrap.copy()
        self.thePred = {}

        self.dSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socketFD = self.dSocket.fileno()

    # populate finger table
    def populate_finger_table(self):
        self.fingerTable = {}
        self.fingerTable[self.bootstrap['ID']] = self.bootstrap
        populated = False
        index = 0
        lastID = 0

        while not populated:
            index += 1
            queryVal = self.me['ID'] + 2**index
            if lastID > queryVal:
                #print "{}: (Finger Table) Finger Table has stored a node for this query".format(self.time_now())
                continue

            # send queries
            self.start_node_query(queryVal)
            response = self.timed_listener("Failed to query for Node with value {}".format(queryVal))

            if response:
                try:
                    response = json.loads(response)
                except Exception as e:
                    print "{}: (Exception From Msg) {}".format(self.time_now(), e)
                    return
                
                #print "{}: (Finger Table) Received response from {} using query {}".format(self.time_now(), json.dumps(response), self.me['ID']+2**index)

                response.pop('cmd')
                response.pop('query')
                response.pop('hops')

                
                if self.fingerTable.has_key(int(response['ID'])) and self.fingerTable[int(response['ID'])]['port'] == (int(response['port'])):
                        # aquired msg from bootstrap. end of the line
                        if int(response['ID']) == self.bootstrap['ID']:
                            populated = True
                            self.useFingerTable = True
                else:
                    print "{} (Finger Table) Node added: {}".format(self.time_now(), json.dumps(response))
                    self.fingerTable[response['ID']] = response
                    lastID = response['ID']
    
            elif not response:
                populated = True
            
            #print json.dumps(self.fingerTable, indent=4)

        if self.useFingerTable:
            print "{}: (Finger Table) Successfully populated the finger table. Will use finger tables for now".format(self.time_now())
            #print json.dumps(self.fingerTable, indent=4)
        else:
            print "{}: (Finger Table) Failed to populate the finger table. Will refrain from using this for now".format(self.time_now())
            
    # main function for listening to input
    def listen(self):
        try:
            self.dSocket.bind(self.myAddress)
        except socket.error as e:
            print "{}: {}".format(self.time_now(), str(e))

        self.check_stability()
        #self.populate_finger_table()

        #print "Listening for input.."

        while not self.done:
            try:
                readFDs, writeFDs, errFDs = select.select([self.socketFD, sys.stdin], [], [])
                while readFDs and len(readFDs) > 0:
                    for desc in readFDs:

                        if desc == self.socketFD:
                            (recvData, addr) = self.dSocket.recvfrom(2048)
                            self.process_msg(recvData, addr)

                        elif desc == sys.stdin:
                            line = sys.stdin.readline().strip()
                            if line == "quit" or line == '':
                                #print self.time_now(), "exit command invoked. quitting.."
                                self.done = True
                            elif line == "pred":
                                print "{}: (Predecessor) {}".format(self.time_now(), json.dumps(self.thePred))#, indent=4))
                            elif line == "succ":
                                print "{}: (Successor) {}".format(self.time_now(), json.dumps(self.theSucc))#, indent=4))
                            elif line == "me":
                                print "{}: (ME) {}".format(self.time_now(), json.dumps(self.me))#, indent=4))
                            elif line == "stab":
                                self.check_stability()
                            elif line == "fingertable":
                                print json.dumps(self.fingerTable, indent=4)
                            elif self.is_int(line):
                                #self.populate_finger_table()
                                self.start_node_query(int(line))

                        readFDs.remove(desc)
            except KeyboardInterrupt:
                print
                self.done = True
            except ValueError as e:
                print "{}: (Received Gibberish) {}".format(self.time_now(), str(e))
                #print "Gibberish message received. Ignoring.."
            except Exception as e:
                print "{}: (Exception From Msg) {}".format(self.time_now(), str(e))
           #     print str(e)
           #     #print "Gibberish message received. Ignoring.."
           #     #print self.time_now(), "Unhandled exception caught at listen(). hm?"

        self.dSocket.close()
        print "{}: Connection closed.".format(self.time_now()) 

    def timed_listener(self, message):
        #print "listening for successor pred? reply"
        try:
            readFDs, writeFDs, errFDs = select.select([self.socketFD, sys.stdin], [], [], self.timeout)

            if readFDs:
                for desc in readFDs:
                    if desc == self.socketFD:
                        (recvData, addr) = self.dSocket.recvfrom(2048)
                        return recvData

                    readFDs.remove(desc)
            else:
                print "{}: (Timed out) {}".format(self.time_now(), message)
                return

        except KeyboardInterrupt:
            print
            self.done = True
        except Exception as e:
            print "{}: {}".format(self.time_now(), str(e))
            #print self.time_now(), "Unhandled exception caught at timed_listener(). hm?"
    
    def stability_helper(self, data):
        node = data['me']
        predNode = data['thePred']
        msg = self.me.copy()
        success = False

        #print "\nStabilizing with these data: "  
        #print json.dumps(data, indent=4)

        # pred is me or similar to me. takeover
        if predNode and predNode['hostname'] == self.me['hostname'] and int(int(predNode['port'])) == self.me['port'] or predNode and int(int(predNode['ID'])) == self.me['ID']:
            success = self.set_succ_permanently(node)
        
        # location found
        elif not predNode or predNode and int(predNode['ID']) < self.me['ID']:
            #print "{}: Attempting to stabilize..".format(self.time_now())

            success = self.set_succ_permanently(node)

            # only change predNode if they have one
            if predNode:
                self.set_pred(predNode)

            #print "{}: Success? {}".format(self.time_now(), success)
            
        # predecessor is bigger than me. can still look for a better spot
        else:
            self.set_succ(predNode)
            #print "\nMoving down the DHT with new potential successor:"
            #print json.dumps(self.theSucc, indent=4)

        #print "{}: Stabilization ended.".format(self.time_now())
        return success

    def check_stability(self):
        #print "checking out DHT from successor: " + json.dumps(self.theSucc)
        
        # "On attempting to stabilize the ring, initiate a new join at the bootstrap node."
        self.set_succ(self.bootstrap)
        self.useFingerTable = False
        
        # initialize message to be sent
        msg = self.me.copy()
        msg['cmd'] = "pred?"

        stable = False
        data = None
        oldData = None

        while not stable:
            self.dSocket.sendto(json.dumps(msg), (self.theSucc['hostname'], self.theSucc['port']))
            data = self.timed_listener("Failed querying for predecessor. Joining from the last known node instead.")

            #print data
            if data:
                data = json.loads(data)
                if data['cmd'] == "myPred":
                    #print "processing myPred message"
                    stable = self.stability_helper(data)
                    #print "done processing. stable: {0}".format(stable)
                else:
                    #print "received a message before stabilizing. checking.."
                    if data['hostname'] == self.me['hostname'] and int(data['port']) == self.me['port']:
                        #print "me"
                        stable = True
            else:
                if oldData:
                    #print oldData
                    stable = self.set_succ_permanently(oldData['me'])
                else:
                    stable = self.set_succ_permanently(self.bootstrap)

            oldData = data

        print "{}: (setPred sent) Ring stabilized with:\n\tsuccessor:\t{}\n\tpredecessor:\t{}".format(self.time_now(), json.dumps(self.theSucc), json.dumps(self.thePred))
        self.populate_finger_table()

    def is_int(self, string):
        try:
            int(string)
            return True
        except ValueError:
            return False

    def get_pred(self, msg):
        #print "Received inquiry for pred"
        response = {
            'msg': {
                'cmd': "myPred",
                'me': self.me,
                'thePred': self.thePred
                },
            'addr': (msg['hostname'], int(msg['port']))
            }
        self.dSocket.sendto(json.dumps(response['msg']), response['addr'])
        #print "Response sent"

    def set_pred(self, newPred):
        #print "Received request to set pred"
        try:
            if newPred['ID'] < self.me['ID']:
                self.thePred['ID'] = int(newPred['ID'])
                self.thePred['hostname'] = newPred['hostname']
                self.thePred['port'] = int(newPred['port'])
                print "{}: (setPred invoked) Pred changed to {}".format(self.time_now(), json.dumps(self.thePred))#, indent=4))
        except:
            print "{}: (setPred failed) unable to set pred unknown values found. Ignoring this setPred.".format(self.time_now()) 
            return

    def set_succ(self, newSucc):
        #print "Setting successor to ID:{0} with port:{1}".format(
            #newSucc['ID'],
            #newSucc['port'])
        try:
            self.theSucc['ID'] = int(newSucc['ID'])
            self.theSucc['hostname'] = newSucc['hostname']
            self.theSucc['port'] = int(newSucc['port'])
        except:
            print "{}: unable to set succ's pred as succ.".format(self.time_now()) 
            return False
        return True

    def set_succ_permanently(self, newSucc):
        #print "Setting successor to ID:{0} with port:{1}".format(
            #newSucc['ID'],
            #newSucc['port'])
        try:
            self.theSucc['ID'] = int(newSucc['ID'])
            self.theSucc['hostname'] = newSucc['hostname']
            self.theSucc['port'] = int(newSucc['port'])
        except:
            print "{}: unable to set succ.".format(self.time_now()) 
            return False

        msg = self.me.copy()
        msg['cmd'] = "setPred"
        msg = {
            'msg': msg,
            'addr': (self.theSucc['hostname'], self.theSucc['port'])
            }
        self.dSocket.sendto(json.dumps(msg['msg']), msg['addr'])
        return True

    def start_node_query(self, num):
        #print "Initiating a query. ({0})".format(num)
        msg = self.me.copy()
        msg['cmd'] = "find"
        msg['query'] = num
        msg['hops'] = -1
        self.node_query(msg)
    
    def node_query(self, msg):
        #print "Find query received. processing.."
        msg['hops'] += 1
        response = None

        # Query successful. Initiating response
        if int(msg['query']) <= self.me['ID']:
            response = self.me.copy()
            response['hops'] = msg['hops']
            response['query'] = msg['query']
            response['cmd'] = "owner"
            response = {
                'msg': response,
                'addr': (msg['hostname'], msg['port'])
                }

        else:
            if self.useFingerTable:
                self.check_stability()

            # finger table exist. use it
            if self.useFingerTable:
                # dict HAXX
                sortedByValFingerList = self.sort_by_val(self.fingerTable)

                for t in sortedByValFingerList:
                    if t[1] >= msg['query']:
                        print "Found a node matching in the finger table:", msg['query'], t[0], json.dumps(self.fingerTable[int(t[0])])
                        response = {
                            'msg': msg,
                            'addr': (self.fingerTable[t[0]]['hostname'], self.fingerTable[t[0]]['port'])
                        }
                        break;
            
            # Query failed. Passing to successor
            else:
                response = {
                    'msg': msg,
                    'addr': (self.theSucc['hostname'], self.theSucc['port'])
                    }
        self.dSocket.sendto(json.dumps(response['msg']), response['addr'])

    def output_query_result(self, msg):
        msg.pop('cmd')
        print "{}: (owner received) Received response from query.\n\tOwner:\t{}".format(self.time_now(), json.dumps(msg))#, indent=4))
        #print "Recieved response for sent query"
        #print json.dumps(msg, indent=4)

    def process_msg(self, recvData, addr):
        try:
            msg = json.loads(recvData)
        except TypeError:
            print recvData
            print "{}: data json conversion failed. ignoring message..".format(self.time_now()) 
            return
            
        if msg.has_key('cmd'):
            cmd = msg['cmd']
            
            if cmd == 'pred?':
                self.get_pred(msg)

            elif cmd == 'myPred':
                print "{}: Received 'myPred' outside stabilization query. Attempting to process.."
                return self.stability_helper(msg)

            elif cmd == 'setPred':
                self.set_pred(msg)

            elif cmd == 'find':
                self.node_query(msg)

            elif cmd == 'owner':
                self.output_query_result(msg)

        else:
            print "{}: Received msg without 'cmd'. ignoring.. ({})".format(self.time_now(), json.dumps(msg))

    def time_now(self):
        return datetime.datetime.now().time()

    def sort_by_key(self, toSortDict):
        sortedByKeyDict = sorted(toSortDict.items(), key=lambda t: t[0])
        return sortedByKeyDict

    def sort_by_val(self, toSortDict):
        sortedByValDict = sorted(toSortDict.items(), key=lambda t: t[1])
        return sortedByValDict

if __name__ == "__main__":
    n = Node()
    print "{}: Node initialized. Attempting to join..".format(n.time_now())
    # just initial thing to kickstart it I guess
    n.listen()
