import matplotlib.pyplot as plt

file1 = open('top.txt', 'r') 
allines = file1.readlines() 
  
lines = []
step = 50
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

allshow = 0
if 1: 
    for i in range(6):
        plt.title(titles[i])
        
        

        plt.title(titles[i])
        plt.plot(data[i])

        v, t = pick("14:58:00", "15:02:00", i)
        plt.plot(t, v)

        for idx in range(len(data[i])):
            if idx % 1000 == 0:
                plt.text(idx, data[i][idx], timestamp[idx])
        mng = plt.get_current_fig_manager()
        mng.window.state('zoomed') 


        plt.show()

tot = [data[0][i]**2 + data[1][i]**2 + data[2][i]**2 for i in range(len(data[0]))]
plt.plot(tot)
for idx in range(len(data[i])):
    if idx % 1000 == 0:
        plt.text(idx, 0, timestamp[idx])
mng = plt.get_current_fig_manager()
mng.window.state('zoomed') 
plt.show()