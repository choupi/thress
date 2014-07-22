import sys

for l in sys.stdin.readlines():
	sl=l.strip().split()
	if float(sl[18])<0: gain='0'
	else: gain=sl[18]
	ss=map(int,sl[:16])
	df=[ ss[i]-ss[i-1] for i in xrange(0,16) if i%4 ]
	df+=[ ss[i]-ss[i-4] for i in xrange(4,16) ]
	print gain, ' '.join(map(str,map(abs,df))), sl[16]
