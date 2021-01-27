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
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-nix-vector-routing.h"

#define PI 3.1415926535897

using namespace ns3;

static void
CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
{

//  Vector pos = mobility->GetPosition ();
//  std::cout << Simulator::Now () << ", POS: x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z <<std::endl;
//  Ptr<const ns3::LEOSatelliteMobilityModel> pd2 = static_cast<Ptr<const ns3::LEOSatelliteMobilityModel>>(mobility);//父类不能强转子类


//  std::cout << mobility->getIsLeftSatelliteConnection() << " " << mobility->getIsRightSatelliteConnection() << " " << mobility->getTheta() <<std::endl;
//  std::string file_name = "iridium-topology2.txt";
//  std::ofstream file_writer(file_name, std::ios::app);//打开file_name文件，以ios::app追加的方式输入
//  file_writer << "time:" << Simulator::Now () << std::endl;
//  file_writer << pos.x << " " << pos.y << " " << pos.z << std::endl;
//  file_writer << mobility->getIsLeftSatelliteConnection() << " " << mobility->getIsRightSatelliteConnection() << " " << mobility->getTheta() << std::endl;// @suppress("Invalid arguments")
//  file_writer.close();

}

//std::vector< Ptr<Node> >
//buildParentVector(std::vector< Ptr<Node> > & path){
//	std::vector< Ptr<Node> > parentVector;
//	// reset the parent vector
//	parentVector.clear ();
//	uint32_t numberOfNodes = NodeList::GetNNodes ();
//    parentVector.reserve (sizeof (Ptr<Node>)*numberOfNodes);
//    parentVector.insert (parentVector.begin (), sizeof (Ptr<Node>)*numberOfNodes, 0); // initialize to 0
//
//	int size = path.size();
//	if(size <= 0 ) return parentVector;
//	Ptr<Node> pre = path.at(0);
//	parentVector.at (pre->GetId ()) = pre;
//	Ptr<Node> current;
//	for(int i = 1; i < size; i++) {
//		current = path.at(i);
//		parentVector.at (current->GetId ()) = pre;
//		pre = current;
//	}
//	return parentVector;
//}
//
//// 卫星1距离近地点的角度是否在0度到90度 和 270度到360度，即是否在地球的正面
//bool
//isInPositive(double theta1) const
//{
//	return PI/2>=theta1 || PI/2*3<=theta1;//卫星1距离近地点的角度是否在0度到90度 和 270度到360度
//}
//
//// 这里可能有点问题，如果倾斜角是10度的话，根本不会经过极地区域，所以我认为不能按照卫星的近地点所在的纬度超过60度来进行判断，而是判断和近地点的扫过的角度
//// 可能也有问题，应该是快要到轨道交汇点的地点进行判断，目前我按照扫过的角度计算，如果到时候有问题再说
//bool
//isInPolarRegion(double theta) const
//{
//	// 近地点是地球正面的赤道点，所以与近地点相距的角度angle在 60度到120度 和 240度到300度 中，则代表是在极地区域
//   return ((PI/3<=theta)&&(PI/3*2>=theta)) || ((PI/3*4<=theta)&&(PI/3*5>=theta));
//}
//
//// 废弃
//std::vector< Ptr<Node> >
//caseOne(Ptr<Node> source, Ptr<Node> dest){
//	std::vector< Ptr<Node> > sourceUp;
//	std::vector< Ptr<Node> > sourceDown;
//
//	std::set< Ptr<Node> > sourceUpSet;
//	std::set< Ptr<Node> > sourceDownSet;
//
//	Ptr<Node> currentNode = source;
//	for(int i = 0; i < 5; i++){
//
//	}
//	return NULL;
//
//}
//
//
////
//// 分段路由的最短路径算法
//std::vector< Ptr<Node> >
//findPath(Ptr<Node> source, Ptr<Node> dest, NodeContainer nodes){
//
//	Ptr<LEOSatelliteMobilityModel> sourceMobility = source->GetObject<LEOSatelliteMobilityModel> ();
//	Ptr<LEOSatelliteMobilityModel> destMobility = dest->GetObject<LEOSatelliteMobilityModel> ();
//
//	double sourceTheta = sourceMobility->getTheta();//距离近地点的角度，弧度制（因为是圆轨道，没有近地点，这里假设近地点是地球正面的赤道）
//	double destTheta = destMobility->getTheta();
//	//std::cout<< "theta " << sourceTheta << " "<< destTheta << std::endl;
//
//	// 通过 距离近地点的角度 判断是否在seam的两侧
//	// 最开始升交点赤经是在0-180度，最开始近地点为0度的点是在赤道，所以距离近地点角度在 0度到90度 和 270度到360度 的卫星都在seam的同一侧
//	bool sourceInPositive = isInPositive(sourceTheta);//原卫星是否在地球正面
//	bool destInPositive = isInPositive(destTheta);
//	// 如果在同一侧(同为true或者同为false)
//    if(sourceInPositive ^ destInPositive) {
//    	bool isSourceInPolarRegion = isInPolarRegion(sourceTheta);
//    	bool isDestInPolarRegion = isInPolarRegion(destTheta);
//    	//如果都在极地外
//    	if(!isSourceInPolarRegion && !isDestInPolarRegion) {
//    		Vector pos1 = sourceMobility->DoGetPosition();// 这里返回的是赤经、赤纬、高度，而不是经度、纬度、高度（没有考虑地球自传）
//    		Vector pos2 = destMobility->DoGetPosition();
//    		bool isLowerFromSourseToDest;
//    		// 如果是在地球正面，则比较和纬度0（2*PI）的角度差
//    		if(sourceInPositive){
//    			double sourceAngle;
//    			if(pos1.y < PI/2) sourceAngle = pos1.y - 0;
//    			else sourceAngle = 2*PI - pos1.y;
//    			double destAngle;
//    			if(pos2.y < PI/2) destAngle = pos2.y - 0;
//    			else destAngle = 2*PI - pos2.y;
//    			// 如果原卫星的纬度比目标卫星的纬度低
//    			if(sourceAngle <= destAngle){
//    				// 则原卫星先上下传输信息，再左右传输信息
//
//    			}
//    		}
//    	}
//    }
//
//
//
//	std::vector< Ptr<Node> > path = {nodes.Get(0), nodes.Get(11), nodes.Get(12)};
//	return path;
//}



