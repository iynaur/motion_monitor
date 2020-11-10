import matplotlib.pyplot as plt
import numpy as np
from scipy.fftpack import fft,ifft

x=np.linspace(0,1,1400)      

#设置需要采样的信号，频率分量有180，390和600
y = np.zeros(1400)
for i in range(3):
    y = y + np.sin(2*np.pi*(i+1)*10*x)

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