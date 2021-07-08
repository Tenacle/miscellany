#!/usr/bin/env python3
import simpy
from scipy import stats
import math
import copy

NEIGHBOUR = {
        0:{ 6:4,
            8:2,
            9:3},
        1:{ 3:1,
            4:2},
        2:{ 3:4,
            5:3},
        3:{ 1:1,
            2:4,
            4:3,
            7:2},
        4:{ 1:2,
            3:3},
        5:{ 2:3,
            6:5},
        6:{ 0:4,
            5:5,
            7:3,
            8:6,
            9:2},
        7:{ 3:2,
            6:3,
            8:1},
        8:{ 0:2,
            6:6,
            7:1},
        9:{ 0:3,
            6:2}}

generated_id = dict()


# For my own testing, please ignore it's not part of actual code :)
message_count_dict = {
        0:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        1:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        2:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        3:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        4:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        5:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        6:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        7:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        8:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0},
        9:{ 0:0,
            1:0,
            2:0,
            3:0,
            4:0,
            5:0,
            6:0,
            7:0,
            8:0,
            9:0}}

# now this is actually part of the code, sort of. 
# This is for stats, a bit more detailed than what will be outputted but I wanted to see the whole picture initially
packet_stats = dict()
router_stats = dict()

# Dijkstra alg inspired from https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
def dijkstra(neighbour_list, root):
    dist = dict()
    prev = dict()
    q = dict()

    for key in neighbour_list.keys():
        if key == root:
            dist[key] = 0
            prev[key] = key
            q[key] = 0
        else:
            dist[key] = math.inf
            prev[key] = -1
            q[key] = math.inf

    # (helpful reading for me) https://stackoverflow.com/questions/3282823/get-the-key-corresponding-to-the-minimum-value-within-a-dictionary
    while q:
#        print(q)
        u = min(q, key=q.get)
        del q[u]
        #print("{}: {}".format(u, neighbour_list[u]))

        for neighbour in neighbour_list[u]:
            if dist[u] is math.inf:
                print("Encountered infinity. Graph error?")
                stop(1)

            val = dist[u] + neighbour_list[u][neighbour]
            #print("{},{}: {} + {} = {}".format(u, neighbour, dist[u], neighbour_list[u][neighbour], val))
            #print("{},{}: comparing {} to {}".format(u, neighbour, val, dist[neighbour]))
            if val < dist[neighbour]:
                #print("\t{},{}: found {} to be lower than {}".format(u, neighbour, val, dist[neighbour]))
                dist[neighbour] = val
                prev[neighbour] = u
                if neighbour in q:
                    q[neighbour] = val
                else:
                    print("************************* neighbour not found. looping back?")
                    stop(1)
    return dist, prev
    
# had fun researching for an answer. https://math.stackexchange.com/questions/882877/produce-unique-number-given-two-integers
def get_unique_id(id1, id2):
    if id1 > id2:
        arr = (id1, id2)
    else:
        arr = (id2, id1)
    if arr not in generated_id:
        generated_id[arr] = ((arr[0] * (arr[0] + 1))/2) + arr[1]
    #else:
    #    print("reusing computed id")
    return generated_id[arr]


# TODO: Prep broadcast events (10 for each router sent according to a poisson distribution with mean 20.
class Router(object):
    def __init__(self, env, router_id, channels, msg_mean, num_msgs,
            message_queue,
            priority_queue):
        self.env = env
        self.router_id = router_id
        self.channels = channels
        self.channels_queue = dict()
        self.msg_distribution = stats.poisson(mu=msg_mean)
        self.num_msgs = num_msgs
        self.dist, self.prev = dijkstra(NEIGHBOUR, router_id)

        # counters
        self.total_recv_messages = 0
        self.total_duplicated_messages = 0

        # router processes
        self.message_queue = message_queue
        self.message_priority_queue = priority_queue
        env.process(self.generate())
        env.process(self.process_message_queue())
        env.process(self.process_priority_queue())
        
        # interface processes
        for key, channel in channels.items():
            # msg queue for each channel
            self.channels_queue[key] = simpy.Store(env)
            env.process(self.get_messages(channel))
            env.process(self.put_messages(self.channels_queue[key], channel))
#        print("{}: channel queue keys {}".format(self.router_id, self.channels_queue.keys()))
#        for key, channel in channels.items():
            

    def is_shortest_path(self, source, from_neighbour):
        # source is neighbour, ofc it's SP
        if source != self.router_id and self.prev[source] == self.router_id and source == from_neighbour:
            #print("{}: >>>> THIS IS NEIGHBOUR ROUTER {} {}".format(self.router_id, source, from_neighbour))
            return True

        curr = self.prev[source]
        # trace from source till current neighbour
        while self.prev[curr] != self.router_id:
            curr = self.prev[curr]

        return from_neighbour == curr


    # generate messages to queue up based on poisson dist. 
    def generate(self):
        counter = 0
        #while counter < self.num_msgs:
        while True:
            if counter >= self.num_msgs:
                break
            yield self.env.timeout(self.msg_distribution.rvs())
            msg = Message(self.router_id, counter, self.env.now)
