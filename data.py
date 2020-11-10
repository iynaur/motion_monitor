import matplotlib.pyplot as plt
import numpy as np
from scipy.fftpack import fft,ifft

file1 = open('top.txt', 'r') 
allines = file1.readlines() 
  
lines = []
step = 1
for i in range(len(allines) // step):
    j = step * i
    if j != 0:
        lines.append(allines[j])

# ax = []
# ay = []
# az = []
# gx = []
# gy = []
# gz = []

data = [[] for _ in range(6)]
timestamp = []

for line in lines:
    paras = line.split()
    for i in range(6):
        para_str = paras[i].split(':')[1]
        data[i].append(float(para_str))
    timestamp.append(paras[9][:8])
titles = ["ax", "ay",  "az",  "gx",  "gy",  "gz", ]

def timecnt(tstr):
    return int(tstr[0:2])*3600 + int(tstr[3:5])*60 + int(tstr[6:8])

def pick(s_str, e_str, i):
    v = []
    t = []
    for idx in range(len(lines)):
        line = lines[idx]
        paras = line.split()
        tstr = paras[9][:8]
        if timecnt(tstr) < timecnt(e_str) and timecnt(tstr) > timecnt(s_str):
            para_str = paras[i].split(':')[1]
            v.append(float(para_str))
            t.append(idx)
    return v, t

def moving_average(interval, windowsize): # return array
    window = np.ones(int(windowsize)) / float(windowsize)
    re = np.convolve(interval, window, 'same')
    return re # return array

def atitude(a, duration):
    res = a.copy() # array slice is different!!!
    n = len(a)
    for i in range(len(a)):
        mi = max(0, i - duration)
        ma = min(n-1, i + duration)
        res[i] = max(a[mi:ma+1]) - min(a[mi:ma+1])
    return res



if 0:
    for i in range(6):
        data[i] = moving_average(data[i], 10) # return array


daisu_timeidx = [250000, 
550000] # weird freq

allshow = 0
if 0: 
    for i in range(6):
        # i = (i + 3) % 6
        plt.title(titles[i])
        
        
        # plt.plot(atitude(data[i], 20))
        plt.plot((data[i]))

        # v, t = pick("14:50:30", "16:14:30", i)
        # plt.plot(t, v)

        curtime = ""
        for idx in range(len(data[i])):
            if timestamp[idx] != curtime and timecnt(timestamp[idx]) % 600 == 0:
                plt.text(idx, data[i][idx], timestamp[idx])
                curtime = timestamp[idx]
        mng = plt.get_current_fig_manager()
        mng.window.state('zoomed') 


        plt.show()
else:
    
    start = 246000
    while start < 250000:
        start += 100
        length = 128
        y = data[0][start:start+ length]
        yy=fft(y)                     #快速傅里叶变换
        # yreal = yy.real               # 获取实数部分
        # yimag = yy.imag               # 获取虚数部分

        yf=abs(fft(y))                # 取绝对值
        yf1=abs(fft(y))/len(y)           #归一化处理
        yf2 = yf1[1 : (int(len(y)/2))]  #由于对称性，只取一半区间

        xf = np.arange(len(y))        # 频率
        xf1 = xf
        xf2 = xf[1 : (int(len(y)/2)):]  #取一半区间
        plt.plot(xf2,yf2,'r')
        mng = plt.get_current_fig_manager()
        mng.window.state('zoomed') 


        plt.show()
exit(0)
tot = [data[0][i]**2 + data[1][i]**2 + data[2][i]**2 for i in range(len(data[0]))]
plt.plot(tot)
for idx in range(len(data[i])):
    if idx % 1000 == 0:
        plt.text(idx, 0, timestamp[idx])
mng = plt.get_current_fig_manager()
mng.window.state('zoomed') 
plt.show()