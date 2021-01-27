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

// 根据ISL的状态进行快照
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

bool
isLinkInterruptionHelper(Ptr<Node> node, double arriveTime, double perid, Ptr<Node> otherNode){
	Ptr<LEOSatelliteMobilityModel> nodeMobility = node->GetObject<LEOSatelliteMobilityModel> ();
	double enterNorthPoleTime = nodeMobility->m_helper.getEnterNorthPoleTime();
	double leaveNorthPoleTime = nodeMobility->m_helper.getLeaveNorthPoleTime();
	double enterSorthPoleTime = nodeMobility->m_helper.getEnterSorthPoleTime();
	double leaveSorthPoleTime = nodeMobility->m_helper.getLeaveSorthPoleTime();
	// 为了保证enterNorthPoleTime<leaveNorthPoleTime<enterSorthPoleTime<leaveSorthPoleTime,即enterNorthPoleTime作为一个周期的起点，这样arriveTimeInFirstPerid容易找到在哪一个区间
	if(enterNorthPoleTime > leaveNorthPoleTime){
	  leaveNorthPoleTime += perid;
	  enterSorthPoleTime += perid;
	  leaveSorthPoleTime += perid;
	} else if(leaveNorthPoleTime > enterSorthPoleTime){
	  enterSorthPoleTime += perid;
	  leaveSorthPoleTime += perid;
	} else if(enterSorthPoleTime > leaveSorthPoleTime){
	  leaveSorthPoleTime += perid;
	}
	//734.17 1731.54 3747.65 4745.02
	double arriveTimeInFirstPerid = fmod(arriveTime, perid);
//	std::cout << node->GetId() << " " << enterNorthPoleTime << " " << leaveNorthPoleTime << " " <<enterSorthPoleTime <<" " << leaveSorthPoleTime << " " <<arriveTimeInFirstPerid<<std::endl;

	// 为了保证arriveTimeInFirstPerid在上面那个周期里面
	if(arriveTimeInFirstPerid < enterNorthPoleTime){
	  arriveTimeInFirstPerid += perid;
	}
	// 如果该卫星所在的位置在极圈里面
	if (arriveTimeInFirstPerid <= leaveNorthPoleTime || (arriveTimeInFirstPerid >= enterSorthPoleTime && arriveTimeInFirstPerid <= leaveSorthPoleTime)) {
		std::cout << "时间: "<< Simulator::Now ().GetMilliSeconds() << "ms "<< node->GetId() << " 和 "<< otherNode->GetId() << " 之间的链路可能中断" << std::endl;
	    return true;
	}
	return false;
}


bool
isLinkInterruption (Ptr<Node> source, Ptr<Node> dest)
{
  std::cout << "时间: "<< Simulator::Now ().GetSeconds() << "s  isLinkInterruption" << std::endl;
  Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
  rp->SetCacheDirty(true);
  std::vector< Ptr<Node> > path = rp->GetShortPath(source, dest);//在当前快照下得到当前卫星发送的分组所经过的卫星
//    for(uint i=0;i<path.size();i++){
//  	  std::cout << "节点编号: "<< path[i]->GetId() << " ";
//    }
//    std::cout << std::endl;
  int hopCount = path.size()-1;
  // 估计分组从源卫星到达目的卫星的时间， 包括传播延时，收发延时，排队延时等
  // 传播延时：同一个轨道前后卫星距离 d =（海拔+地球半径）*sin（32.72/2）*2*1000 = 4032411.4931 m，卫星间激光通信： v = 299792458 m/s，delay = d / v = 13 ms
  // 收发延时：1024byte/5Mbps = 1024byte/675000byte/s = 0.001517037s = 1.5ms
  // 保护延时（代替排队延时）：暂设为100ms
  // 分组从一个卫星发送到另一个卫星的时间：1次传播延时+2次收发延时+1次排队延时 = 16ms + 保护延时
  // 所以总延时 = （16ms + 保护延时）* 跳数
  int sumDelayTime = (16+10000)*hopCount;//单位ms
  double currentTime = Simulator::Now ().GetMilliSeconds();
  double arriveTime = (currentTime + sumDelayTime)/1000;// 分组到到目的节点的时间  ms----->s

  Ptr<LEOSatelliteMobilityModel> sourceMobility = source->GetObject<LEOSatelliteMobilityModel> ();
  double perid = sourceMobility->GetSatSphericalPos().period;
  for(uint i = 0; i < path.size() - 1; i++){
	  Ptr<Node> node = path[i];
	  Ptr<Node> nextNode = path[i+1];
	  int currentNodeId = node->GetId();
	  int nextNodeId = nextNode->GetId();
	  //同一轨道链路的话，不存在将断链路，直接进入下一次循环
	  // 如果是轨道间链路，其中，11是同一轨道的卫星数量
	  if(currentNodeId == nextNodeId - 11 || currentNodeId == nextNodeId + 11 ){
		  // 判断当前卫星是否在分组到达目的节点时间段过程中进入极区
		  bool flag =  isLinkInterruptionHelper(node, arriveTime, perid, nextNode);
		  if (flag) return true;
		  // 判断下一个卫星是否在分组到达目的节点时间段过程中进入极区
		  flag = isLinkInterruptionHelper(nextNode, arriveTime, perid, node);
		  if (flag) return true;
	  }
   }
  return false;
}




