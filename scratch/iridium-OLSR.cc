#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include <string>
#include <fstream>
#include <string>
#include <cmath>

#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/olsr-helper.h"

#define PI 3.1415926535897

using namespace ns3;

void
newSnapshotWay(NodeContainer nodes, double simulatorLastTime, double T){
	std::set<double> s;
	for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j){
	      Ptr<Node> object = *j;
	      Ptr<LEOSatelliteMobilityModel> mobility = object->GetObject<LEOSatelliteMobilityModel> ();
	      double enterNorthPoleTime = mobility->m_helper.getEnterNorthPoleTime();
	      while (enterNorthPoleTime <= simulatorLastTime) {
	    	  s.insert(enterNorthPoleTime);
	    	  enterNorthPoleTime = enterNorthPoleTime + T;
	      }
	      double enterSorthPoleTime = mobility->m_helper.getEnterSorthPoleTime();
	      while (enterSorthPoleTime <= simulatorLastTime) {
	    	  s.insert(enterSorthPoleTime);
	    	  enterSorthPoleTime = enterSorthPoleTime + T;
	      }
	      double leaveNorthPoleTime = mobility->m_helper.getLeaveNorthPoleTime();
		  while (leaveNorthPoleTime <= simulatorLastTime) {
			  s.insert(leaveNorthPoleTime);
			  leaveNorthPoleTime = leaveNorthPoleTime + T;
		  }
		  double leaveSorthPoleTime = mobility->m_helper.getLeaveSorthPoleTime();
		  while (leaveSorthPoleTime <= simulatorLastTime) {
			  s.insert(leaveSorthPoleTime);
			  leaveSorthPoleTime = leaveSorthPoleTime + T;
		  }
	}
	std::cout<<"newSnapshotWay: set.size= "<< s.size() << std::endl;
	std::set<double>::iterator iter;
    for(iter = s.begin() ; iter != s.end() ; ++iter){
         std::cout<<*iter<<" ";
     }
    std::cout<<std::endl;
	for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j){
	      Ptr<Node> object = *j;
	      Ptr<LEOSatelliteMobilityModel> mobility = object->GetObject<LEOSatelliteMobilityModel> ();
	      for(iter = s.begin() ; iter != s.end() ; ++iter){
	 		  Simulator::Schedule (Seconds (*iter), &LEOSatelliteMobilityModel::UpdatePosition, mobility);
	       }
	}
}



void
Progress (NodeContainer nodes, int timeInterval)
{
   std::cout << "时间: "<< Simulator::Now ().GetMilliSeconds() << "ms  Progress" << std::endl;

	Ptr<Node> node_1 = nodes.Get(1);
	Ptr<LEOSatelliteMobilityModel> node_1Mobility = node_1->GetObject<LEOSatelliteMobilityModel> ();
	std::cout << "node1连接: "<< std::boolalpha <<node_1Mobility->m_helper.m_pos.isRightSatelliteConnection << std::endl;

	Ptr<Node> node_12 = nodes.Get(12);
	Ptr<LEOSatelliteMobilityModel> node_12Mobility = node_12->GetObject<LEOSatelliteMobilityModel> ();
	std::cout << "node12连接: "<< std::boolalpha <<node_12Mobility->m_helper.m_pos.isLeftSatelliteConnection << std::endl;
//    Simulator::Schedule (Seconds (timeInterval), Progress, nodes, timeInterval);
}

void
setFaultSatellite (Ptr<Node> node, bool flag)
{
    std::cout << "时间: "<< Simulator::Now ().GetMilliSeconds() << "  setFaultSatellite" << std::endl;
	Ptr<LEOSatelliteMobilityModel> node_position = node->GetObject<LEOSatelliteMobilityModel> ();
	node_position->m_helper.setIsFault(flag); // @suppress("Invalid arguments")
}



int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  Time::SetResolution (Time::NS);
  //必须有下面2个语句，才会打印udp的信息，默认是info级别的
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);


  // 当前开始时间
  time_t t = time(0);
  char tmp[64];
  strftime( tmp, sizeof(tmp), "%Y/%m/%d %X",localtime(&t) );
  puts( tmp );


  const double ALTITUDE = 780;//卫星海拔
  const double INCLINATION = 86.4;//倾角
  // Format: nodenumber, longitude（升交点赤经，角度制）, alpha(距离近地点的角度，角度制，因为是圆轨道，没有近地点，这里假设近地点是地球正面的赤道) and plane(轨道编号).
