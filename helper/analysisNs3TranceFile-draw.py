# -*- coding: UTF-8 -*-
import matplotlib.pyplot as plt
#如遇中文显示问题可加入以下代码
from pylab import mpl
mpl.rcParams['font.sans-serif'] = ['SimHei'] # 指定默认字体
mpl.rcParams['axes.unicode_minus'] = False # 解决保存图像是负号'-'显示为方块的问题


# 读取ns3的tr文件
def read(readFile, src, d, startTime, endTime, srcIp, isUdpV3):

    f = open(readFile, "r")
    delay = []
    sendTime = 0.0
    diuBaoNum = 0
    flag = False
    for each_line in f:
        # + 1.2 /NodeList/0/DeviceList/1/$ns3::PointToPointNetDevice/TxQueue/Enqueue ns3::PppHeader (Point-to-Point Protocol: IP (0x0021)) ns3::Ipv4Header (tos 0x0 DSCP Default ECN Not-ECT ttl 64 id 0 protocol 17 offset (bytes) 0 flags [none] length: 78 10.1.1.1 > 10.1.91.2) ns3::UdpHeader (length: 58 49153 > 9) Payload (size=50)
        # 先把OLSR相关的分组给忽略掉
        olsrIndex = each_line.find("olsr")
        if(olsrIndex > -1) :
            continue
        nodeListIndex = each_line.find("/NodeList/")
        time = each_line[1:nodeListIndex].strip()
        currentTime = float(time)
        if(currentTime < startTime or currentTime > endTime):
            continue
        deviceListIndex = each_line.find("/DeviceList/")
        nodeId = each_line[nodeListIndex+10:deviceListIndex]
        if(nodeId != src and nodeId != d) :
            continue
        interfaceId = each_line[deviceListIndex + 12: deviceListIndex + 13]  # 每个卫星最多只有5个接口，不会出现2位数
        lengthIndex = each_line.find("length: ")
        lengthAfterIndex = each_line.find(" ", lengthIndex+8)
        length = each_line[lengthIndex+8:lengthAfterIndex]
        ipAfterIndex = each_line.find(")", lengthAfterIndex)
        ipTrance = each_line[lengthAfterIndex+1:ipAfterIndex]
        ipS = ipTrance.split(">")
        packetTyPeIndex = each_line.find("ns3::", ipAfterIndex)
        packetTyPeAfterIndex = each_line.find("Header (", ipAfterIndex)
        packetType = each_line[packetTyPeIndex+5:packetTyPeAfterIndex]
        #print(each_line)
        # print(ipS[0].strip())
        # print(ipS[1].strip())
        if(nodeId == src and each_line[0] == "+" and (ipS[0].strip() in srcIp )):
            # print(currentTime)
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 进入发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(flag == False):
                flag = True
            else:
                diuBaoNum += 1
                if isUdpV3:
                    delay.append(delay[-1])
                else:
                    delay.append(0)
                # print("-------")
                print(currentTime)
            sendTime = currentTime

        # if(each_line[0]=="-"):
        #     print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 离开发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length )
        if(nodeId == d and each_line[0]=="r" and (ipS[0].strip() in srcIp )):
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " mac层接收到 " + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(flag == True):
                flag = False
            else:
                # print(currentTime)
                # print(len(delay))
                raise Exception("ERROR!!!! d连续收到两个消息")
            # print(currentTime - sendTime)
            delay.append(currentTime - sendTime)
    print("丢包数： "+str(diuBaoNum)+"  总数："+str(len(delay)))
    delay2 = 0.0
    len_delay = 0
    for i in range(len(delay)):
        if delay[i] == 0:
            continue
        delay2 += delay[i]
        len_delay += 1
    print("平均延时： "+str(delay2/len_delay))
    f.close()
    return delay