// 从某一节点向下找节点至另一节点，依次把他们加入path（dest不会加入到path中）
void
afterNodeAddToPath(std::vector< Ptr<Node> > & path, Ptr<Node> currentNode, Ptr<Node> dest){
	while(currentNode->GetId() != dest->GetId()) {
		Ptr<Node> temp = currentNode;
		path.push_back(temp);
		currentNode = currentNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.down;
	}
}

// 从某一节点向上找节点至另一节点，依次把他们加入path（dest不会加入到path中）
void
beforeNodeAddToPath(std::vector< Ptr<Node> > & path, Ptr<Node> currentNode, Ptr<Node> dest){
	while(currentNode->GetId() != dest->GetId()) {
		Ptr<Node> temp = currentNode;
		path.push_back(temp);
		currentNode = currentNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.up;
	}
}

// 从某一节点向下找节点至另一节点，计算其中跳数
int
afterNodeLength(Ptr<Node> currentNode, Ptr<Node> dest){
	int hop = 0;
	while(currentNode->GetId() != dest->GetId()) {
		hop++;
		currentNode = currentNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.down;
	}
	return hop;
}

// 从某一节点向上找节点至另一节点，计算其中跳数
int
beforeNodeLength(Ptr<Node> currentNode, Ptr<Node> dest){
	int hop = 0;
	while(currentNode->GetId() != dest->GetId()) {
		hop++;
		currentNode = currentNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.up;
	}
	return hop;
}

