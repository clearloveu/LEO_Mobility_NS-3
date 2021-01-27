import numpy as np
import matplotlib as mpl
mpl.use("TkAgg")
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animmation

"""
比iridiumConstellation2.py多了和左右卫星动态连接的过程
"""


def update(time):
    global satellite, text, figure, line
    for i in range(len(position[time])):
        lineTemp = satellite[i]
        # 更新卫星位置
        lineTemp.set_data(position[time][i][0], position[time][i][1])
        lineTemp.set_3d_properties(position[time][i][2])
        # 更新卫星注释位置
        # textSat = text[i]
        # textSat.remove()
        # textSat = ax.text(position[time][i][0], position[time][i][1], position[time][i][2], "satellite" + str(i))
        # text[i] = textSat

    # 更新连线
    for i in range(len(connection[time])):
        elegment = line[i]
        current_x, current_y, current_z = position[time][i][0], position[time][i][1], position[time][i][2]

        left_connection = elegment[0]
        right_connection = elegment[1]
        # 如果和左边卫星相连
        if(i>=11 and connection[time][i][0]):
            # 铱星每个轨道11个卫星，所以它左边的卫星的坐标是i-11
            left_x, left_y, left_z = position[time][i-11][0], position[time][i-11][1], position[time][i-11][2]
            left_connection.set_data([current_x, left_x], [current_y, left_y])
            left_connection.set_3d_properties([current_z, left_z])
        else:
            left_connection.set_data([], [])
            left_connection.set_3d_properties([])

        # 如果和右边卫星相连
        if(i<=54 and connection[time][i][1]):
            # 铱星每个轨道11个卫星，所以它右边的卫星的坐标是i+11
            right_x, right_y, right_z = position[time][i+11][0], position[time][i+11][1], position[time][i+11][2]
            right_connection.set_data([current_x, right_x], [current_y, right_y])
            right_connection.set_3d_properties([current_z, right_z])
        else:
            right_connection.set_data([], [])
            right_connection.set_3d_properties([])


    # print(time)
    # if(time==1):
    #     x = [0, 60000]
    #     y = [0, 60000]
    #     z = [0, 60000]
    #     # 将数组中的前两个点进行连线
    #     figure, = ax.plot(x, y, z, c='r')
    # if(time==2):
    #     figure.set_data([], [])
    #     figure.set_3d_properties([])



def init():
    global satellite, text, line
    satellite = []
    text = []
    line = []

    # 画卫星
    for i in range(len(position[0])):
        # 卫星的初始位置
        line3, = ax.plot([position[0][i][0]], [position[0][i][1]], [position[0][i][2]], marker='o', color='red', markersize=4)
        satellite.append(line3)
        # 卫星的注释
        # textSat = ax.text(position[0][i][0], position[0][i][1], position[0][i][2], "satellite"+str(i))
        # text.append(textSat)

    # 画连线
    for i in range(len(connection[0])):
        elegment = []
        current_x, current_y, current_z = position[0][i][0], position[0][i][1], position[0][i][2]
        # 如果和左边卫星相连
        if(i>=11 and connection[0][i][0]):
            # 铱星每个轨道11个卫星，所以它左边的卫星的坐标是i-11
            left_x, left_y, left_z = position[0][i-11][0], position[0][i-11][1], position[0][i-11][2]
            left_connection, = ax.plot([current_x, left_x], [current_y, left_y], [current_z, left_z], c='black')
        else:
            left_connection, = ax.plot([], [], [])
        elegment.append(left_connection)
        # 如果和右边卫星相连
        if(i<=54 and connection[0][i][1]):
            # 铱星每个轨道11个卫星，所以它右边的卫星的坐标是i+11
            right_x, right_y, right_z = position[0][i+11][0], position[0][i+11][1], position[0][i+11][2]
            right_connection, = ax.plot([current_x, right_x], [current_y, right_y], [current_z, right_z], c='black')
        else:
            right_connection, = ax.plot([], [], [])
        elegment.append(right_connection)
        line.append(elegment)





