


# double iridiumConstellation[3][11][4] = {{{0, 0, 0, 0}, {1, 0, 32.73, 0}, {2, 0, 65.45, 0}, {3, 0, 98.18, 0},
#                                           {4, 0, 130.91, 0}, {5, 0, 163.64, 0}, {6, 0, 196.36, 0},{7, 0, 229.09, 0},
#                                           {8, 0, 261.82, 0}, {9, 0, 294.55, 0}, {10, 0, 327.27, 0}},
#                                          {{11, 31.6, 16.36, 1}, {12, 31.6, 49.09, 1}, {13, 31.6, 81.82, 1}, {14, 31.6, 114.55, 1},
#                                           {15, 31.6, 147.27, 1}, {16, 31.6, 180, 1}, {17, 31.6, 212.73, 1}, {18, 31.6, 245.45, 1},
#                                           {19, 31.6, 278.18, 1}, {20, 0, 310.91, 1}, {21, 0, 343.64, 1}},
#                                          {{22, 63.2, 0, 2}, {23, 63.2, 32.73, 2}, {24, 63.2, 65.45, 2}, {24, 63.2, 98.18, 2},
#                                           {25, 63.2, 130.91, 2}, {26, 63.2, 163.64, 2}, {27, 63.2, 196.36, 2}, {28, 63.2, 229.09, 2},
#                                           {29, 63.2, 261.82, 2}, {30, 63.2, 294.55, 2}, {31, 63.2, 327.27, 2}}};

# 生成格式：[卫星的标识(从1开始),卫星的升交点赤经，距离近地点的角度，第几个轨道]
# 参数说明：nodenumber：卫星的标识  longitude：升交点赤经  plane：第几个轨道   n：轨道上的卫星数量
def generate(nodenumber, longitude, plane, n):
    res = ""
    dengcha = 360/n # 等差，让卫星的初始位置在轨道上平均分
    for i in range(n):
        if plane%2 == 0:
            res = res+"{"+str(nodenumber)+","+str(longitude)+","+str(round(dengcha*i, 2))+","+str(plane)+"}"+","
        else :
            res = res+"{"+str(nodenumber)+","+str(longitude)+","+str(round(dengcha*i + dengcha/2, 2))+","+str(plane)+"}"+","
        nodenumber += 1
    return res[:-1]

# 66颗卫星 ,轨道面：6个
def init(satellite_number, plane_number):
    dengcha = 180//plane_number # 等差，让每一个轨道的升交点赤经等分(pai-星座)
    res = "{"
    nodenumber = 0
    for i in range(plane_number):
        res = res+"{" + generate(nodenumber, dengcha*i, i, satellite_number//plane_number)+"}"+","
        nodenumber = nodenumber+satellite_number//plane_number
    return res[:-1]+"}"

if __name__ == '__main__':
    # ns3中的铱星
    #print(init(33, 3))

    # 铱星二代
    # 66颗卫星 倾角：86.4 工作轨道高度：780  轨道面：6个
    # print(init(66, 6))

    # OneWeb
    # 720颗将被发射到倾角为87.9°的1200km高度轨道，均匀分布在18个轨道面 ，每个轨道面工作星36颗，辅以4颗备份星
    # print(init(720, 18))

    # StarLink
    # 1600颗的轨道高度为1150km，轨道倾角为53°，分布于32个轨道面，每个轨道部署50颗卫星
    # print(init(1600, 32))
    # 1600颗的轨道高度为1110km，轨道倾角53.8°，分布于32个轨道，每个轨道部署50颗卫星
    # print(init(1600, 32))
    # 400 颗的轨道高度为 1130km，轨道倾角 74°，分布于 8 个轨道，每个轨道部署 50 颗卫星
    #print(init(400, 8))
    # 375 颗的轨道高度为 1275km，轨道倾角 81°，分布于 5 个轨道，每个轨道部署 75 颗卫星
    #print(init(375, 5))
    # 450 颗的轨道高度为 1325km，轨道倾角 70°，分布于 6 个轨道，每个轨道部署 75 颗卫星
    print(init(450, 6))