std::vector< Ptr<Node> >
segmentRoutingPath (Ptr<Node> source, Ptr<Node> dest)
{
	std::vector< Ptr<Node> > path;
	  Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
	// 判断当前源卫星节点和目的卫星节点所在的位置是否在极圈内或者在极圈边缘
	Ptr<LEOSatelliteMobilityModel> sourceMobility = source->GetObject<LEOSatelliteMobilityModel> ();
	bool sourceFlag = sourceMobility->m_helper.isInOrNearPolarRegion(); // @suppress("Invalid arguments")
	Ptr<LEOSatelliteMobilityModel> destMobility = dest->GetObject<LEOSatelliteMobilityModel> ();
	bool destFlag = destMobility->m_helper.isInOrNearPolarRegion(); // @suppress("Invalid arguments")

	// 如果出现了分组发送过程中会出现联络中断的情况，则源节点和目的节点至少一个在极圈内或者在极圈边缘
	if(!sourceFlag && !destFlag) {
		std::cout << "不可能出现的情况，请检查代码逻辑" << std::endl;
	}else if(sourceFlag && destFlag){  //2个都在极圈范围内
		// 对应4个中间节点
		Ptr<Node> segmentSourceAfterNode = sourceMobility->m_helper.findAfterNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		Ptr<Node> segmentSourceBeboreNode = sourceMobility->m_helper.findBeforeNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		Ptr<Node> segmentDestAfterNode = destMobility->m_helper.findAfterNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		Ptr<Node> segmentDestBeboreNode = destMobility->m_helper.findBeforeNodeNotInPolarRegion(); // @suppress("Invalid arguments")

//		std::cout << "segmentSourceAfterNode: " << segmentSourceAfterNode->GetId() << std::endl;
//		std::cout << "segmentSourceBeboreNode: " << segmentSourceBeboreNode->GetId() << std::endl;
//		std::cout << "segmentDestAfterNode: " << segmentDestAfterNode->GetId() << std::endl;
//		std::cout << "segmentDestBeboreNode: " << segmentDestBeboreNode->GetId() << std::endl;

		Ptr<LEOSatelliteMobilityModel> segmentSourceBeboreNodeMobility = segmentSourceBeboreNode->GetObject<LEOSatelliteMobilityModel> ();
//		std::cout << "111" << std::boolalpha << segmentSourceBeboreNodeMobility->m_helper.m_pos.isRightSatelliteConnection << std::endl;
		Ptr<LEOSatelliteMobilityModel> segmentDestBeboreNodeMobility = segmentDestBeboreNode->GetObject<LEOSatelliteMobilityModel> ();
//		std::cout << "222" << std::boolalpha << segmentDestBeboreNodeMobility->m_helper.m_pos.isLeftSatelliteConnection << std::endl;
		// 计算2对中间节点之间的路径
		std::vector< Ptr<Node> > path1 = rp->GetShortPath(segmentSourceAfterNode, segmentDestAfterNode);
		std::vector< Ptr<Node> > path2 = rp->GetShortPath(segmentSourceBeboreNode, segmentDestAfterNode);
		std::vector< Ptr<Node> > path3 = rp->GetShortPath(segmentSourceAfterNode, segmentDestBeboreNode);
		std::vector< Ptr<Node> > path4 = rp->GetShortPath(segmentSourceBeboreNode, segmentDestBeboreNode);
		// 计算选择哪一条路径的总跳数，选择跳数最小的那一条路径
		int selectPath1Length = path1.size() - 1 + afterNodeLength(source, segmentSourceAfterNode) + beforeNodeLength(segmentDestAfterNode, dest);
		int selectPath2Length = path2.size() - 1 + beforeNodeLength(source, segmentSourceBeboreNode) + beforeNodeLength(segmentDestAfterNode, dest);
		int selectPath3Length = path3.size() - 1 + afterNodeLength(source, segmentSourceAfterNode) + afterNodeLength(segmentDestBeboreNode, dest);
		int selectPath4Length = path4.size() - 1 + beforeNodeLength(source, segmentSourceBeboreNode) + afterNodeLength(segmentDestBeboreNode, dest);

//		std::cout << "segmentRoutingPath: 原节点和目的节点都在极圈内部或者极圈边缘" << std::endl;
//    	std::cout << "path1: " << selectPath1Length;
//    	for(uint i=0;i<path1.size();i++){
//		  std::cout << " 节点编号: "<< path1[i]->GetId() << " ";
//		}
//		std::cout << std::endl;
//		std::cout << "path2: "<< selectPath2Length;
//		for(uint i=0;i<path2.size();i++){
//		  std::cout << " 节点编号: "<< path2[i]->GetId() << " ";
//		}
//		std::cout << std::endl;
//		std::cout << "path3: "<< selectPath3Length;
//		for(uint i=0;i<path3.size();i++){
//		  std::cout << " 节点编号: "<< path3[i]->GetId() << " ";
//		}
//		std::cout << std::endl;
//		std::cout << "path4: "<< selectPath4Length;
//		for(uint i=0;i<path4.size();i++){
//		  std::cout << " 节点编号: "<< path4[i]->GetId() << " ";
//		}
//		std::cout << std::endl;


		if(selectPath1Length < selectPath2Length && selectPath1Length < selectPath3Length && selectPath1Length < selectPath4Length){
			Ptr<Node> currentNode = source;
			afterNodeAddToPath(path, currentNode, segmentSourceAfterNode);
			for(uint i=0; i<path1.size(); i++){
				path.push_back(path1[i]);
			}
			Ptr<LEOSatelliteMobilityModel> segmentDestAfterNodeMobility = segmentDestAfterNode->GetObject<LEOSatelliteMobilityModel> ();
			currentNode = segmentDestAfterNodeMobility->m_helper.m_pos.up;
			beforeNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);

		} else if(selectPath2Length < selectPath1Length && selectPath2Length < selectPath3Length && selectPath2Length < selectPath4Length){

			Ptr<Node> currentNode = source;
			beforeNodeAddToPath(path, currentNode, segmentSourceBeboreNode);
			for(uint i=0; i<path2.size(); i++){
				path.push_back(path2[i]);
			}
			Ptr<LEOSatelliteMobilityModel> segmentDestAfterNodeMobility = segmentDestAfterNode->GetObject<LEOSatelliteMobilityModel> ();
			currentNode = segmentDestAfterNodeMobility->m_helper.m_pos.up;
			beforeNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);




		} else if(selectPath3Length < selectPath1Length && selectPath3Length < selectPath2Length && selectPath3Length < selectPath4Length){

			Ptr<Node> currentNode = source;
			afterNodeAddToPath(path, currentNode, segmentSourceAfterNode);
			for(uint i=0; i<path3.size(); i++){
				path.push_back(path3[i]);
			}
			Ptr<LEOSatelliteMobilityModel> segmentDestBeboreNodeMobility = segmentDestBeboreNode->GetObject<LEOSatelliteMobilityModel> ();
			currentNode = segmentDestBeboreNodeMobility->m_helper.m_pos.down;
			afterNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);


		} else {

			Ptr<Node> currentNode = source;
			beforeNodeAddToPath(path, currentNode, segmentSourceBeboreNode);
			for(uint i=0; i<path4.size(); i++){
				path.push_back(path4[i]);
			}
			Ptr<LEOSatelliteMobilityModel> segmentDestBeboreNodeMobility = segmentDestBeboreNode->GetObject<LEOSatelliteMobilityModel> ();
			currentNode = segmentDestBeboreNodeMobility->m_helper.m_pos.down;
			afterNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);
		}

	}else if(sourceFlag){   // 原节点在极圈范围内
		Ptr<Node> segmentSourceAfterNode = sourceMobility->m_helper.findAfterNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		Ptr<Node> segmentSourceBeboreNode = sourceMobility->m_helper.findBeforeNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		// 比距离，看2个中间节点和目标节点的距离哪一个近
		std::vector< Ptr<Node> > path1 = rp->GetShortPath(segmentSourceAfterNode, dest);
		std::vector< Ptr<Node> > path2 = rp->GetShortPath(segmentSourceBeboreNode, dest);
		std::cout << "segmentRoutingPath: 原节点在极圈内部或者极圈边缘" << std::endl;
		std::cout << "path1: " << path1.size();
    	for(uint i=0;i<path1.size();i++){
		  std::cout << " 节点编号: "<< path1[i]->GetId() << " ";
		}
		std::cout << std::endl;
		std::cout << "path2: "<< path2.size();
		for(uint i=0;i<path2.size();i++){
		  std::cout << " 节点编号: "<< path2[i]->GetId() << " ";
		}
		std::cout << std::endl;
		// 如果paht1的路径长度小，则把path1的路径组合到path中返回，否则组合path2的路径到path
		if(path1.size() < path2.size()){
			Ptr<Node> currentNode = source;
			afterNodeAddToPath(path, currentNode, segmentSourceAfterNode);
			for(uint i=0; i<path1.size(); i++) {
				path.push_back(path1[i]);
			}
		} else {
			Ptr<Node> currentNode = source;
			beforeNodeAddToPath(path, currentNode, segmentSourceBeboreNode);
			for(uint i=0; i<path2.size(); i++) {
				path.push_back(path2[i]);
			}
		}
	}else if(destFlag){
		Ptr<Node> segmentDestAfterNode = destMobility->m_helper.findAfterNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		Ptr<Node> segmentDestBeboreNode = destMobility->m_helper.findBeforeNodeNotInPolarRegion(); // @suppress("Invalid arguments")
		// 比距离，看2个中间节点和原节点的距离哪一个近
		std::vector< Ptr<Node> > path1 = rp->GetShortPath(source, segmentDestAfterNode);
		std::vector< Ptr<Node> > path2 = rp->GetShortPath(source, segmentDestBeboreNode);
		std::cout << "segmentRoutingPath: 目的节点在极圈内部或者极圈边缘" << std::endl;
		std::cout << "path1: " << path1.size();
    	for(uint i=0;i<path1.size();i++){
		   std::cout << " 节点编号: "<< path1[i]->GetId() << " ";
		}
		std::cout << std::endl;
		std::cout << "path2: "<< path2.size();
		for(uint i=0;i<path2.size();i++){
		   std::cout << " 节点编号: "<< path2[i]->GetId() << " ";
		}
		std::cout << std::endl;
		// 如果paht1的路径长度小，则把path1的路径组合到path中返回，否则组合path2的路径到path
		if(path1.size() < path2.size()){
			for(uint i=0; i < path1.size(); i++) {
				path.push_back(path1[i]);
			}
			Ptr<Node> currentNode = segmentDestAfterNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.up;
			beforeNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);
		} else {
			for(uint i=0; i < path2.size(); i++) {
				path.push_back(path2[i]);
			}
			Ptr<Node> currentNode = segmentDestBeboreNode->GetObject<LEOSatelliteMobilityModel> ()->m_helper.m_pos.down;
			afterNodeAddToPath(path, currentNode, dest);
			path.push_back(dest);
		}
	}
	return path;
}



