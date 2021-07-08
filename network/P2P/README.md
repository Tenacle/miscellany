Node.py
 > Finger table not used (tho it exist there)

NodeFingerTable.py
 > Finger table used.
 > Querries from ID+2**n
 > Smart->doesn't send a query to same node (ignored querries that would be replied by nodes already in the table)

Some extra commands:
 > quit - quits
 > pred - prints pred
 > succ - prints succ
 > me - prints me
 > stab - stabilizes. looks for fingertable too in NodeFingerTable
 > fingertable - prints fingertable. NOTE: this also works for Node.py but is not used/not updated in any way

For printing stuff:
 > I have some extra prints. I hope that's not a bother for logging.
