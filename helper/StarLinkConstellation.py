import numpy as np
import matplotlib as mpl
mpl.use("TkAgg")
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animmation

def update(time):
    global line, text
    for i in range(len(position[time])):
        lineTemp = line[i]
        # 更新卫星位置
        lineTemp.set_data(position[time][i][0], position[time][i][1])
        lineTemp.set_3d_properties(position[time][i][2])
        # 更新卫星注释位置
        # textSat = text[i]
        # textSat.remove()
        # textSat = ax.text(position[time][i][0], position[time][i][1], position[time][i][2], "satellite" + str(i))
        # text[i] = textSat


def init():
    global line, text
    line = []
    text = []
    for i in range(len(position[0])):
        # 卫星的初始位置
        line3, = ax.plot([position[0][i][0]], [position[0][i][1]], [position[0][i][2]], marker='o', color='red', markersize=4)
        line.append(line3)
        # 卫星的注释
        # textSat = ax.text(position[0][i][0], position[0][i][1], position[0][i][2], "satellite"+str(i))
        # text.append(textSat)


# 读取StarLink-topology.txt文件
def read(readFile):
    global time, position
    f = open(readFile, "r")
    time = []
    position = [] 
    index = 0
    for each_line in f:
        if(each_line[0:4]=="time"):
            # time.append(int(each_line[1:]))
            # position.append([])
            continue
        else:
            if(index % 4425 == 0): position.append([])
            index += 1
            a, b, c = each_line.split(" ")
            temp = []
            temp.append(float(a))
            temp.append(float(b))
            c = c.rstrip("\n")# 可能c尾部有\n，要去掉
            temp.append(float(c))
            position[-1].append(temp)
    f.close()


# 画第x个高度的卫星轨道（轨道数：num）
# 参数说明： num：轨道数  inclination：倾斜角  r:轨道半径  color：轨道颜色  ax：画线的工具
def draw(num,inclination,r,color,ax,omega1,t_range):
    x0 = r*np.cos(omega1*t_range)
    y0 = r*np.sin(omega1*t_range)
    z0 = 0
    x1 = x0
    y1 = np.cos(inclination)*y0-np.sin(inclination)*z0
    z1 = np.sin(inclination)*y0+np.cos(inclination)*z0
    dengcha = 180//num
    for i in range(num):
        shengjiaodian = dengcha*i*np.pi/180
        x2 = np.cos(shengjiaodian)*x1-np.sin(shengjiaodian)*y1
        y2 = np.sin(shengjiaodian)*x1+np.cos(shengjiaodian)*y1
        z2 = z1
        ax.plot(x2, y2, z2, color)




if __name__ == '__main__':

    read("startLink-2020-12-21.txt")

    
    R = 6378  # 地球半径
    r1 = 6378 + 1150 # StarLink卫星的半径r
    inc1 = 53 * np.pi / 180 # 倾斜角
    r2 = 6378 + 1110
    inc2 = 53.8 * np.pi / 180
    r3 = 6378 + 1130
    inc3 = 74 * np.pi / 180
    r4 = 6378 + 1275
    inc4 = 81 * np.pi / 180
    r5 = 6378 + 1325
    inc5 = 70 * np.pi / 180


    f = plt.figure(figsize=(12,12))
    ax = f.add_subplot(111, projection='3d')

    # equal表示x和y维度在数据坐标中的长度相同。要获取方轴，您可以手动设置纵横比：
    # ax.set_aspect(1./ax.get_data_ratio())
    # ax.set_aspect('equal')
    ax.set_title("StarLink Model")

    # 地球模型
    ax.plot([0], [0], [0], marker='o', color='blue', markersize=220)
    # u = np.linspace(0, 2 * np.pi, 100) # linspace的功能是创建等差数列。
    # v = np.linspace(0, np.pi, 100)
    # x = R * np.outer(np.cos(u), np.sin(v)) # outer函数是用来求外积的
    # y = R * np.outer(np.sin(u), np.sin(v))
    # z = R * np.outer(np.ones(np.size(u)), np.cos(v))
    # # 就结果来看，x，y，z都是100*100的矩阵（一共10000个点），且满足x矩阵中的每一个元素Xi*Xi+Yi*Yi+Zi*Zi = R*R
    # # 所以可以理解为10000个点组成了一个球
    # ax.plot_surface(x, y, z, color='b')

    # 坐标信息，包括坐标长度和单位
    ax.set_xlim([-(r5 + 200), (r5 + 200)])
    ax.set_xlabel('X/km')
    ax.set_ylim([-(r5 + 200), (r5 + 200)])
    ax.set_ylabel('Y/km')
    ax.set_zlim([-(r5 + 200), (r5 + 200)])
    ax.set_zlabel('Z/km')

    omega1 = 2 * np.pi
    t_range = np.arange(0, 1 + 0.005, 0.005)
    t_len = len(t_range)
    # 画第x个高度的卫星轨道
    draw(32,inc1,r1,'purple',ax,omega1,t_range)
    draw(32,inc2,r2,'r',ax,omega1,t_range)
    draw(8,inc3,r3,'tomato',ax,omega1,t_range)
    draw(5,inc4,r4,'turquoise',ax,omega1,t_range)
    draw(6,inc5,r5,'chartreuse',ax,omega1,t_range)


    ani = animmation.FuncAnimation(f, update, frames=[i for i in range(len(position))], init_func=init, interval=20)

    # ani.save('planet.gif', writer='imagemagick', fps=40)
    plt.show()