// 路径变成父节点数组，为了调用nix内部的api
std::vector< Ptr<Node> >
buildParentVector(std::vector< Ptr<Node> > & path){
	std::vector< Ptr<Node> > parentVector;
	// reset the parent vector
	parentVector.clear ();
	uint32_t numberOfNodes = NodeList::GetNNodes ();
    parentVector.reserve (sizeof (Ptr<Node>)*numberOfNodes);
    parentVector.insert (parentVector.begin (), sizeof (Ptr<Node>)*numberOfNodes, 0); // initialize to 0

	int size = path.size();
	if(size <= 0 ) return parentVector;
	Ptr<Node> pre = path.at(0);
	parentVector.at (pre->GetId ()) = pre;
	Ptr<Node> current;
	for(int i = 1; i < size; i++) {
		current = path.at(i);
		parentVector.at (current->GetId ()) = pre;
		pre = current;
	}
	return parentVector;
}

void
Progress (Ipv4Address destinationIp, Ptr<Node> source, Ptr<Node> dest, double timeInterval)
{
//  std::cout << "时间: "<< Simulator::Now ().GetMilliSeconds() << "  Progress" << std::endl;
	bool flag = isLinkInterruption(source, dest);//分组发送过程中是否会出现链路中断的标识
    if (flag) {
    	std::vector< Ptr<Node> > newPath = segmentRoutingPath(source, dest);//分段路由
    	std::cout << "newPath: ";
    	for(uint i=0;i<newPath.size();i++){
		  std::cout << "节点编号: "<< newPath[i]->GetId() << " ";
		}
		std::cout << std::endl;

    	std::vector< Ptr<Node> > parentVector = buildParentVector(newPath);//父节点数组
    	Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
    	Simulator::ScheduleNow (&Ipv4NixVectorRouting::UpdateNixVectorInCache, rp ,destinationIp, parentVector, source, dest);//规定nix路由走分段路由

    }

    Simulator::Schedule (Seconds (timeInterval), Progress, destinationIp, source, dest, timeInterval);
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
      position->setRoutingAlgorithmAndSnapShotWay(2, 1); // @suppress("Invalid arguments")
      NS_ASSERT (position != 0);
      index++;

    }



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
  serverApps.Start (Seconds (700.0));
  serverApps.Stop (Seconds (6000.0));

  UdpEchoClientHelper echoClient (i12i13.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (50.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (730.0));
  clientApps.Stop (Seconds (6000.0));


  // 构造UpdateNixVectorInCache 参数，做实验
  // (Ipv4Address destinationIp, const std::vector< Ptr<Node> > & parentVector, Ptr<Node> source, Ptr<Node> dest)
  Ipv4Address destinationIp = i12i13.GetAddress (0);
  Ptr<Node> source = nodes.Get(0);
  Ptr<Node> dest = nodes.Get(12);

  Simulator::Schedule (Seconds (729.5), Progress,destinationIp,source,dest,50);
  double simulatorLastTime = 7000.0;
  newSnapshotWay(nodes, simulatorLastTime, 6026.957216098);
  Simulator::Stop (Seconds (simulatorLastTime));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("iridium-nix-segment-routing2.tr"));
//  p2p.EnablePcapAll ("iridium-nix-segment-routing");

  Simulator::Run ();
  Simulator::Destroy ();

  // 当前结束时间
  t = time(0);
  strftime( tmp, sizeof(tmp), "%Y/%m/%d %X" ,localtime(&t) );
  puts( tmp );
}