#            if self.router_id == 1:
#                print(">>>>>>>>>>>>>>> {}: Generated message {} at time \t\t{}".format(self.router_id, counter, self.env.now))
            self.message_queue.put(msg)
            counter += 1

#        if self.router_id == 1:
#            print(">>>>>>>>>>>>>> {}: FINISHED GENERATING MESSAGES at {}".format(self.router_id, self.env.now))

    # process for sending messages to relevant channel
    def process_message_queue(self):
        while True:
            msg = yield self.message_queue.get()

            for neighbour_queue in self.channels_queue.values():
                new_msg = copy.deepcopy(msg)
                neighbour_queue.put(new_msg)

            if len(self.message_queue.items) <= 0 and len(self.message_priority_queue.items) <= 0:
                router_stats[self.router_id]['time_ended'] = self.env.now

    # process arrived packets too, but give it "priority" :)
    def process_priority_queue(self):
        while True:
            msg = yield self.message_priority_queue.get()

            for neighbour_queue in self.channels_queue.values():
                new_msg = copy.deepcopy(msg)
                neighbour_queue.put(new_msg)


    ###### below are processes that are initialized for every channel (14 in total, at least two per router)
    # TODO process for putting messages to the medium (sending to other routers)
    def put_messages(self, queue, channel):
        sent_gen_msg = 0
        while True:
            msg = yield queue.get()
            # send to neighbour
            with channel.request() as request:
                yield request
                
                # attempting to send message
                channel.message_from = self.router_id
                if msg.source == self.router_id:
                    sent_gen_msg += 1
                msg.curr_source = self.router_id
                #yield self.env.timeout(channel.weight)
#                if msg.source == 1 and (self.router_id == 1 or self.router_id == 3 or self.router_id == 4):
#                    print("{}: sending message {}:{} with weight: {} \t{}".format(self.router_id, msg.source, msg.packet_id, channel.weight, self.env.now))
                #channel.put_in(self.router_id, msg)
                yield self.env.timeout(channel.weight)
                yield channel.get_put(self.router_id).put(msg)
#                if msg.source == 1 and (self.router_id == 1 or self.router_id == 3 or self.router_id == 4):
#                    print("{}: sent message {}:{} with weight: {} \t{}".format(self.router_id, msg.source, msg.packet_id, channel.weight, self.env.now))
                # msg successfully sent to router

    # TODO process for checking if there's a message in the medium and then process it
    def get_messages(self, channel):
        while True:
            msg = yield channel.get_from(self.router_id)
#            if msg.source == 1 and (self.router_id == 1 or self.router_id == 3 or self.router_id == 4):
#                print("{}: received message {}:{} with weight: {} \t{}".format(self.router_id, msg.source, msg.packet_id, channel.weight, self.env.now))
            self.total_recv_messages += 1
            if self.is_shortest_path(msg.source, msg.curr_source):
                visited_all = msg.visit(self.router_id, self.env.now)
                #if visited_all:
#                    if msg.source == 1 and (self.router_id == 1 or self.router_id == 3 or self.router_id == 4):
#                        print("\n\n{}: MESSAGE has visited all. source{} last router{}".format(self.router_id, msg.source, msg.curr_source))
#                if self.router_id == 1:
#                    print("{}: SP from source {} from router {}\t{}".format(self.router_id, msg.source, msg.curr_source, self.env.now))
                self.total_duplicated_messages += 1
                message_count_dict[self.router_id][msg.source] += 1
                self.message_priority_queue.put(msg)
            #else:
#                if self.router_id == 1:
#                    print("{}: LP from source {} from router {}\t{}".format(self.router_id, msg.source, msg.curr_source, self.env.now))
#            if self.router_id == 1:
#                print("{}: PRIORITY Q REMAINED>> {}".format(self.router_id, self.message_priority_queue))
#        if self.router_id == 1:
#            print(">>>>>>>>>>>>>> {}: FINISHED RECEIVING MESSAGES at {}".format(self.router_id, self.env.now))


# total of 14 will be made
class Channel(object):
    # interfaces/connections (neighbours)
    def __init__(self, env, weight, router1, router2):
        self.env = env
        self.interface = simpy.Resource(env, capacity=1)
        self.message = dict()
        for router in [router1, router2]:
            self.message[router] = simpy.Store(env)
        self.weight = weight

    # from https://simpy.readthedocs.io/en/latest/examples/latency.html
    def latency(self, router, msg):
        yield self.env.timeout(self.weight)
        for key, message in self.message.items():
            if key != router:
                yield message.put(msg)
    def put_in(self, router, msg):
        self.env.process(self.latency(router, msg))

    def get_put(self, router):
        for key, val in self.message.items():
            if key != router:
                return val

    def get_from(self, router):
        return self.message[router].get()

    def request(self):
        return self.interface.request()