//  double iridiumConstellation[6][11][4] = {{{0,0,0.0,0},{1,0,32.73,0},{2,0,65.45,0},{3,0,98.18,0},{4,0,130.91,0},{5,0,163.64,0},{6,0,196.36,0},{7,0,229.09,0},{8,0,261.82,0},{9,0,294.55,0},{10,0,327.27,0}},{{11,30,0.0,1},{12,30,32.73,1},{13,30,65.45,1},{14,30,98.18,1},{15,30,130.91,1},{16,30,163.64,1},{17,30,196.36,1},{18,30,229.09,1},{19,30,261.82,1},{20,30,294.55,1},{21,30,327.27,1}},{{22,60,0.0,2},{23,60,32.73,2},{24,60,65.45,2},{25,60,98.18,2},{26,60,130.91,2},{27,60,163.64,2},{28,60,196.36,2},{29,60,229.09,2},{30,60,261.82,2},{31,60,294.55,2},{32,60,327.27,2}},{{33,90,0.0,3},{34,90,32.73,3},{35,90,65.45,3},{36,90,98.18,3},{37,90,130.91,3},{38,90,163.64,3},{39,90,196.36,3},{40,90,229.09,3},{41,90,261.82,3},{42,90,294.55,3},{43,90,327.27,3}},{{44,120,0.0,4},{45,120,32.73,4},{46,120,65.45,4},{47,120,98.18,4},{48,120,130.91,4},{49,120,163.64,4},{50,120,196.36,4},{51,120,229.09,4},{52,120,261.82,4},{53,120,294.55,4},{54,120,327.27,4}},{{55,150,0.0,5},{56,150,32.73,5},{57,150,65.45,5},{58,150,98.18,5},{59,150,130.91,5},{60,150,163.64,5},{61,150,196.36,5},{62,150,229.09,5},{63,150,261.82,5},{64,150,294.55,5},{65,150,327.27,5}}};
  double iridiumConstellation[6][11][4] = {{{0,0,0.0,0},{1,0,32.73,0},{2,0,65.45,0},{3,0,98.18,0},{4,0,130.91,0},{5,0,163.64,0},{6,0,196.36,0},{7,0,229.09,0},{8,0,261.82,0},{9,0,294.55,0},{10,0,327.27,0}},{{11,30,16.36,1},{12,30,49.09,1},{13,30,81.82,1},{14,30,114.55,1},{15,30,147.27,1},{16,30,180.0,1},{17,30,212.73,1},{18,30,245.45,1},{19,30,278.18,1},{20,30,310.91,1},{21,30,343.64,1}},{{22,60,0.0,2},{23,60,32.73,2},{24,60,65.45,2},{25,60,98.18,2},{26,60,130.91,2},{27,60,163.64,2},{28,60,196.36,2},{29,60,229.09,2},{30,60,261.82,2},{31,60,294.55,2},{32,60,327.27,2}},{{33,90,16.36,3},{34,90,49.09,3},{35,90,81.82,3},{36,90,114.55,3},{37,90,147.27,3},{38,90,180.0,3},{39,90,212.73,3},{40,90,245.45,3},{41,90,278.18,3},{42,90,310.91,3},{43,90,343.64,3}},{{44,120,0.0,4},{45,120,32.73,4},{46,120,65.45,4},{47,120,98.18,4},{48,120,130.91,4},{49,120,163.64,4},{50,120,196.36,4},{51,120,229.09,4},{52,120,261.82,4},{53,120,294.55,4},{54,120,327.27,4}},{{55,150,16.36,5},{56,150,49.09,5},{57,150,81.82,5},{58,150,114.55,5},{59,150,147.27,5},{60,150,180.0,5},{61,150,212.73,5},{62,150,245.45,5},{63,150,278.18,5},{64,150,310.91,5},{65,150,343.64,5}}};
  NodeContainer nodes;
  nodes.Create (66);


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::LEOSatelliteMobilityModel");

  mobility.InstallAll ();

  int index = 0;
  // iterate our nodes and print their position.
  for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j){
      Ptr<Node> object = *j;
      Ptr<LEOSatelliteMobilityModel> position = object->GetObject<LEOSatelliteMobilityModel> ();
      // 设置每个卫星的初始位置
      struct LEOSatPolarPos pPos;
      pPos.altitude = ALTITUDE;
      pPos.longitude =  iridiumConstellation[index/11][index%11][1];
      pPos.alpha =  iridiumConstellation[index/11][index%11][2];
      pPos.inclination =  INCLINATION;
      pPos.plane =  iridiumConstellation[index/11][index%11][3];
      pPos.self = nodes.Get(index);
      pPos.index = index;
      pPos.planeNum = 6;
      pPos.satelliteNumInOnePlane = 11;// @suppress("Field cannot be resolved")
      // 设置每个卫星的相邻卫星节点
      // 同轨道的上下节点
      if(index%11 == 0){
    	  pPos.before = nodes.Get(index+1);
    	  pPos.after = nodes.Get(index+10);
      }else if(index%11 == 10){
    	  pPos.before = nodes.Get(index-10);
    	  pPos.after = nodes.Get(index-1);
      }else{
    	  pPos.before = nodes.Get(index+1);
    	  pPos.after = nodes.Get(index-1);
      }
      // 不同轨道的左右节点
      if(index/11 == 0){
    	  // 最左边的轨道，这里的卫星的左边卫星是在seam的另一侧，所以它们的left为空
    	  pPos.left = NULL;
    	  pPos.right = nodes.Get(index+11);
      }else if(index/11 == 5){
    	  // 最右边的轨道，这里的卫星的右边卫星是在seam的另一侧，所以它们的right为空
    	  pPos.left = nodes.Get(index-11);
    	  pPos.right = NULL;
      }else{
    	  pPos.left = nodes.Get(index-11);
    	  pPos.right = nodes.Get(index+11);
      }

      position->SetSatSphericalPos(pPos); // @suppress("Invalid arguments")
      position->setRoutingAlgorithmAndSnapShotWay(3, 2); // @suppress("Invalid arguments")
      NS_ASSERT (position != 0);
      index++;

    }



  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (nodes);


  index = 0;
  int ipIndex = 1;
  PointToPointHelper p2p;
  Ipv4AddressHelper ipv4;
  Ipv4InterfaceContainer ipv4InterfaceContainer[121];
  while(index<66){
	  int nextNodeIndex;
	  if(index%11 != 10){
		  nextNodeIndex = index+1;
	  } else {
		  nextNodeIndex = index-10;
	  }
	  NodeContainer nodeContainer = NodeContainer (nodes.Get (index), nodes.Get (nextNodeIndex));
	  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	  p2p.SetChannelAttribute ("Delay", StringValue ("13ms"));
	  NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);
	  std::string ip = "10.1." + std::to_string(ipIndex) + ".0";
	  const char* ipp = ip.c_str();
	  ipv4.SetBase (ipp, "255.255.255.0");
	  Ipv4InterfaceContainer ipv4Container = ipv4.Assign (netDeviceContainer);
	  ipv4InterfaceContainer[ipIndex-1] = ipv4Container;
	  index++;
	  ipIndex++;
  }
  index = 0;
  while(index<55){
	  NodeContainer nodeContainer = NodeContainer (nodes.Get (index), nodes.Get (index+11));

	  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	  p2p.SetChannelAttribute ("Delay", StringValue ("13ms"));
	  NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);

	  std::string ip = "10.1." + std::to_string(ipIndex) + ".0";
	  const char* ipp = ip.c_str();
	  ipv4.SetBase (ipp, "255.255.255.0");
	  Ipv4InterfaceContainer ipv4Container = ipv4.Assign (netDeviceContainer);
	  ipv4InterfaceContainer[ipIndex-1] = ipv4Container;
	  index++;
	  ipIndex++;
  }

  //  Ipv4InterfaceContainer i0i1 = ipv4InterfaceContainer[0];//卫星编号0和1之间的链路的ip分配
    Ipv4InterfaceContainer i1i2 = ipv4InterfaceContainer[1];//卫星编号1和2之间的链路的ip分配
    Ipv4InterfaceContainer i11i12 = ipv4InterfaceContainer[11];//卫星编号11和12之间的链路的ip分配
    Ipv4InterfaceContainer i12i13 = ipv4InterfaceContainer[12];//卫星编号12和13之间的链路的ip分配
    Ipv4InterfaceContainer i23i34 = ipv4InterfaceContainer[90];//卫星编号23和34之间的链路的ip分配
    Ipv4InterfaceContainer i0i11 = ipv4InterfaceContainer[66];//卫星编号0和11之间的链路的ip分配
    Ipv4InterfaceContainer i1i12 = ipv4InterfaceContainer[67];//卫星编号1和12之间的链路的ip分配
    Ipv4InterfaceContainer i2i13 = ipv4InterfaceContainer[68];//卫星编号2和13之间的链路的ip分配
    Ipv4InterfaceContainer i33i34 = ipv4InterfaceContainer[33];//卫星编号33和34之间的链路的ip分配

    // 实验1：s=22，d=1，t=186.22
    Ptr<Node> source = nodes.Get(22);
    Ptr<Node> dest = nodes.Get(1);
    Ipv4Address destinationIp = i1i2.GetAddress (0);
    double serverStartTime = 170.0;
    double serverEndTime = 215.0;
    double clientStartTime = 180.0;
    double clientEndTime = 215.0;
    double interval = 0.1;

    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (dest);
    serverApps.Start (Seconds (serverStartTime));
    serverApps.Stop (Seconds (serverEndTime));

    UdpEchoClientHelper echoClient (destinationIp, 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (200));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (interval)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (source);
    clientApps.Start (Seconds (clientStartTime));
    clientApps.Stop (Seconds (clientEndTime));

  	// 航点发生故障
    Simulator::Schedule (Seconds (184.72), setFaultSatellite, nodes.Get(0), true);
    Simulator::Schedule (Seconds (186.00), setFaultSatellite, nodes.Get(0), false);
    //上面设置的注意点：这里设置时间时，注意如果故障卫星恢复的时间在将断链路断开的时间之后，则会转变成非航点卫星故障，而非航点卫星故障并不一定能找到新的航点，即不一定有新路径，如果找不到，程序会报错（后期解决）