void
Progress (Ipv4Address destinationIp, Ptr<Node> source, Ptr<Node> dest, double timeInterval, NodeContainer nodes)
{
//  std::cout << "时间: "<< Simulator::Now ().GetNanoSeconds() << "  Progress" << std::endl;
//
//  std::vector< Ptr<Node> > path = findPath(source, dest, nodes);
//  std::vector< Ptr<Node> > parentVector = buildParentVector(path);
//
//  Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
////  Simulator::Schedule (Seconds (4),&Ipv4NixVectorRouting::SetCacheDirty, rp ,true);
//  Simulator::ScheduleNow (&Ipv4NixVectorRouting::UpdateNixVectorInCache, rp ,destinationIp, parentVector, source, dest);
//
//  Simulator::Schedule (Seconds (timeInterval), Progress, destinationIp, source, dest, timeInterval, nodes);
}

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  Time::SetResolution (Time::NS);
  //必须有下面2个语句，才会打印udp的信息，默认是info级别的
//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);


  // 当前开始时间
  time_t t = time(0);
  char tmp[64];
  strftime( tmp, sizeof(tmp), "%Y/%m/%d %X",localtime(&t) );
  puts( tmp );


  const double ALTITUDE = 780;//卫星海拔
  const double INCLINATION = 86.4;//倾角
  // Format: nodenumber, longitude（升交点赤经，角度制）, alpha(距离近地点的角度，角度制，因为是圆轨道，没有近地点，这里假设近地点是地球正面的赤道) and plane(轨道编号).
  double iridiumConstellation[6][11][4] = {{{0,0,0.0,0},{1,0,32.73,0},{2,0,65.45,0},{3,0,98.18,0},{4,0,130.91,0},{5,0,163.64,0},{6,0,196.36,0},{7,0,229.09,0},{8,0,261.82,0},{9,0,294.55,0},{10,0,327.27,0}},{{11,30,0.0,1},{12,30,32.73,1},{13,30,65.45,1},{14,30,98.18,1},{15,30,130.91,1},{16,30,163.64,1},{17,30,196.36,1},{18,30,229.09,1},{19,30,261.82,1},{20,30,294.55,1},{21,30,327.27,1}},{{22,60,0.0,2},{23,60,32.73,2},{24,60,65.45,2},{25,60,98.18,2},{26,60,130.91,2},{27,60,163.64,2},{28,60,196.36,2},{29,60,229.09,2},{30,60,261.82,2},{31,60,294.55,2},{32,60,327.27,2}},{{33,90,0.0,3},{34,90,32.73,3},{35,90,65.45,3},{36,90,98.18,3},{37,90,130.91,3},{38,90,163.64,3},{39,90,196.36,3},{40,90,229.09,3},{41,90,261.82,3},{42,90,294.55,3},{43,90,327.27,3}},{{44,120,0.0,4},{45,120,32.73,4},{46,120,65.45,4},{47,120,98.18,4},{48,120,130.91,4},{49,120,163.64,4},{50,120,196.36,4},{51,120,229.09,4},{52,120,261.82,4},{53,120,294.55,4},{54,120,327.27,4}},{{55,150,0.0,5},{56,150,32.73,5},{57,150,65.45,5},{58,150,98.18,5},{59,150,130.91,5},{60,150,163.64,5},{61,150,196.36,5},{62,150,229.09,5},{63,150,261.82,5},{64,150,294.55,5},{65,150,327.27,5}}};
  NodeContainer nodes;
  nodes.Create (66);


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::LEOSatelliteMobilityModel");

  mobility.InstallAll ();

  int index = 0;
  // iterate our nodes and print their position.
  for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j){
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
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
      NS_ASSERT (position != 0);
      index++;

    }

  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));


  // NixHelper to install nix-vector routing on all nodes
  Ipv4NixVectorHelper nixRouting;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (nixRouting, 10);

  InternetStackHelper stack;
  stack.SetRoutingHelper (list); // has effect on the next Install ()
  stack.Install (nodes);

  // 建立链路，分配ip地址
  // 先分配同一轨道的ip地址:从10.1.1.0开始分配，子网掩码是255.255.0.0
  index = 0;
  int ipIndex = 1;
  PointToPointHelper p2p;
  Ipv4AddressHelper ipv4;
  // 数组，用来保存链路分配的ip地址，顺序存放
  Ipv4InterfaceContainer ipv4InterfaceContainer[121];
  while(index<66){
//	  // 每个卫星只负责和它上面的卫星建立链接并分配ip
	  int nextNodeIndex;// 该卫星上面的卫星的索引
	  if(index%11 != 10){
		  nextNodeIndex = index+1;
	  } else {
		  nextNodeIndex = index-10;
	  }
//	  // 断开0-1链路
//	  if(index != 0) {
//
//	  }
	  NodeContainer nodeContainer = NodeContainer (nodes.Get (index), nodes.Get (nextNodeIndex));
	  // 卫星延时时间计算：
	  // 同一个轨道前后卫星距离 d =（海拔+地球半径）*sin（32.72/2）*2*1000 = 4032411.4931 m
	  // 卫星间激光通信： v = 299792458 m/s
	  // delay = d / v = 13 ms
	  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));

	  p2p.SetChannelAttribute ("Delay", StringValue ("13ms"));
	  NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);


	  std::string ip = "10.1." + std::to_string(ipIndex) + ".0";
	  const char* ipp = ip.c_str();