def get_interfaces(env):
    interfaces = dict()
    interface_dict = dict()
    for key, neighbour_list in NEIGHBOUR.items():
        interface_dict[key] = dict()
        for neighbour, weight in neighbour_list.items():
            interface_id = get_unique_id(key, neighbour)
            if interface_id not in interfaces:
                interfaces[interface_id] = Channel(env, weight, key, neighbour)
            interface_dict[key][neighbour] = interfaces[interface_id]
    return interfaces, interface_dict

# check if there are any msgs for the router
#def receive(self, router_id)

# occupy the channel and send a message to the router
#def send(self, message, sender, recver):
#    # TODO handle resources to make sure it's used only once (CSMA)
#    with self.interfaces[get_unique_id(sender, recver)].request() as request:
#        yield request
#
#        print("{}: communicating to {} using interface {}")
#        yield self.env.process()


class Message(object):
    def __init__(self, source, packet_id, time_started):
        self.source = source
        self.curr_source = source
        self.packet_id = packet_id          # for detailed view. I wanna see things in detail
        packet_stats[source][packet_id]['visited'].append(source)
        self.time_started = time_started
        self.time_finished = -1
        #self.time_started and ended
        
    def last_router(self):
        return packet_stats[source][packet_id]['visited'][-1]

    def visit(self, router, time):
        if router not in packet_stats[self.source][self.packet_id]['visited']:
            packet_stats[self.source][self.packet_id]['visited'].append(router)
        #if self.source == 1:
            #print("{}: packet {} from time {} ({}) with {} routers 'visited' {}".format(self.source, self.packet_id, self.time_started, self.time_finished, len(packet_stats[self.source][self.packet_id]['visited']), packet_stats[self.source][self.packet_id]['visited']))
        if len(packet_stats[self.source][self.packet_id]['visited']) == 10:
            self.time_finished = time
            packet_stats[self.source][self.packet_id]['trans_time'] = self.time_finished - self.time_started
#            print("MESSAGE FROM SOURCE {} HAS VISITED ALL ROUTERS {} \ttime: {}".format(self.source, packet_stats[self.source][self.packet_id]['visited'], time))
            return True
        return False

#def rpf(router_list, source):



# TODO: Stats

def run(neighbour_list, mu, n):
    r = dict()
    main_queue = dict()
    priority_queue = dict()
    env = simpy.Environment()
    interfaces, interface_dict = get_interfaces(env)
    #print(interfaces)
    #print(interface_dict)
    for i in range(len(neighbour_list)):
        packet_stats[i] = dict()
        router_stats[i] = {'time_ended':-1}
        for j in range(n):
            packet_stats[i][j] = {'trans_time':0,'visited':[]}
        main_queue[i] = simpy.Store(env)
        priority_queue[i] = simpy.Store(env)
        r[i] = Router(env, i, interface_dict[i], mu, n, main_queue[i], priority_queue[i])
    
    env.run()
    
#    for i in range(len(neighbour_list)):
#        print("{}: Broadcasted {} incoming messages \n\t{} \n\t{}".format(i, r[i].total_duplicated_messages, r[i].message_priority_queue.items, r[i].message_queue.items))
#        print("{}: TOTAL       {} messages".format(i, r[i].total_recv_messages))

#    for key, val in message_count_dict.items():
#        print("Count for router", key)
#        for r, c in val.items():
#            print("{} \t {}".format(r, c))

#    for key, val in packet_stats.items():
#        print("Count for router", key)
#        for r, c in val.items():
#            print("{} \t {}".format(r, c))

    avg_trans_time = 0
    for r, msg in packet_stats.items():
        avg_r_trans_time = 0
        for packet_id, val in msg.items():
            avg_r_trans_time += val['trans_time']
        print("Router: {}\tTime finished: {}\tAverage transmit time: {}".format(r, router_stats[r]['time_ended'], avg_r_trans_time/n))
        avg_trans_time += avg_r_trans_time/n

    print("Total average transmit time:", avg_trans_time/len(neighbour_list))
        
run(NEIGHBOUR, 20, 10)

#d = {0:math.inf, 1:math.inf, 2:0, 3:1}
#print(d)
#print(min(d, key=d.get))
#print(min(d.values()))

#q = []
#q.append(5)
#q.append(2)
#q.append(1)
#print(q)
#print(q[1])
#q.remove(1)
#print(q)