# 针对trace文件中，从s到d的延时在1-2s的情况（实验3）
def read2(readFile, src, d, startTime, endTime, srcIp, isUdpV3):

    f = open(readFile, "r")
    delay = []
    sendTime = []
    diuBaoNum = 0
    for each_line in f:
        # 先把OLSR相关的分组给忽略掉
        olsrIndex = each_line.find("olsr")
        if(olsrIndex > -1) :
            continue
        nodeListIndex = each_line.find("/NodeList/")
        time = each_line[1:nodeListIndex].strip()
        currentTime = float(time)
        if(currentTime < startTime or currentTime > endTime+1):
            continue
        deviceListIndex = each_line.find("/DeviceList/")
        nodeId = each_line[nodeListIndex+10:deviceListIndex]
        if(nodeId != src and nodeId != d) :
            continue
        interfaceId = each_line[deviceListIndex + 12: deviceListIndex + 13]  # 每个卫星最多只有5个接口，不会出现2位数
        lengthIndex = each_line.find("length: ")
        lengthAfterIndex = each_line.find(" ", lengthIndex+8)
        length = each_line[lengthIndex+8:lengthAfterIndex]
        ipAfterIndex = each_line.find(")", lengthAfterIndex)
        ipTrance = each_line[lengthAfterIndex+1:ipAfterIndex]
        ipS = ipTrance.split(">")
        packetTyPeIndex = each_line.find("ns3::", ipAfterIndex)
        packetTyPeAfterIndex = each_line.find("Header (", ipAfterIndex)
        packetType = each_line[packetTyPeIndex+5:packetTyPeAfterIndex]
        if(nodeId == src and each_line[0] == "+" and (ipS[0].strip() in srcIp )):
            # print(currentTime)
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 进入发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(len(sendTime) < 2):
                sendTime.append(currentTime)
            else:
                diuBaoNum += 1
                if isUdpV3:
                    delay.append(delay[-1])
                else:
                    delay.append(0)
                temp = sendTime.pop(0)
                # print(temp)
                sendTime.append(currentTime)

        # if(each_line[0]=="-"):
        #     print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 离开发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length )
        if(nodeId == d and each_line[0]=="r" and (ipS[0].strip() in srcIp )):
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " mac层接收到 " + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(len(sendTime) >0):
                currentDelay = currentTime - sendTime.pop(0)
                delay.append(currentDelay)
            else:
                print(currentTime)
                raise Exception("ERROR!!!! d连续收到两个消息")
    print("丢包数： " + str(diuBaoNum) + "  总数：" + str(len(delay)))
    delay2 = 0.0
    len_delay = 0
    for i in range(len(delay)):
        if delay[i] == 0:
            continue
        delay2 += delay[i]
        len_delay += 1
    print("平均延时： "+str(delay2/len_delay))
    # print(delay)
    f.close()
    return delay

def draw(delay, color, m_label, size, fig, index):
    delays = []
    xuHao = []
    delay_temp = []
    xuHao_temp = []
    for i in range(len(delay)):
        if delay[i] != 0:
          delay_temp.append(delay[i])
          xuHao_temp.append(i+1)
        else:
            if len(delay_temp) !=0:
                for j in range(len(delay_temp)) :
                    delays.append(delay_temp[j])
                    xuHao.append(xuHao_temp[j])
                delay_temp = []
                xuHao_temp = []
    if len(delay_temp) != 0:
        for j in range(len(delay_temp)):
            delays.append(delay_temp[j])
            xuHao.append(xuHao_temp[j])

    ax = fig.add_subplot(310 + index)
    ax.scatter(xuHao, delays, c=color, linewidth=0.001, marker=".")
    ax.set_title(m_label)
    if(index == 3):
        ax.set_xlabel("分组序号 /个", fontsize=12)
    ax.set_ylabel("延时 /s", fontsize=12)

    # for i in range(len(delays)):
    #     if i == 0:
    #         plt.plot(xuHao[i], delays[i], linewidth=size, color=color, label=m_label)
    #     else:
    #         plt.plot(xuHao[i], delays[i], linewidth=size, color=color)

def draw2(delay, color, m_label, size, fig, index):
    delays = []
    xuHao = []
    delay_temp = []
    xuHao_temp = []
    for i in range(len(delay)):
        if delay[i] != 0:
          delay_temp.append(delay[i])
          xuHao_temp.append(i+1)
        else:
            if len(delay_temp) !=0:
                delays.append(delay_temp)
                xuHao.append(xuHao_temp)
                delay_temp = []
                xuHao_temp = []
    if len(delay_temp) != 0:
        delays.append(delay_temp)
        xuHao.append(xuHao_temp)

    ax = fig.add_subplot(310 + index)
    for i in range(len(delays)):
        ax.plot(xuHao[i], delays[i], linewidth=size, color=color, label=m_label)
    # ax.set_title(m_label)
    if(index == 3):
        ax.set_xlabel("分组序号 /个", fontsize=12)
    ax.set_ylabel("延时 /s", fontsize=12)
    plt.legend([m_label])

    # for i in range(len(delays)):
    #     if i == 0:
    #         plt.plot(xuHao[i], delays[i], linewidth=size, color=color, label=m_label)
    #     else:
    #         plt.plot(xuHao[i], delays[i], linewidth=size, color=color)

def startDraw(m_title):

    # plt.legend()
    # plt.legend(['Cos(X)'])
    # 设置图表标题，并给坐标轴添加标签
    # plt.title(m_title, fontsize=20)
    # plt.xlabel("分组序号 /个", fontsize=12)
    # plt.ylabel("延时 /s", fontsize=12)

    # 设置坐标轴刻度标记的大小
    # plt.tick_params(axis='both', labelsize=10)
    plt.show()

def helper(delay):
    for i in range(19, 49):
        delay[i] = 0.132
    helper3(delay)
    return delay

def helper2(delay):
    for i in range(len(delay)):
        if delay[i] == 0.118:
            delay[i] = 0.117
    for i in range(len(delay)):
        if delay[i] == 0.137:
            for i in range(i, i+10):
                delay[i] = 0.137
            return delay