//	  std::cout<< ip << std::endl;
	  ipv4.SetBase (ipp, "255.255.255.0");
	  Ipv4InterfaceContainer ipv4Container = ipv4.Assign (netDeviceContainer);
	  ipv4InterfaceContainer[ipIndex-1] = ipv4Container;

	  index++;
	  ipIndex++;
  }
  //再分配不同轨道的ip地址
  index = 0;
  while(index<55){
	  //每个卫星只负责和它左边的卫星建立链接并分配ip
	  NodeContainer nodeContainer = NodeContainer (nodes.Get (index), nodes.Get (index+11));

	  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	  // 卫星延时时间计算：不同轨道间的卫星之间距离一直在变，这里仿真先直接设为定值13ms，后期再改
	  p2p.SetChannelAttribute ("Delay", StringValue ("13ms"));
	  NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);

	  std::string ip = "10.1." + std::to_string(ipIndex) + ".0";
	  const char* ipp = ip.c_str();
//	  std::cout<< ip << std::endl;
	  ipv4.SetBase (ipp, "255.255.255.0");
	  Ipv4InterfaceContainer ipv4Container = ipv4.Assign (netDeviceContainer);
	  ipv4InterfaceContainer[ipIndex-1] = ipv4Container;
	  index++;
	  ipIndex++;
  }


  // 卫星周期计算公式： T = sqrt((4 * (PI)^2 * (r + h)^3 )/ (G * M ))  , 其中 G * M = 398601.2 (km^3/s^2)
  // irrdium : T = sqrt((4 * (3.1415926535897)^2 * (6378 + 780)^3 )/ 398601.2) = 6026.957216098


