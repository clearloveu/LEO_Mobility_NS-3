import sys

# 读取ns3的tr文件
def read(readFile, src, d, startTime, endTime, srcIp):

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
    print("丢包数： "+str(diuBaoNum))
    delay2 = 0.0
    for i in range(len(delay)):
        delay2 += delay[i]
    print("平均延时： "+str(delay2/len(delay)))
    print(len(delay))

    f.close()


# 针对trace文件中，从s到d的延时在1-2s的情况（实验3）
def read2(readFile, src, d, startTime, endTime, srcIp):

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
        if(nodeId == src and each_line[0] == "+" and (ipS[0].strip() in srcIp )):
            # print(currentTime)
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 进入发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(len(sendTime) < 2):
                sendTime.append(currentTime)
            else:
                diuBaoNum += 1
                print(sendTime.pop(0))
                sendTime.append(currentTime)

        # if(each_line[0]=="-"):
        #     print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 离开发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length )
        if(nodeId == d and each_line[0]=="r" and (ipS[0].strip() in srcIp )):
            # print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " mac层接收到 " + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
            if(len(sendTime) >0):
                currentDelay = currentTime - sendTime.pop(0)
                delay.append(currentDelay)
            else:
                raise Exception("ERROR!!!! d连续收到两个消息")
    print("丢包数： "+str(diuBaoNum))
    delay2 = 0.0
    for i in range(len(delay)):
        delay2 += delay[i]
    print("平均延时： "+str(delay2/len(delay)))
    print(len(delay))
    print(delay)
    f.close()


if __name__ == '__main__':
    # fileName = "iridium-topology2-udpV3-3.tr"
    fileName = "iridium-SWS-3-2.tr"
    # fileName = "iridium-OLSR-3-3.tr"

    # 实验1
    # read2(fileName, "22", "1", 180.0, 190.0, ["10.1.23.1", "10.1.78.2", "10.1.89.1", "10.1.33.2"])  # s的ip地址：10.1.23.1 10.1.78.2

    # 实验2
    read2(fileName, "24", "1", 181.0, 201.0, ["10.1.24.2", "10.1.25.1", "10.1.91.1", "10.1.80.2"])  # s的ip地址：10.1.24.2

    # 实验3
    # read2(fileName, "5", "33", 728.0, 748.0, ["10.1.5.2", "10.1.6.1", "10.1.72.1"])  # s的ip地址：10.1.72.1