# 读取iridium-topology2.txt文件
def read(readFile):
    global time, position, connection
    f = open(readFile, "r")
    time = []
    position = [] # 结构：[某一个时刻下33个卫星的位置：[每一个卫星的位置，共33个：[],......],.....]
    connection = [] # 结构：[某一个时刻下33个卫星的位置：[每一个卫星和左右卫星是否连接的标识，共33个：[false,true],......],.....]
    index = 0
    for each_line in f:
        if(each_line[0:4]=="time"):
            # time.append(int(each_line[1:]))
            # position.append([])
            continue
        elif(each_line[0:5]!="false" and each_line[0:4]!="true"):
            if(index % 66 == 0): position.append([])
            x, y, z = each_line.split(" ")
            temp = []
            temp.append(float(x))
            temp.append(float(y))
            z = z.rstrip("\n")# 可能c尾部有\n，要去掉
            temp.append(float(z))
            position[-1].append(temp)
        else:
            if(index % 66 == 0): connection.append([])
            index += 1
            left, right, theta = each_line.split(" ")
            temp = []
            temp.append("true"==left)
            temp.append("true"==right)
            # theta暂时不用管
            # theta= theta.rstrip("\n")# 可能c尾部有\n，要去掉
            # temp.append(float(theta))
            connection[-1].append(temp)
    # print(len(position))
    # print(len(position[0]))
    # print(len(connection))
    # print(len(connection[0]))
    # print(position)
    f.close()


if __name__ == '__main__':

    read("iridium2-2020-12-21-2.txt")
    # read("iridium2-2020-7-5.txt")

    # 地球半径
    R = 6378
    # iridium卫星的半径r
    r = 6378 + 780
    # 倾斜角
    phi = 86.4 * np.pi / 180



    f = plt.figure(figsize=(12,12))
    ax = f.add_subplot(111, projection='3d')

    # equal表示x和y维度在数据坐标中的长度相同。要获取方轴，您可以手动设置纵横比：
    # ax.set_aspect(1./ax.get_data_ratio())
    # ax.set_aspect('equal')
    ax.set_title("Iiridium2 Model")

    # 地球模型
    # ax.plot([0], [0], [0], marker='o', color='blue', markersize=250)

    # u = np.linspace(0, 2 * np.pi, 100) # linspace的功能是创建等差数列。
    # v = np.linspace(0, np.pi, 100)
    # x = R * np.outer(np.cos(u), np.sin(v)) # outer函数是用来求外积的
    # y = R * np.outer(np.sin(u), np.sin(v))
    # z = R * np.outer(np.ones(np.size(u)), np.cos(v))
    # # 就结果来看，x，y，z都是100*100的矩阵（一共10000个点），且满足x矩阵中的每一个元素Xi*Xi+Yi*Yi+Zi*Zi = R*R
    # # 所以可以理解为10000个点组成了一个球
    # ax.plot_surface(x, y, z, color='b')

    # 坐标信息，包括坐标长度和单位
    ax.set_xlim([-(r + 200), (r + 200)])
    ax.set_xlabel('X/km')
    ax.set_ylim([-(r + 200), (r + 200)])
    ax.set_ylabel('Y/km')
    ax.set_zlim([-(r + 200), (r + 200)])
    ax.set_zlabel('Z/km')

    # 画6个卫星轨道
    omega1 = 2 * np.pi
    t_range = np.arange(0, 1 + 0.005, 0.005)
    t_len = len(t_range)
    x0 = r*np.cos(omega1*t_range)
    y0 = r*np.sin(omega1*t_range)
    z0 = 0
    x1 = x0
    y1 = np.cos(phi)*y0-np.sin(phi)*z0
    z1 = np.sin(phi)*y0+np.cos(phi)*z0
    dengcha = 180//6
    for i in range(6):
        shengjiaodian = dengcha*i*np.pi/180
        x21 = np.cos(shengjiaodian)*x1-np.sin(shengjiaodian)*y1
        y21 = np.sin(shengjiaodian)*x1+np.cos(shengjiaodian)*y1
        z21 = z1
        ax.plot(x21, y21, z21, 'purple')


    ani = animmation.FuncAnimation(f, update, frames=[i for i in range(len(position))], init_func=init, interval=20)

    # ani.save('Iiridium2.gif', writer='imagemagick')
    # ani.save('Iiridium2.gif', writer='imagemagick',fps=20)
    plt.show()