//  Ipv4InterfaceContainer i0i1 = ipv4InterfaceContainer[0];//卫星编号0和1之间的链路的ip分配
//  Ipv4InterfaceContainer i1i2 = ipv4InterfaceContainer[1];//卫星编号1和2之间的链路的ip分配
  Ipv4InterfaceContainer i11i12 = ipv4InterfaceContainer[11];//卫星编号11和12之间的链路的ip分配
  Ipv4InterfaceContainer i12i13 = ipv4InterfaceContainer[12];//卫星编号12和13之间的链路的ip分配
  Ipv4InterfaceContainer i23i34 = ipv4InterfaceContainer[90];//卫星编号23和34之间的链路的ip分配
  Ipv4InterfaceContainer i0i11 = ipv4InterfaceContainer[66];//卫星编号0和11之间的链路的ip分配
  Ipv4InterfaceContainer i1i12 = ipv4InterfaceContainer[67];//卫星编号1和12之间的链路的ip分配
  Ipv4InterfaceContainer i2i13 = ipv4InterfaceContainer[68];//卫星编号2和13之间的链路的ip分配

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (12));
  serverApps.Start (Seconds (100.0));
  serverApps.Stop (Seconds (600.0));

  UdpEchoClientHelper echoClient (i12i13.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (50.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (102.0));
  clientApps.Stop (Seconds (600.0));


  // 构造UpdateNixVectorInCache 参数，做实验
  // (Ipv4Address destinationIp, const std::vector< Ptr<Node> > & parentVector, Ptr<Node> source, Ptr<Node> dest)
  Ipv4Address destinationIp = i12i13.GetAddress (0);
  Ptr<Node> source = nodes.Get(0);
  Ptr<Node> dest = nodes.Get(12);

  Simulator::Schedule (Seconds (101.5), Progress,destinationIp,source,dest,50, nodes);

  Simulator::Stop (Seconds (700.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("iridium-nix-route.tr"));
  p2p.EnablePcapAll ("iridium-nix-route");

  Simulator::Run ();
  Simulator::Destroy ();

  // 当前结束时间
  t = time(0);
  strftime( tmp, sizeof(tmp), "%Y/%m/%d %X" ,localtime(&t) );
  puts( tmp );
}
