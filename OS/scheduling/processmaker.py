import random
names = ["short_thread_", "med_thread_", "long_thread_", "io_thread_", ]
count = [0, 0, 0, 0]
rand_io_range = [ (10,50), (20, 60), (30, 60), (50,100)]
rand_time_range = [  (5, 20), (20, 50),  (50, 1000), (10, 1000)]

with open("processes.txt", 'w') as f:
    for _ in range(100):
        t = random.randint(0, 3)
        # thread_name thread_type priority thread_length odds_of_IO
        f.write("%s%d %d %d %d %d\n"%(
            names[t],
            count[t],
            t,
            random.randint(0,2),
            random.randint(rand_time_range[t][0], rand_time_range[t][1]),
            random.randint(rand_io_range[t][0], rand_io_range[t][1])
        ))
        count[t] += 1