//    // 实验2：s=24，d=1，t=186.22
//    Ptr<Node> source2 = nodes.Get(24);
//    Ptr<Node> dest2 = nodes.Get(1);
//    Ipv4Address destinationIp2 = i1i2.GetAddress (0);
//    double serverStartTime2 = 170.0;
//    double serverEndTime2 = 215.0;
//    double clientStartTime2 = 181.0;
//    double clientEndTime2 = 215.0;
//    double interval2 = 0.1;
//
//    UdpEchoServerHelper echoServer2 (9);
//    ApplicationContainer serverApps2 = echoServer2.Install (dest2);
//    serverApps2.Start (Seconds (serverStartTime2));
//    serverApps2.Stop (Seconds (serverEndTime2));
//
//    UdpEchoClientHelper echoClient2 (destinationIp2, 9);
//    echoClient2.SetAttribute ("MaxPackets", UintegerValue (200));
//    echoClient2.SetAttribute ("Interval", TimeValue (Seconds (interval2)));
//    echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));
//
//    ApplicationContainer clientApps2 = echoClient2.Install (source2);
//    clientApps2.Start (Seconds (clientStartTime2));
//    clientApps2.Stop (Seconds (clientEndTime2));


//    // 实验3：s=6，d=33，t=734.17
//    Ptr<Node> source3 = nodes.Get(5);
//    Ptr<Node> dest3 = nodes.Get(33);
//    Ipv4Address destinationIp3 = i33i34.GetAddress (0);
//    double serverStartTime3 = 720.0;
//    double serverEndTime3 = 800.0;
//    double clientStartTime3 = 728.0;
//    double clientEndTime3 = 800.0;
//    double interval3 = 0.1;  //
//
//    UdpEchoServerHelper echoServer3 (9);
//    ApplicationContainer serverApps3 = echoServer3.Install (dest3);
//    serverApps3.Start (Seconds (serverStartTime3));
//    serverApps3.Stop (Seconds (serverEndTime3));
//
//    UdpEchoClientHelper echoClient3 (destinationIp3, 9);
//    echoClient3.SetAttribute ("MaxPackets", UintegerValue (200)); //
//    echoClient3.SetAttribute ("Interval", TimeValue (Seconds (interval3)));
//    echoClient3.SetAttribute ("PacketSize", UintegerValue (1024));
//
//    ApplicationContainer clientApps3 = echoClient3.Install (source3);
//    clientApps3.Start (Seconds (clientStartTime3));
//    clientApps3.Stop (Seconds (clientEndTime3));

    // 非航点发生故障
