import sys

# 读取ns3的tr文件
def read(readFile):

    f = open(readFile, "r")
    
    for each_line in f:
        # + 1.2 /NodeList/0/DeviceList/1/$ns3::PointToPointNetDevice/TxQueue/Enqueue ns3::PppHeader (Point-to-Point Protocol: IP (0x0021)) ns3::Ipv4Header (tos 0x0 DSCP Default ECN Not-ECT ttl 64 id 0 protocol 17 offset (bytes) 0 flags [none] length: 78 10.1.1.1 > 10.1.91.2) ns3::UdpHeader (length: 58 49153 > 9) Payload (size=50)
        nodeListIndex = each_line.find("/NodeList/")
        time = each_line[1:nodeListIndex].strip()
        deviceListIndex = each_line.find("/DeviceList/")
        nodeId = each_line[nodeListIndex+10:deviceListIndex]
        interfaceId = each_line[deviceListIndex+12:deviceListIndex+13] # 每个卫星最多只有5个接口，不会出现2位数
        lengthIndex = each_line.find("length: ")
        lengthAfterIndex = each_line.find(" ", lengthIndex+8)
        length = each_line[lengthIndex+8:lengthAfterIndex]
        ipAfterIndex = each_line.find(")", lengthAfterIndex)
        ipTrance = each_line[lengthAfterIndex+1:ipAfterIndex]
        packetTyPeIndex = each_line.find("ns3::", ipAfterIndex)
        packetTyPeAfterIndex = each_line.find("Header (", ipAfterIndex)
        packetType = each_line[packetTyPeIndex+5:packetTyPeAfterIndex]
        #print(each_line)
        if(each_line[0]=="+"):
            print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 进入发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
        elif(each_line[0]=="-"):
            print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " 离开发送缓存" + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length )
        elif(each_line[0]=="r"):
            print("time: " + time + " 节点: " + nodeId + " 接口id: " + interfaceId + " mac层接收到 " + " ip: "+ ipTrance + " 数据包类型: " + packetType + " 分组长度: " + length)
    f.close()


if __name__ == '__main__':
    argv = sys.argv[1]
    fileName = "iridium-topology2-udpV3.tr"
    if (argv == "1"):
        fileName = "iridium-topology2-udpV3.tr"
    elif (argv == "2"):
        fileName = "iridium-nix-route.tr"
    elif (argv == "3"):
        fileName = "iridium-nix-segment-routing.tr"
    elif (argv == "4"):
        fileName = "iridium-nix-segment-routing2.tr"
    elif (argv == "5"):
        fileName = "iridium-SWS.tr"
    print("read fileName" + fileName)
    read(fileName)




