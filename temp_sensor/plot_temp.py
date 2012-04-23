import serial
import Gnuplot
from numpy import *
import time

print "Setting up port"
port=serial.Serial('/dev/ttyACM0',2400)
sample_rate=3 #update every 10s
start=time.time()
g=Gnuplot.Gnuplot()
g.xlabel('Elapsed Time (minutes)')
g.ylabel('Temperature')
Data=empty((0,2))
while 1:
    now=time.time()
    elapsed=(now-start)/60.0 #elapsed time in minutes
    next_sample=now+sample_rate
    
    raw=port.read(port.inWaiting())
    if len(raw)>0:
        print (raw)
        temps=empty(0)
        for c in raw:
            t=float(ord(c))
            temps=concatenate([temps,array([t])])
        print temps.mean()
        point=array([elapsed,temps.mean()]).reshape(-1,2)
        Data=concatenate([Data,point])
        plot=Gnuplot.Data(Data,with_='lines')
        g.plot(plot)
    time.sleep(next_sample-time.time())
    