//    Simulator::Schedule (Seconds (728.85), setFaultSatellite, nodes.Get(11), true);
//    Simulator::Schedule (Seconds (729.85), setFaultSatellite, nodes.Get(11), false);
//    Simulator::Schedule (Seconds (730.85), setFaultSatellite, nodes.Get(21), true);
//    Simulator::Schedule (Seconds (732.85), setFaultSatellite, nodes.Get(21), false);
//  	// 航点发生故障
//  	Simulator::Schedule (Seconds (731.85), setFaultSatellite, nodes.Get(10), true);
//  	Simulator::Schedule (Seconds (733.85), setFaultSatellite, nodes.Get(10), false);


    double simulatorLastTime = 800.0;
    newSnapshotWay(nodes, simulatorLastTime, 6026.957216098);
    Simulator::Stop (Seconds (simulatorLastTime));


  AsciiTraceHelper ascii;
//  p2p.EnableAsciiAll (ascii.CreateFileStream ("iridium-OLSR.tr"));
//  p2p.EnablePcapAll ("iridium-OLSR");
  NodeContainer node1_12;
  node1_12.Add(nodes.Get(1));
  node1_12.Add(nodes.Get(22));
  node1_12.Add(nodes.Get(24));
  node1_12.Add(nodes.Get(5));
  node1_12.Add(nodes.Get(33));
  p2p.EnableAscii (ascii.CreateFileStream ("iridium-OLSR.tr"), node1_12);
//  p2p.EnablePcap ("iridium-OLSR", node1_12);
  p2p.EnablePcapAll ("iridium-OLSR");

//  Simulator::Schedule (Seconds (187), Progress,nodes,3);

  Simulator::Run ();
  Simulator::Destroy ();

  // 当前结束时间
  t = time(0);
  strftime( tmp, sizeof(tmp), "%Y/%m/%d %X" ,localtime(&t) );
  puts( tmp );
}
