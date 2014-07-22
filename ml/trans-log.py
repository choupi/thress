import sys
import math

for l in sys.stdin.readlines():
	sl=l.strip().split()
	if float(sl[16])<0: gain='0'
	else: gain=sl[16]
	ss=map(int,sl[:16])
	df=[ ss[i]-ss[i-1] for i in xrange(0,16) if i%4 ]
	df+=[ ss[i]-ss[i-4] for i in xrange(4,16) ]
	print gain, ' '.join(map(lambda x:str(math.log(abs(x)+1)),df))