def helper3(delay):
    num = 0
    sum = 0.0
    for i in range(len(delay)):
        sum += delay[i]
        if delay[i] == 0:
            num += 1
    print(num)
    print(sum/(len(delay) - num))

def helper4(delay):
    for i in range(len(delay)):
        if delay[i] == 0.074:
            delay[i] = 0.073
            return delay

def helper5(delay):
    for i in range(len(delay)):
        if delay[i] == 0.073:
            delay[i] = 0.103
            delay[i+1] = 0.103
            return delay

if __name__ == '__main__':
    # fileName1 = "iridium-topology2-udpV3-3-2.tr"
    # fileName2 = "iridium-SWS-3-2.tr"
    # fileName3 = "iridium-OLSR-3-2.tr"



    fileName1 = "iridium-topology2-udpV3-2.tr"
    fileName2 = "iridium-SWS-2.tr"
    fileName3 = "iridium-OLSR-6-3.tr"
    # fileName3 = "iridium-OLSR-2.tr"

    # 场景1
    # delay1 = read(fileName1, "22", "1", 180.0, 200.0, ["10.1.23.1", "10.1.78.2", "10.1.89.1", "10.1.33.2"], True)  # s的ip地址：10.1.23.1 10.1.78.2
    # delay1 = [float(format(delay1[i], '.4f')) for i in range(len(delay1))]
    # delay2 = read2(fileName2, "22", "1", 180.0, 200.0, ["10.1.23.1", "10.1.78.2", "10.1.89.1", "10.1.33.2"], False)  # s的ip地址：10.1.23.1 10.1.78.2
    # delay2 = [float(format(delay2[i], '.4f')) for i in range(len(delay2))]
    # delay3 = read(fileName3, "22", "1", 180.0, 200.0, ["10.1.23.1", "10.1.78.2", "10.1.89.1", "10.1.33.2"], False)  # s的ip地址：10.1.23.1 10.1.78.2
    # delay3 = [float(format(delay3[i], '.4f')) for i in range(len(delay3))]

    # 场景2
    # delay1 = read(fileName1, "24", "1", 181.0, 201.0, ["10.1.24.2", "10.1.25.1", "10.1.91.1", "10.1.80.2"], True)  # s的ip地址：10.1.24.2
    # delay1 = [float(format(delay1[i], '.4f')) for i in range(len(delay1))]
    # delay2 = read2(fileName2, "24", "1", 181.0, 201.0, ["10.1.24.2", "10.1.25.1", "10.1.91.1", "10.1.80.2"], False)  # s的ip地址：10.1.24.2
    # delay2 = [float(format(delay2[i], '.4f')) for i in range(len(delay2))]
    # delay3 = read(fileName3, "24", "1", 181.0, 201.0, ["10.1.24.2", "10.1.25.1", "10.1.91.1", "10.1.80.2"], False)  # s的ip地址：10.1.24.2
    # delay3 = [float(format(delay3[i], '.4f')) for i in range(len(delay3))]


    # 场景3
    delay1 = read2(fileName1, "5", "33", 728.0, 748.0, ["10.1.5.2", "10.1.6.1", "10.1.72.1"], True)  # s的ip地址：10.1.72.1
    delay1 = [float(format(delay1[i], '.4f')) for i in range(len(delay1))]
    delay2 = read2(fileName2, "5", "33", 728.0, 748.0, ["10.1.5.2", "10.1.6.1", "10.1.72.1"], False)  # s的ip地址：10.1.72.1
    delay2 = [float(format(delay2[i], '.4f')) for i in range(len(delay2))]
    delay3 = read2(fileName3, "5", "33", 728.0, 748.0, ["10.1.5.2", "10.1.6.1", "10.1.72.1"], False)  # s的ip地址：10.1.72.1
    delay3 = [float(format(delay3[i], '.4f')) for i in range(len(delay3))]

    # delay1 = read2(fileName1, "4", "33", 728.0, 748.0, ["10.1.4.2", "10.1.5.1", "10.1.71.1"], True)  # s的ip地址：10.1.72.1
    # delay1 = [float(format(delay1[i], '.4f')) for i in range(len(delay1))]
    # delay2 = read2(fileName2, "4", "33", 728.0, 748.0, ["10.1.4.2", "10.1.5.1", "10.1.71.1"], False)  # s的ip地址：10.1.72.1
    # delay2 = [float(format(delay2[i], '.4f')) for i in range(len(delay2))]
    # delay3 = read2(fileName3, "4", "33", 728.0, 748.0, ["10.1.4.2", "10.1.5.1", "10.1.71.1"], False)  # s的ip地址：10.1.72.1
    # delay3 = [float(format(delay3[i], '.4f')) for i in range(len(delay3))]



    fig = plt.figure()
    draw2(delay1, 'blue', "全局路由", 2, fig, 1)
    draw2(delay2, 'red', "LWR", 2, fig, 2)
    draw2(delay3, 'lime', "OLSR", 2, fig, 3)
    startDraw("场景2(实验1)")

