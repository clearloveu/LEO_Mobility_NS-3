#include "ns3/simulator.h"
#include "ns3/log.h"
#include "leo-satellite-mobility-model.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-nix-vector-routing.h"


#define PI 3.1415926535897
#define MU 398601.2 // Greek Mu (km^3/s^2)  是一个常量=G万有引力常数*M地球质量，下面用来计算半径---->周期
#define LIGHT 299793 // m/s
#define EARTH_PERIOD 86164 // seconds   地球周期
#define EARTH_RADIUS 6378  // km   地球半径
#define GEO_ALTITUDE 35786 // km   GEO卫星的高度
#define ATMOS_MARGIN 150 // km   大气边缘

#define DEG_TO_RAD(x) ((x) * PI/180)
#define RAD_TO_DEG(x) ((x) * 180/PI)
#define DISTANCE(s_x, s_y, s_z, e_x, e_y, e_z) (sqrt((s_x - e_x) * (s_x - e_x) \
                + (s_y - e_y) * (s_y - e_y) + (s_z - e_z) * (s_z - e_z)))   // 2点的距离
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LEOSatelliteHelper");



LEOSatSphericalPos
LEOSatelliteHelper::computeCurPos(double timeAdvance) const
{
//  std::cout << Simulator::Now ().GetSeconds()<< "+computeCurPos+"<<m_pos.self->GetId() <<std::endl;
  LEOSatSphericalPos initialSphericalPos =  m_pos;
  LEOSatSphericalPos curSphericalPos;
  if(timeAdvance>initialSphericalPos.period) timeAdvance = fmod(timeAdvance, initialSphericalPos.period);//对更新位置的时间差进行卫星周期 先取余
  double angle = timeAdvance/initialSphericalPos.period*2*PI;//得到在更新的时间差下，卫星扫过的角度（弧度制）
  angle = angle +initialSphericalPos.theta;//加上上次的角度，得到现在的角度
  angle = fmod(angle, 2*PI);//如果绕了好几圈了，则对2*PI取余
  
  curSphericalPos.r = initialSphericalPos.r;
  curSphericalPos.theta = angle;
  curSphericalPos.phi = initialSphericalPos.phi;
  curSphericalPos.inclination = initialSphericalPos.inclination;
  curSphericalPos.period = initialSphericalPos.period;

  curSphericalPos.planeNum = initialSphericalPos.planeNum;
  curSphericalPos.plane = initialSphericalPos.plane;
  curSphericalPos.index = initialSphericalPos.index;
  curSphericalPos.self = initialSphericalPos.self;
  curSphericalPos.satelliteNumInOnePlane = initialSphericalPos.satelliteNumInOnePlane;
  // 保存相邻卫星的信息
  curSphericalPos.up = initialSphericalPos.up;
  curSphericalPos.down = initialSphericalPos.down;
  curSphericalPos.left = initialSphericalPos.left;
  curSphericalPos.right = initialSphericalPos.right;


	Ptr<Node> self = m_pos.self;
	Ptr<LEOSatelliteMobilityModel> mobility = self->GetObject<LEOSatelliteMobilityModel> ();
	int routingAlgorithmNumber = mobility->getRoutingAlgorithmNumber();

	// 如果路由算法不是nix，则结合isFault参数调整链路状态
	if(routingAlgorithmNumber !=2){
		  // 判断自己或者左右相邻卫星的纬度高于60度
		  bool isOwnInPolarRegion = isInPolarRegion2(curSphericalPos);
		  // 如果自己是故障卫星或者已经在极地区域，直接断开和左右卫星的连接
		  if (isFault || isOwnInPolarRegion){
			  curSphericalPos.isLeftSatelliteConnection = false;
			  curSphericalPos.isRightSatelliteConnection = false;
			  // 这里要更新其左右邻居的状态，因为考虑：程序在执行到第一个卫星的更新位置时，此时右卫星可能还在极圈外，如果此卫星也在极圈外，则当前的链路状态是连接的；
			  // 当更新到右卫星的时候，此时右卫星进入极圈，理应终止联络，此时如果不更新左卫星的状态，则会出现混乱，故这里要更新左右卫星的链路状态
			  // 如果左卫星存在
			  if(curSphericalPos.left!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.left->GetObject<LEOSatelliteMobilityModel> ();
				  position->m_helper.m_pos.isRightSatelliteConnection = false;
			  }
			  // 如果右卫星存在
			  if(curSphericalPos.right!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.right->GetObject<LEOSatelliteMobilityModel> ();
				  position->m_helper.m_pos.isLeftSatelliteConnection = false;
			  }
		  }
		  else {
			  // 如果左卫星存在
			  if(curSphericalPos.left!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.left->GetObject<LEOSatelliteMobilityModel> ();
				  LEOSatSphericalPos leftSatellitePos = position->m_helper.m_pos;
				  // 如果左卫星出现故障或者在极区范围内
				  if(position->m_helper.isFault == true || isInPolarRegion2(leftSatellitePos)) {
					  curSphericalPos.isLeftSatelliteConnection = false;
					  // 更新计算位置和设置链路的函数，使后面的更新前面的状态
					  position->m_helper.m_pos.isRightSatelliteConnection = false;
				  }
				  else {
					  curSphericalPos.isLeftSatelliteConnection = true;
					  position->m_helper.m_pos.isRightSatelliteConnection = true;
				  }
			  }else curSphericalPos.isLeftSatelliteConnection = false;
			  // 如果右卫星存在
			  if(curSphericalPos.right!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.right->GetObject<LEOSatelliteMobilityModel> ();
				  LEOSatSphericalPos rightSatellitePos = position->m_helper.m_pos;
				  // 如果右卫星出现故障或者在极区范围内
				  if(position->m_helper.isFault == true ||isInPolarRegion2(rightSatellitePos)) {
					  curSphericalPos.isRightSatelliteConnection = false;
					  position->m_helper.m_pos.isLeftSatelliteConnection = false;
				  }
				  else {
					  curSphericalPos.isRightSatelliteConnection = true;
					  position->m_helper.m_pos.isLeftSatelliteConnection = true;
				  }
			  }else curSphericalPos.isRightSatelliteConnection = false;
		  }
	} else {
		  // 如果是nix 算法，则不结合isFault参数进行链路设计
		  // 这样设计的原因是：如果不做这样的区分，当某个卫星故障后，在SWS算法中，通过nix内部的最短路径算法得到的最短路径将会自动避开故障卫星（因为故障卫星的4条链路都断开了），
		  // 但是在SWS算法中，故障卫星的处理应该在最后，而不是在最开始就考虑进去；
		  // 第二个原因：假设s=8，d=43，从10到43的最短路径本来是10-21-32-43，因此最终路径应该是：9-10-21-32-43
		  // 但是如果32出现故障，此时得到的最短路径将会10-9-20-31-42-43，因此最终路径应该是：9-10-9-20-31-42-43，此时会再次经过9，在nix内部，这是不允许的，会构成回路

		  bool isOwnInPolarRegion = isInPolarRegion2(curSphericalPos);
		  // 如果自己是故障卫星或者已经在极地区域，直接断开和左右卫星的连接
		  if (isOwnInPolarRegion){
			  curSphericalPos.isLeftSatelliteConnection = false;
			  curSphericalPos.isRightSatelliteConnection = false;
			  // 这里要更新其左右邻居的状态，因为考虑：程序在执行到第一个卫星的更新位置时，此时右卫星可能还在极圈外，如果此卫星也在极圈外，则当前的链路状态是连接的；
			  // 当更新到右卫星的时候，此时右卫星进入极圈，理应终止联络，此时如果不更新左卫星的状态，则会出现混乱，故这里要更新左右卫星的链路状态
			  // 如果左卫星存在
			  if(curSphericalPos.left!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.left->GetObject<LEOSatelliteMobilityModel> ();
				  position->m_helper.m_pos.isRightSatelliteConnection = false;
			  }
			  // 如果右卫星存在
			  if(curSphericalPos.right!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.right->GetObject<LEOSatelliteMobilityModel> ();
				  position->m_helper.m_pos.isLeftSatelliteConnection = false;
			  }
		  }
		  else {
			  // 如果左卫星存在
			  if(curSphericalPos.left!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.left->GetObject<LEOSatelliteMobilityModel> ();
				  LEOSatSphericalPos leftSatellitePos = position->m_helper.m_pos;
				  // 如果左卫星出现故障或者在极区范围内
				  if(isInPolarRegion2(leftSatellitePos)) {
					  curSphericalPos.isLeftSatelliteConnection = false;
					  // 更新计算位置和设置链路的函数，使后面的更新前面的状态
					  position->m_helper.m_pos.isRightSatelliteConnection = false;
				  }
				  else {
					  curSphericalPos.isLeftSatelliteConnection = true;
					  position->m_helper.m_pos.isRightSatelliteConnection = true;
				  }
			  }else curSphericalPos.isLeftSatelliteConnection = false;
			  // 如果右卫星存在
			  if(curSphericalPos.right!=NULL){
				  Ptr<LEOSatelliteMobilityModel> position = curSphericalPos.right->GetObject<LEOSatelliteMobilityModel> ();
				  LEOSatSphericalPos rightSatellitePos = position->m_helper.m_pos;
				  // 如果右卫星出现故障或者在极区范围内
				  if(isInPolarRegion2(rightSatellitePos)) {
					  curSphericalPos.isRightSatelliteConnection = false;
					  position->m_helper.m_pos.isLeftSatelliteConnection = false;
				  }
				  else {
					  curSphericalPos.isRightSatelliteConnection = true;
					  position->m_helper.m_pos.isLeftSatelliteConnection = true;
				  }
			  }else curSphericalPos.isRightSatelliteConnection = false;
		  }
	}

//  std::cout<< curSphericalPos.self->GetId() << " " << Simulator::Now ().GetSeconds()   <<std::endl;
//  std::cout << "isLeftSatelliteConnection="  <<  std::boolalpha << curSphericalPos.isLeftSatelliteConnection << std::endl;
//  std::cout << "isRightSatelliteConnection="  <<  std::boolalpha <<curSphericalPos.isRightSatelliteConnection << std::endl;


  return curSphericalPos;

}


// 设置接口权值，设置链路通断，调用计算路由表
void
LEOSatelliteHelper::handle() const
{
	Ptr<Ipv4> ipv4_self = m_pos.self->GetObject<Ipv4> ();
	Ptr<Ipv4> ipv4_before = m_pos.up->GetObject<Ipv4> ();
	Ptr<Ipv4> ipv4_after = m_pos.down->GetObject<Ipv4> ();
	double beforeNodeDistance = getBeforeDistance();
	double afterNodeDistance = getAfterDistance();
	double leftNodeDistance = getLeftDistance();
	double rightNodeDistance = getRightDistance();

	int afterInterfaceIndex = 1;//下面的接口编号
	int beforeInterfaceIndex = 2;//上面的接口编号
	int leftInterfaceIndex = 3;//左边的接口编号
	int rightInterfaceIndex = 4;//右边的接口编号

	// 如果当前卫星是每一个轨道的第一个卫星，则上面的接口编号是1，下面的是2；其他的卫星都是下面的接口编号是1，上面的是2；
	if(m_pos.index % m_pos.satelliteNumInOnePlane == 0) {
		beforeInterfaceIndex = 1;
		afterInterfaceIndex = 2;
	}
	// 如果左边没有卫星，则接口编号3是右边的ip接口，如果右边没有接口，则用不到rightInterfaceIndex了，不需要管他
	if(m_pos.left==NULL){
		rightInterfaceIndex = 3;
	}

	// 设置上面卫星的接口权值
	ipv4_self->SetMetric(beforeInterfaceIndex, beforeNodeDistance);
	ipv4_self->SetUp(beforeInterfaceIndex);
	// 如果当前卫星上面的卫星是每一个轨道的第一个卫星
	if((m_pos.index + 1) % m_pos.satelliteNumInOnePlane == 0){
		ipv4_before->SetMetric(2, beforeNodeDistance);
		ipv4_before->SetUp(2);
	} else{
		ipv4_before->SetMetric(1, beforeNodeDistance);
		ipv4_before->SetUp(1);
	}

	// 设置下面卫星的接口权值
	ipv4_self->SetMetric(afterInterfaceIndex, afterNodeDistance);
	ipv4_self->SetUp(afterInterfaceIndex);
	// 如果当前卫星下面的卫星是每一个轨道的第一个卫星
	if((m_pos.index - 1) % m_pos.satelliteNumInOnePlane == 0){
		ipv4_after->SetMetric(1, afterNodeDistance);
		ipv4_after->SetUp(1);
	} else{
		ipv4_after->SetMetric(2, afterNodeDistance);
		ipv4_after->SetUp(2);
	}

	// 设置左边卫星的接口权值和是否连通
	// 如果左卫星存在
    if(m_pos.left!=NULL){
    	Ptr<Ipv4> ipv4_left = m_pos.left->GetObject<Ipv4> ();
    	int lefttNodeRightInterfaceIndex = 4;//左边卫星的右边接口编号
    	// 如果当前轨道编号是1，则左边卫星的轨道编号是0，则左边卫星的右边接口编号是3
    	if(m_pos.plane == 1){
    		lefttNodeRightInterfaceIndex = 3;
    	}
    	// 如果连通，则更新链路权值和状态
  	    if(m_pos.isLeftSatelliteConnection) {
  	    	ipv4_self->SetMetric (leftInterfaceIndex, leftNodeDistance);
  	    	ipv4_self->SetUp (leftInterfaceIndex);
  	    	ipv4_left->SetMetric(lefttNodeRightInterfaceIndex, leftNodeDistance);
  	    	ipv4_left->SetUp(lefttNodeRightInterfaceIndex);
	    } else{
	    	ipv4_self->SetDown (leftInterfaceIndex);
	    	ipv4_left->SetDown(lefttNodeRightInterfaceIndex);
	    }
    }
	// 设置右边卫星的接口权值和是否连通
	// 如果右卫星存在
    if(m_pos.right!=NULL){
    	Ptr<Ipv4> ipv4_right = m_pos.right->GetObject<Ipv4> ();
    	int rightNodeInterfaceIndex = 3;//左边卫星的右边接口编号

    	// 如果连通，则更新链路权值和状态
  	    if(m_pos.isRightSatelliteConnection) {
  	    	ipv4_self->SetMetric (rightInterfaceIndex, rightNodeDistance);
  	    	ipv4_self->SetUp (rightInterfaceIndex);
  	    	ipv4_right->SetMetric(rightNodeInterfaceIndex, rightNodeDistance);
  	    	ipv4_right->SetUp(rightNodeInterfaceIndex);
	    } else{
	    	ipv4_self->SetDown (rightInterfaceIndex);
	    	ipv4_right->SetDown(rightNodeInterfaceIndex);
	    }
    }
    Ptr<LEOSatelliteMobilityModel> selfMobility = m_pos.self->GetObject<LEOSatelliteMobilityModel> ();
    int selfRoutingAlgorithmNumber = selfMobility->getRoutingAlgorithmNumber();
//    // 更新全局路由
//    if(selfRoutingAlgorithmNumber == 1) {
//		// 65 == 6*11 - 1 ,即最后一个卫星更新位置的时候才去计算全局路由
//		if(m_pos.index == m_pos.planeNum * m_pos.satelliteNumInOnePlane - 1) {
//			std::cout << "时间: "<< Simulator::Now () << " 卫星编号"<<m_pos.index << ": ready to update route"   << std::endl;
//			// 每次都更新路由表
//			Ipv4GlobalRoutingHelper::RecomputeRoutingTables ();
////			Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
//		}
//    }
    // 更新nix路由
    if(selfRoutingAlgorithmNumber == 2) {
    	// 65 == 6*11 - 1 ,即最后一个卫星更新位置的时候才更新nix的路由信息
		if(m_pos.index == m_pos.planeNum * m_pos.satelliteNumInOnePlane - 1) {
//			std::cout << "时间: "<< Simulator::Now () << " 卫星编号"<<m_pos.index << ": ready to update nix route"   << std::endl;
		    Ptr<Ipv4NixVectorRouting> rp = m_pos.self->GetObject<Ipv4NixVectorRouting> ();
		    rp->SetCacheDirty(true);
		}
    }
}


bool
LEOSatelliteHelper::isInPolarRegion1(double theta) const
{
	// 近地点是地球正面的赤道点，所以与近地点相距的角度angle在 60度到120度 和 240度到300度 中，则代表是在极地区域，要断开连接的
   return ((PI/3<=theta)&&(PI/3*2>=theta)) || ((PI/3*4<=theta)&&(PI/3*5>=theta));
}

// 当前卫星所在的纬度是否超过60度来进行判断是否进入极地区域(如果倾斜角是10度的话，根本不会经过极地区域)
bool
LEOSatelliteHelper::isInPolarRegion2(LEOSatSphericalPos pos) const
{
	 Vector currentPos = getCoordinateBySatSpher(pos);
	 double r = currentPos.x * currentPos.x + currentPos.y * currentPos.y + currentPos.z * currentPos.z;
	 double weiDu = abs(currentPos.z)/sqrt(r);// sin(纬度)，如果这个值大于0.866025 （sin60度）的话，则在极地区域
	 if(weiDu >= 0.866025) return true;
	 else return false;
}

bool
LEOSatelliteHelper::isInPolarRegion2(Ptr<Node> node) const{
	Ptr<LEOSatelliteMobilityModel> mobility = node->GetObject<LEOSatelliteMobilityModel> ();
	// 先调用update函数，更新卫星的最新位置
	mobility->m_helper.UpdateOnlyComputeCurPos();
	LEOSatSphericalPos pos = mobility->m_helper.m_pos;
	return isInPolarRegion2(pos);
}

// 判断当前卫星是否在极圈内部或者在极圈边缘,在分段路由算法中会被调用
bool
LEOSatelliteHelper::isInOrNearPolarRegion(void) const
{
	// 先调用update函数，更新卫星的最新位置
	UpdateOnlyComputeCurPos();

	Vector currentPos = getCoordinateBySatSpher(m_pos);
	double r = currentPos.x * currentPos.x + currentPos.y * currentPos.y + currentPos.z * currentPos.z;
    double weiDu = abs(currentPos.z)/sqrt(r);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
    if(weiDu >= 0.848048) {
    	return true;
    }
    else {
    	// 判断该卫星的左右卫星是否在极圈内部或者在极圈边缘
    	if (m_pos.left != NULL){
    		Ptr<Node> leftNode = m_pos.left;
    		Ptr<LEOSatelliteMobilityModel> leftNodeMobility = leftNode->GetObject<LEOSatelliteMobilityModel> ();
    		leftNodeMobility->m_helper.UpdateOnlyComputeCurPos();
    		Vector leftPos = getCoordinateBySatSpher(leftNodeMobility->m_helper.m_pos);
    		double r_left = leftPos.x * leftPos.x + leftPos.y * leftPos.y + leftPos.z * leftPos.z;
    		double weiDu_left = abs(leftPos.z)/sqrt(r_left);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
    		if(weiDu_left >= 0.848048) {
    			return true;
    		}
    	}
    	if (m_pos.right != NULL){
			Ptr<Node> rightNode = m_pos.right;
			Ptr<LEOSatelliteMobilityModel> rightNodeMobility = rightNode->GetObject<LEOSatelliteMobilityModel> ();
			rightNodeMobility->m_helper.UpdateOnlyComputeCurPos();
			Vector rightPos = getCoordinateBySatSpher(rightNodeMobility->m_helper.m_pos);
			double r_right = rightPos.x * rightPos.x + rightPos.y * rightPos.y + rightPos.z * rightPos.z;
			double weiDu_right = abs(rightPos.z)/sqrt(r_right);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
			if(weiDu_right >= 0.848048) {
				return true;
			}
		}
    	// 如果左右卫星都不在极圈内部或者在极圈边缘，则返回false
    	return false;
    }


	// 不合理，废弃
//	bool currentNodeFlag = isInPolarRegion2(m_pos);
//	if(currentNodeFlag){
//		return true;
//	}
//	Ptr<Node> nextNode = m_pos.after;
//	Ptr<Node> beforeNode = m_pos.before;
//	Ptr<LEOSatelliteMobilityModel> nextNodeMobility = nextNode->GetObject<LEOSatelliteMobilityModel> ();
//	Ptr<LEOSatelliteMobilityModel> beforeNodeMobility = beforeNode->GetObject<LEOSatelliteMobilityModel> ();
//	bool nextNodeFlag = isInPolarRegion2(nextNodeMobility->m_helper.m_pos);
//	bool beforeNodeFlag = isInPolarRegion2(beforeNodeMobility->m_helper.m_pos);
//	// 如果它邻居卫星节点是在极地区域，则代表当前卫星节点在极圈边缘
//	if(nextNodeFlag || beforeNodeFlag) {
//		return true;
//	}
//	return false;
}



bool
LEOSatelliteHelper::isInOrNearPolarRegion(Ptr<Node> node) const
{
	Ptr<LEOSatelliteMobilityModel> mobility = node->GetObject<LEOSatelliteMobilityModel> ();
	// 先调用update函数，更新卫星的最新位置
	mobility->m_helper.UpdateOnlyComputeCurPos();
	LEOSatSphericalPos pos = mobility->m_helper.m_pos;

	Vector currentPos = getCoordinateBySatSpher(pos);
	double r = currentPos.x * currentPos.x + currentPos.y * currentPos.y + currentPos.z * currentPos.z;
    double weiDu = abs(currentPos.z)/sqrt(r);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
    if(weiDu >= 0.848048) {
    	return true;
    }
    else {
    	// 判断该卫星的左右卫星是否在极圈内部或者在极圈边缘
    	if (pos.left != NULL){
    		Ptr<Node> leftNode = pos.left;
    		Ptr<LEOSatelliteMobilityModel> leftNodeMobility = leftNode->GetObject<LEOSatelliteMobilityModel> ();
    		leftNodeMobility->m_helper.UpdateOnlyComputeCurPos();
    		Vector leftPos = getCoordinateBySatSpher(leftNodeMobility->m_helper.m_pos);
    		double r_left = leftPos.x * leftPos.x + leftPos.y * leftPos.y + leftPos.z * leftPos.z;
    		double weiDu_left = abs(leftPos.z)/sqrt(r_left);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
    		if(weiDu_left >= 0.848048) {
    			return true;
    		}
    	}
    	if (pos.right != NULL){
			Ptr<Node> rightNode = pos.right;
			Ptr<LEOSatelliteMobilityModel> rightNodeMobility = rightNode->GetObject<LEOSatelliteMobilityModel> ();
			rightNodeMobility->m_helper.UpdateOnlyComputeCurPos();
			Vector rightPos = getCoordinateBySatSpher(rightNodeMobility->m_helper.m_pos);
			double r_right = rightPos.x * rightPos.x + rightPos.y * rightPos.y + rightPos.z * rightPos.z;
			double weiDu_right = abs(rightPos.z)/sqrt(r_right);// sin(纬度)，如果这个值大于0.848048（sin58度）的话，则在极圈内部或者在极圈边缘
			if(weiDu_right >= 0.848048) {
				return true;
			}
		}
    	// 如果左右卫星都不在极圈内部或者在极圈边缘，则返回false
    	return false;
    }
}


//在调用isInOrNearPolarRegion得到true的情况下，调用此函数将得到和该卫星同一轨道且向下的第一个不在极圈的卫星
Ptr<Node>
LEOSatelliteHelper::findAfterNodeNotInPolarRegion(void) const
{
	Ptr<Node> afterNode = m_pos.down;
	int flag = 500;
	// 如果卫星不经过极圈范围(卫星倾角太小)，则代表这里会死循环，需要注意
	while(true){
		flag--;
		if(flag<0){
			std::cout<<"leo-satellite-helper.cc里的findAfterNodeNotInPolarRegion出现死循环，可能的原因是卫星不经过极圈范围"<<std::endl;
		}
		Ptr<LEOSatelliteMobilityModel> afterNodeMobility = afterNode->GetObject<LEOSatelliteMobilityModel> ();
		afterNodeMobility->m_helper.UpdateOnlyComputeCurPos();
		bool flag = afterNodeMobility->m_helper.isInOrNearPolarRegion();
		if(!flag) return afterNode;
		else afterNode = afterNodeMobility->m_helper.m_pos.down;
	}
}

//在调用isInOrNearPolarRegion得到true的情况下，调用此函数将得到和该卫星同一轨道且向上的第一个不在极圈的卫星
Ptr<Node>
LEOSatelliteHelper::findBeforeNodeNotInPolarRegion(void) const
{
	Ptr<Node> beforeNode = m_pos.up;
	int flag = 500;
	// 如果卫星不经过极圈范围(卫星倾角太小)，则代表这里会死循环，需要注意
	while(true){
		flag--;
		if(flag<0){
			std::cout<<"leo-satellite-helper.cc里的findBeforeNodeNotInPolarRegion出现死循环，可能的原因是卫星不经过极圈范围"<<std::endl;
		}
		Ptr<LEOSatelliteMobilityModel> beforeNodeMobility = beforeNode->GetObject<LEOSatelliteMobilityModel> ();
		beforeNodeMobility->m_helper.UpdateOnlyComputeCurPos();
		bool flag = beforeNodeMobility->m_helper.isInOrNearPolarRegion();
		if(!flag) return beforeNode;
		else beforeNode = beforeNodeMobility->m_helper.m_pos.up;
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
//	std::cout << std::boolalpha << nodeMobility->m_helper.m_pos.isLeftSatelliteConnection<<std::endl;

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
LEOSatelliteHelper::isLinkInterruption (Ptr<Node> dest) const{
	Ptr<Node> source = m_pos.self;
//	std::cout << "时间: "<< Simulator::Now ().GetSeconds() << "s  isLinkInterruption" << std::endl;
	Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
	rp->SetCacheDirty(true);
	std::vector< Ptr<Node> > path = rp->GetShortPath(source, dest);//在当前快照下得到当前卫星发送的分组所经过的卫星
//	for(uint i=0;i<path.size();i++){
//	  std::cout << "节点编号: "<< path[i]->GetId() << " ";
//	}
//	std::cout << std::endl;
	int hopCount = path.size()-1;
	// 估计分组从源卫星到达目的卫星的时间， 包括传播延时，收发延时，排队延时等
	// 传播延时：同一个轨道前后卫星距离 d =（海拔+地球半径）*sin（32.72/2）*2*1000 = 4032411.4931 m，卫星间激光通信： v = 299792458 m/s，delay = d / v = 13 ms
	// 收发延时：1024byte/5Mbps = 1024byte/675000byte/s = 0.001517037s = 1.5ms
	// 保护延时（代替排队延时）：暂设为100ms
	// 分组从一个卫星发送到另一个卫星的时间：1次传播延时+2次收发延时+1次排队延时 = 16ms + 保护延时
	// 所以总延时 = （16ms + 保护延时）* 跳数
	int sumDelayTime = (16+500)*hopCount;//单位ms
	double currentTime = Simulator::Now ().GetMilliSeconds();
	double arriveTime = (currentTime + sumDelayTime)/1000;// 分组到到目的节点的时间  ms----->s

	Ptr<LEOSatelliteMobilityModel> sourceMobility = source->GetObject<LEOSatelliteMobilityModel> ();
	double perid = sourceMobility->m_helper.m_pos.period;
	for(uint i = 0; i < path.size() - 1; i++){
	  Ptr<Node> node = path[i];
	  Ptr<Node> nextNode = path[i+1];
	  int currentNodeId = node->GetId();
	  int nextNodeId = nextNode->GetId();
	  //同一轨道链路的话，不存在将断链路，直接进入下一次循环
	  // 如果是轨道间链路
	  if(currentNodeId == nextNodeId - m_pos.satelliteNumInOnePlane || currentNodeId == nextNodeId + m_pos.satelliteNumInOnePlane ){
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

bool
LEOSatelliteHelper::moveDirection (Ptr<Node> node) const{
	Ptr<LEOSatelliteMobilityModel> model = node->GetObject<LEOSatelliteMobilityModel> ();
	// 通过和近地点相距的角度衡量，因为假设近地点在赤道上方且从南向北方向的位置上
	// 所以从南向北的卫星距离近地点的角度时0-90度和270-360度
	double theta = model->m_helper.m_pos.theta;
	// 返回当前卫星的运动方向，true代表从南向北，false代表从北向南
	return (PI/2>=theta || PI/2*3<=theta);
}

//bool
//LEOSatelliteHelper::packetMoveDirection (Ptr<Node> dest) const{
//	Ptr<Node> source = m_pos.self;
//	//  std::cout << "时间: "<< Simulator::Now ().GetMilliSeconds() << "  isLinkInterruption" << std::endl;
//	Ptr<Ipv4NixVectorRouting> rp = source->GetObject<Ipv4NixVectorRouting> ();
//	rp->SetCacheDirty(true);
//	std::vector< Ptr<Node> > path = rp->GetShortPath(source, dest);//在当前快照下得到当前卫星发送的分组所经过的卫星
//	Ptr<Node> currentNode = path[0];
//	for(uint i=1;i<path.size();i++){
//		Ptr<LEOSatelliteMobilityModel> currentNodeMobility = currentNode->GetObject<LEOSatelliteMobilityModel> ();
//		Ptr<Node> nextNode = path[i];
//		if(currentNodeMobility->m_helper.m_pos.up->GetId() == nextNode->GetId()) return true;//true代表在卫星轨道上逆时针方向，即在近地点处是从南向北
//		if(currentNodeMobility->m_helper.m_pos.down->GetId() == nextNode->GetId()) return false;//false代表在卫星轨道上顺时针方向，即在近地点处是从北向南
//		currentNode = nextNode;
//	}
//	NS_ASSERT(NULL); // 程序不应该走到这儿，因为如果分组没有在同轨道上传输，则不应该调用这个函数（在判断s和d的运动方向是否一致那里，不应该是不一致的情况）
//	return false;
//}


Ptr<Node>
LEOSatelliteHelper::findFirstNodeOutOfPolarRegionByDoubleV(Ptr<Node> src, Ptr<Node> dest) const{
	bool moveDir = moveDirection(src);// s的卫星运动方向
	Ptr<Node> currentNode = src;
	Ptr<Node> upNodeOutOfPR; // 从s往上走的第一个出极区且和s运动方向不一致的节点
	Ptr<Node> downNodeOutOfPR;// 从s往下走的第一个出极区且和s运动方向不一致的节点
	while(true){
		Ptr<LEOSatelliteMobilityModel> currentNode_model = currentNode->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<Node> nextNode = currentNode_model->m_helper.m_pos.up;
//		std::cout<< nextNode->GetId() << "+1+"<<std::endl;
		bool currentMoveDirection = moveDirection(nextNode);
		if(currentMoveDirection != moveDir) { // 如果下一个卫星的运动方向和s的运动方向不一致
			bool flag = isInPolarRegion2(nextNode);
			if(!flag) {
				upNodeOutOfPR = nextNode; // 如果不在极区，说明下一个卫星是穿越极区的第一个卫星节点
				break;
			}
		}
		currentNode = nextNode;
	}
	currentNode = src;
	while(true){
		Ptr<LEOSatelliteMobilityModel> currentNode_model = currentNode->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<Node> nextNode = currentNode_model->m_helper.m_pos.down;
//		std::cout<< nextNode->GetId() << "+2+"<<std::endl;
		bool currentMoveDirection = moveDirection(nextNode);
		if(currentMoveDirection != moveDir) { // 如果下一个卫星的运动方向和s的运动方向不一致
			bool flag = isInPolarRegion2(nextNode);
			if(!flag) {
				downNodeOutOfPR = nextNode; // 如果不在极区，说明下一个卫星是穿越极区的第一个卫星节点
				break;
			}
		}
		currentNode = nextNode;
	}
	Ptr<Ipv4NixVectorRouting> rp = currentNode->GetObject<Ipv4NixVectorRouting> ();
	std::vector< Ptr<Node> > path1 = rp->GetShortPath(upNodeOutOfPR, dest);
	std::vector< Ptr<Node> > path2 = rp->GetShortPath(downNodeOutOfPR, dest);
	// 选择其中某一个到dest跳数较小的点
	if(path1.size() <= path2.size()) {
		return upNodeOutOfPR;
	} else {
		return downNodeOutOfPR;
	}
}

bool
LEOSatelliteHelper::isQiHangNode(Ptr<Node> node) const{
	Ptr<LEOSatelliteMobilityModel> model = node->GetObject<LEOSatelliteMobilityModel> ();
	bool flag = isInPolarRegion2(node);
	// 如果该节点已经在极区
	if(flag){
		// 判断是否经过极点，通过theta的范围判断
		double theta = model->m_helper.m_pos.theta;
		return (PI/2>=theta || (PI<=theta && theta <= PI/2*3));//已经在极区的前提下，theta范围可以放宽
	}else {
		// 如果不在极区，则看其本身和左右卫星的纬度是否大于58度。论文中是计算在分组传输的过程中是否会出现轨道间链路连接到中断的过程，这里简单处理，后期再改
		return isInOrNearPolarRegion(node);
	}
}

bool
LEOSatelliteHelper::isSafeNode(Ptr<Node> node) const{
	// 论文中安全节点是分组传输过程中一直拥有轨道间链路的卫星节点，这里只要其本身和左右卫星的纬度小于58度即可，简单处理，后期再改
//	std::cout<<"isSafeNode" <<std::endl;
	bool flag = isInOrNearPolarRegion(node);
	if(flag) return false;
	else return true;
}

Ptr<Node>
LEOSatelliteHelper::findSafeNode(Ptr<Node> node) const{
	Ptr<Node> currentNode = node;
	while(true){
		Ptr<LEOSatelliteMobilityModel> currentNode_model = currentNode->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<Node> nextNode = currentNode_model->m_helper.m_pos.down; // 逆着运动方向就是下方的卫星
		bool flag = isSafeNode(nextNode);
		if(flag) return nextNode;
		currentNode = nextNode;
	}
}


void
LEOSatelliteHelper::findWayPoint(Ptr<Node> src, Ptr<Node> ssrc, Ptr<Node> dest) {
//	std::cout<<"findWayPoint"<<std::endl;
	bool ssrcQihangFlag = isQiHangNode(ssrc);
	if(ssrcQihangFlag) {
		Ptr<Node> waypoint1 = findSafeNode(ssrc);
		std::vector<Ptr<Node>> temp;
		temp.push_back(waypoint1);
		W["src"] = temp;
	} else {
		if(ssrc->GetId() != src->GetId()) {
			std::vector<Ptr<Node>> temp;
			temp.push_back(ssrc);
			W["src"] = temp;
		}
	}
	bool destQihangFlag = isQiHangNode(dest);
	if(destQihangFlag) {
		Ptr<Node> waypoint2 = findSafeNode(dest);
		std::vector<Ptr<Node>> temp;
		temp.push_back(waypoint2);
		W["des"] = temp;
	}
}



void
LEOSatelliteHelper::jiangDuanLinkDeal(Ptr<Node> dest) {
	Ptr<Node> src = m_pos.self;
	bool srcMoveFlag = moveDirection(src);
	bool destMoveFlag = moveDirection(dest);
//	std::cout<< "srcMoveFlag: " << std::boolalpha <<srcMoveFlag << " destMoveFlag: " << std::boolalpha <<destMoveFlag<< std::endl;

	Ptr<Node> ssrc = src;
	if (srcMoveFlag != destMoveFlag) {
		ssrc = findFirstNodeOutOfPolarRegionByDoubleV(src ,dest);
	};
	findWayPoint(src, ssrc, dest);

	//测试，遍历航点集合
    std::map<std::string, std::vector<Ptr<Node>>>::iterator iter;
    for(iter = W.begin(); iter != W.end(); iter++) {
    	std::cout << iter->first << ": ";
    	std::vector<Ptr<Node>> temp = iter->second;
    	for(uint i=0;i<temp.size();i++){
    		std::cout<< temp[i]->GetId() << " " ;
    	}
    	std::cout<<std::endl;
    }
}


Vector
LEOSatelliteHelper::getCoordinateBySatSpher(LEOSatSphericalPos pos) const
{

	  double r = pos.r;  // 半径
	  double theta = pos.theta; //距离近地点的角度，弧度制
	  double phi = pos.phi; //升交点赤经，弧度制
	  double i = pos.inclination;//倾角，弧度制
	  // 这里利用轨道六要素和近地点计算某个时刻的卫星位置（圆轨道没有近地点，这里把圆当作椭圆，假设有）
	  // 这里三维坐标中，x轴代表赤经为0的线，y轴代表赤纬为0的线，z轴代表高度（xoy面即赤道面）
	  double x0 = r*cos(theta);
	  double y0 = r*sin(theta);
	  double z0 = 0;
	  // 计算过程参考ipad
	  double x1 = x0;
	  double y1 = cos(i)*y0-sin(i)*z0;
	  double z1 = sin(i)*y0+cos(i)*z0;

	  double x = cos(phi)*x1-sin(phi)*y1;
	  double y = sin(phi)*x1+cos(phi)*y1;
	  double z = z1;

	  return Vector (x, y, z) ;
}


double
LEOSatelliteHelper::getDistanceFromOtherNode(Ptr<Node> node) const
{
	UpdateOnlyComputeCurPos();
	Vector m_pos3D = getCoordinateBySatSpher(m_pos);
	Ptr<LEOSatelliteMobilityModel> model = node->GetObject<LEOSatelliteMobilityModel> ();
	model->m_helper.UpdateOnlyComputeCurPos();
	LEOSatSphericalPos pos = model->DoGetSatSphericalPos();
	Vector pos3D = getCoordinateBySatSpher(pos);
	return sqrt((m_pos3D.x-pos3D.x)*(m_pos3D.x-pos3D.x)+(m_pos3D.y-pos3D.y)*(m_pos3D.y-pos3D.y)+(m_pos3D.z-pos3D.z)*(m_pos3D.z-pos3D.z));
}


// 获得本卫星和上方卫星的距离
double
LEOSatelliteHelper::getBeforeDistance(void) const
{
	Ptr<Node> beforeNode = m_pos.up;

	return getDistanceFromOtherNode(beforeNode);
}
// 获得本卫星和下方卫星的距离
double
LEOSatelliteHelper::getAfterDistance(void) const
{
	Ptr<Node> afterNode = m_pos.down;

	return getDistanceFromOtherNode(afterNode);
}
// 获得本卫星和左边卫星的距离
double
LEOSatelliteHelper::getLeftDistance(void) const
{
	// 如果左卫星存在
	if(m_pos.left !=NULL){
		Ptr<Node> leftNode = m_pos.left;
		return getDistanceFromOtherNode(leftNode);
	}else return -1.0;
}
// 获得本卫星和右边卫星的距离
double
LEOSatelliteHelper::getRightDistance(void) const
{
	// 如果右卫星存在
	if(m_pos.right !=NULL){
		Ptr<Node> rightNode = m_pos.right;
		return getDistanceFromOtherNode(rightNode);
	}else return -1.0;
}

// 获得第一个周期内进入北极圈的时间
double
LEOSatelliteHelper::getEnterNorthPoleTime(void) const
{
	return enterNorthPoleTime;
}

// 获得第一个周期内离开北极圈的时间
double
LEOSatelliteHelper::getLeaveNorthPoleTime(void) const
{
	return leaveNorthPoleTime;
}

// 获得第一个周期内进入南极圈的时间
double
LEOSatelliteHelper::getEnterSorthPoleTime(void) const
{
	return enterSorthPoleTime;
}

// 获得第一个周期内离开南极圈的时间
double
LEOSatelliteHelper::getLeaveSorthPoleTime(void) const
{
	return leaveSorthPoleTime;
}

LEOSatSphericalPos
LEOSatelliteHelper::convertPolarToSpherical(const LEOSatPolarPos &polarPos)
{
   LEOSatSphericalPos sphericalPos;
   // Check validity of passed values.
   if (polarPos.altitude < 0)
   {
      NS_LOG_FUNCTION(this << "Altitude out of bounds");
   }
   if ((polarPos.longitude < -180) || (polarPos.longitude) > 180)
   {
      NS_LOG_FUNCTION(this << "Longitude out of bounds");
   }
   if ((polarPos.alpha < 0) || (polarPos.alpha >= 360))
   {
      NS_LOG_FUNCTION(this << "Alpha out of bounds");
   }
   if ((polarPos.inclination < 0) || (polarPos.inclination > 180))
   {
      NS_LOG_FUNCTION(this << "Inclination out of bounds");
   }
   sphericalPos.r = polarPos.altitude + EARTH_RADIUS;
   sphericalPos.theta = DEG_TO_RAD(polarPos.alpha);
   if (polarPos.longitude < 0)
   {
     sphericalPos.phi = DEG_TO_RAD(360 + polarPos.longitude);
   }
   else
   {
      sphericalPos.phi = DEG_TO_RAD(polarPos.longitude);
   }
   sphericalPos.inclination = DEG_TO_RAD(polarPos.inclination);
   double num = sphericalPos.r * sphericalPos.r * sphericalPos.r;
   sphericalPos.period = 2 * PI * sqrt(num/MU);

   sphericalPos.planeNum = polarPos.planeNum;
   sphericalPos.plane = polarPos.plane;
   sphericalPos.index = polarPos.index;
   sphericalPos.self = polarPos.self;
   sphericalPos.satelliteNumInOnePlane = polarPos.satelliteNumInOnePlane;
   // 保存该卫星的相邻卫星的信息(因为理想情况下，同海拔高度的卫星的运行周期一样，所以一个卫星的左右相邻卫星不会变)
   sphericalPos.up = polarPos.before;
   sphericalPos.down = polarPos.after;
   sphericalPos.left = polarPos.left ;
   sphericalPos.right = polarPos.right;

   // 用来标识其左右相邻卫星是否与该卫星相连（极地处发生连接断开，这里认为卫星只要纬度高于60度就断开连接），初始为true，在更新位置时改变
   sphericalPos.isLeftSatelliteConnection = true;
   sphericalPos.isRightSatelliteConnection = true;
   return sphericalPos;
}

LEOSatelliteHelper::LEOSatelliteHelper()
     : m_paused (true)
{
   NS_LOG_FUNCTION (this);
}

// Sucharitha: Make sure that function aborts with invalid values
LEOSatelliteHelper::LEOSatelliteHelper (const LEOSatPolarPos &polarPos)
{
  m_pos = convertPolarToSpherical(polarPos);
}

// add by zg, order to allocate position of satellite by positionAllocator
// 已废弃
void
LEOSatelliteHelper::SetPos(const Vector &position)
{
//	std::cout <<"+++"<<cos(1.0471975511965976)<< std::endl; cos(1.0471975511965976)=0.5代表cos函数计算的是弧度制，而不是角度制
	LEOSatSphericalPos sphericalPos;
	double my_r = sqrt(position.x*position.x+position.y*position.y+position.z*position.z);
	double my_theta = acos(position.z/my_r);
	double my_phi = acos(position.x/my_r/sin(my_theta));
	sphericalPos.r = my_r;
	sphericalPos.theta = my_theta;
	sphericalPos.phi = my_phi;
//	std::cout << "-------LEOSatelliteHelper::SetPos   x=" << position.x << ", y=" << position.y << ", z=" << position.z << ",r="<<my_r<<",theta="<<my_theta<<",phi="<<my_phi<< std::endl;
	// 倾斜度先固定是60度
	//弧度与角度转换公式：
	//	1° = π / 180 ≈ 0.01745 rad
	//	1rad = 180 / π = 57.30°
	// 60度 -------------> 1.0471975511965976
	sphericalPos.inclination = 1.0471975511965976;
	double num = sphericalPos.r * sphericalPos.r * sphericalPos.r;
	sphericalPos.period = 2 * PI * sqrt(num/MU);//万有引力方程
	m_pos = sphericalPos;

    m_lastUpdate = Simulator::Now();
}

// modify
void
LEOSatelliteHelper::SetPos(const LEOSatPolarPos& pos)
{
    m_pos = convertPolarToSpherical(pos);
	Ptr<LEOSatelliteMobilityModel> mobility = m_pos.self->GetObject<LEOSatelliteMobilityModel> ();
	bool enableRouting = mobility->getEnableRouting();
	if(enableRouting == true){
		// 如果路由启动的情况下，则增加计算进入、离开南北极圈的时间，后面SWS路由算法会用到
	    LEOSatSphericalPos temp = m_pos;
		conculateTime(temp);
	}
    m_lastUpdate = Simulator::Now();
}

void
LEOSatelliteHelper::conculateTime(LEOSatSphericalPos temp)
{
	Vector lastPos = getCoordinateBySatSpher(temp);
	// 周期T
	double T = m_pos.period;
	// 把360度分成T*100份，计算每一份是否是进入或者离开极圈的（快照的思路）,一份的时间就是0.01s
	// 步长
	double step = 2*PI/(T*100);
	double time = 0;
	while(time < T){
		time = time + 0.01;
		bool isLastPosInPolarRegion = isInPolarRegion2(temp);
		temp.theta = temp.theta + step;
		bool iscurrentPosInPolarRegion = isInPolarRegion2(temp);
		Vector currentPos = getCoordinateBySatSpher(temp);
		// 如果两者的状态不一致，则说明必定发生了进入或离开极圈这个事情
		if(isLastPosInPolarRegion ^ iscurrentPosInPolarRegion){
//			std::cout << "发生了状态变化" << std::endl;

			//北半球且是进入北极圈
			if(lastPos.z > 0 && lastPos.z < currentPos.z){
				if(time > T) time = T; // 如果超过周期了，则设为周期的最后一秒(int 和 double的差距)
				enterNorthPoleTime = time;
			}
			//北半球且是离开北极圈
			if(lastPos.z > 0 && lastPos.z > currentPos.z){
				if(time > T) time = T;
				leaveNorthPoleTime = time;
			}
			//南半球且是进入南极圈
			if(lastPos.z < 0 && lastPos.z > currentPos.z){
				if(time > T) time = T;
				enterSorthPoleTime = time;
			}
			//南半球且是离开南极圈
			if(lastPos.z < 0 && lastPos.z < currentPos.z){
				if(time > T) time = T;
				leaveSorthPoleTime = time;
			}
		}
		lastPos = currentPos;
	}
	//std::cout << "节点编号：" << m_pos.index << " 4个值： " << enterNorthPoleTime << " " << leaveNorthPoleTime << " " << enterSorthPoleTime << " " << leaveSorthPoleTime <<std::endl;
}

LEOSatSphericalPos
LEOSatelliteHelper::GetCurrentPos(void) const
{
  NS_LOG_FUNCTION (this);
  UpdateOnlyComputeCurPos();
  return m_pos;
}

Vector
LEOSatelliteHelper::GetVelocity (void) const
{
  NS_LOG_FUNCTION (this);
  return m_paused ? Vector (0.0, 0.0, 0.0) : m_velocity;
}

// modify by zg
void
LEOSatelliteHelper::Update (void) const
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now();
  NS_ASSERT(m_lastUpdate <= now);
  Time deltaTime = now - m_lastUpdate;
  m_lastUpdate = now;
  double deltaS = deltaTime.GetSeconds();
  // 防止循环调用
  if (deltaS == 0.0){
	  return;
  }
  if (m_paused)
  {
    return;
  }
  m_pos = computeCurPos(deltaS);
  if(isFault != true) {
		Ptr<LEOSatelliteMobilityModel> mobility = m_pos.self->GetObject<LEOSatelliteMobilityModel> ();
		bool enableRouting = mobility->getEnableRouting();
		if(enableRouting == true){
			  handle();// 设置接口权值，设置链路通断，调用计算路由表(如果是节点之间没有建立链路的例子，如OneWeb、StartLink等星座，则这行不应该被调用，否则会报错)
		}
  }
}

void
LEOSatelliteHelper::UpdateOnlyComputeCurPos (void) const
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now();
  NS_ASSERT(m_lastUpdate <= now);
  Time deltaTime = now - m_lastUpdate;
  m_lastUpdate = now;
  double deltaS = deltaTime.GetSeconds();
  // 防止循环调用
  if (deltaS == 0.0){
	  return;
  }
  if (m_paused)
  {
    return;
  }
  m_pos = computeCurPos(deltaS);
  if(isFault != true) {
		Ptr<LEOSatelliteMobilityModel> mobility = m_pos.self->GetObject<LEOSatelliteMobilityModel> ();
		bool enableRouting = mobility->getEnableRouting();
		if(enableRouting == true){
			  handle();// 设置接口权值，设置链路通断，调用计算路由表(如果是节点之间没有建立链路的例子，如OneWeb、StartLink等星座，则这行不应该被调用，否则会报错)
		}
  }
}

std::vector< Ptr<Node> >
LEOSatelliteHelper::SWSMainDeal(Ptr<Node> dest) {
	std::cout << "时间: "<< Simulator::Now ().GetSeconds() << "s  SWSMainDeal" << std::endl;
	W.clear();
	std::vector< Ptr<Node> > path = findPathFromNix(dest);  // 从s到d的原始路径
	std::cout<<"原始：  ";
	for (uint i=0;i<path.size();i++){
		std::cout<<path[i]->GetId()<<" ";
	}
	std::cout<<std::endl;

	bool flag = isLinkInterruption(dest); //如果出现将断链路
	if (flag) {
		jiangDuanLinkDeal(dest);
		// 新路径，测试
		path = findPathFromNix(dest);
		std::cout<<"出现链路故障：  ";
		for (uint i=0;i<path.size();i++){
			std::cout<<path[i]->GetId()<<" ";
		}
		std::cout<<std::endl;
	}

	flag = false;// 航点是否发生故障的标志
	if(W.find("src")->second.size() > 0) {
		Ptr<Node> waypoint1 = W.find("src")->second[0];
		Ptr<LEOSatelliteMobilityModel> model = waypoint1->GetObject<LEOSatelliteMobilityModel> ();
		flag = model->m_helper.isFault;
	}
	if(W.find("des")->second.size() > 0) {
		Ptr<Node> waypoint2 = W.find("des")->second[0];
		Ptr<LEOSatelliteMobilityModel> model = waypoint2->GetObject<LEOSatelliteMobilityModel> ();
		flag = flag || model->m_helper.isFault;
	}
	if(flag){
		wayPointFaultDeal(dest);//航点故障卫星处理逻辑
		// 新路径，测试
		path = findPathFromNix(dest);

		std::cout<<"航点故障卫星：  ";
		for (uint i=0;i<path.size();i++){
			std::cout<<path[i]->GetId()<<" ";
		}
		std::cout<<std::endl;
	}

	int faultSatelliteIndex = -1;// 发生故障的卫星编号，假设只有一个卫星发生故障
	path = findPathFromNix(dest);//通过W构造的最新路径
	// 假设src和dest不会出现故障
	for(uint i = 1; i < path.size()-1; i++){
		Ptr<Node> currentNode = path[i];
		Ptr<LEOSatelliteMobilityModel> currentNode_model = currentNode->GetObject<LEOSatelliteMobilityModel> ();
		// 该卫星出现故障
		if(currentNode_model->m_helper.isFault){
			faultSatelliteIndex = i;
			break;
		}
	}
	if(faultSatelliteIndex != -1){
		satelliteFaultDeal(dest, faultSatelliteIndex);// 非航点卫星故障处理逻辑
		// 新路径，测试
		path = findPathFromNix(dest);
		std::cout<<"非航点故障卫星：  ";
		for (uint i=0;i<path.size();i++){
			std::cout<<path[i]->GetId()<<" ";
		}
		std::cout<<std::endl;
	}
//	std::cout<<"时间: "<< Simulator::Now ().GetSeconds()<<"s:  ";
	path = findPathFromNix(dest);
//	for (uint i=0;i<path.size();i++){
//		std::cout<<path[i]->GetId()<<" ";
//	}
//	std::cout<<std::endl;
	return path;
}

std::vector< Ptr<Node> >
LEOSatelliteHelper::findPathFromNix(Ptr<Node> dest){
	// 新路径
	Ptr<Node> src = m_pos.self;
	Ptr<Ipv4NixVectorRouting> rp = src->GetObject<Ipv4NixVectorRouting> ();
	std::vector< Ptr<Node> > path;
	std::vector< Ptr<Node> > W_temp;
	W_temp.push_back(src);
	// src对应的航点最多一个
	if(W.find("src")->second.size() > 0) {
		Ptr<Node> waypoint1 = W.find("src")->second[0];
		W_temp.push_back(waypoint1);
	}
	// 中间故障节点对应多个航点
	if(W.find("mid")->second.size() > 0) {
		std::vector< Ptr<Node> > temp = W.find("mid")->second;
		for(uint i = 0; i < temp.size(); i++){
			W_temp.push_back(temp[i]);
		}
	}
	// des对应一个航点
	if(W.find("des")->second.size() > 0) {
		Ptr<Node> waypoint2 = W.find("des")->second[0];
		W_temp.push_back(waypoint2);
	}
	W_temp.push_back(dest);
	// 调用nix生成路径
	for(uint i=0; i<W_temp.size()-1;i++){
		Ptr<Node> from = W_temp[i];
		Ptr<Node> to = W_temp[i+1];
		std::vector< Ptr<Node> > path_temp = rp->GetShortPath(from, to);
		for(uint i = 0; i < path_temp.size()-1; i++){
			path.push_back(path_temp[i]);
		}
	}
	path.push_back(dest);
	return path;
}


void
LEOSatelliteHelper::setIsFault(bool flag){
	Ptr<Node> self = m_pos.self;
	Ptr<LEOSatelliteMobilityModel> mobility = self->GetObject<LEOSatelliteMobilityModel> ();
	int routingAlgorithmNumber = mobility->getRoutingAlgorithmNumber();
	isFault = flag;
	// 如果是故障卫星，且路由算法不是nix，则关闭该卫星的所有链路
	if(routingAlgorithmNumber !=2){
		if(isFault){
			closeLink();
		} else {
			Update(); // 如果故障卫星恢复，则链路也恢复，内部调用handle函数
		}
	}
	// 如果是nix路由算法，则不做设置，在SWS算法中解决这问题，因为如果设置了，nix路由计算路径直接会跳过这个故障节点
}

void
LEOSatelliteHelper::wayPointFaultDeal(Ptr<Node> dest){
	W.clear();
	Ptr<Node> src = m_pos.self;
	bool srcMoveFlag = moveDirection(src);
	bool destMoveFlag = moveDirection(dest);
//	std::cout<< "srcMoveFlag: " << std::boolalpha <<srcMoveFlag << " destMoveFlag: " << std::boolalpha <<destMoveFlag<< std::endl;

	// 如果运动方向一致
	if (srcMoveFlag == destMoveFlag) {
		Ptr<Node> g = findFirstNodeByV(src);
		std::vector<Ptr<Node>> temp1;
		temp1.push_back(g);
		W["src"] = temp1;

		Ptr<Node> h = findFirstNodeByV(dest);
		std::vector<Ptr<Node>> temp2;
		temp2.push_back(h);
		W["des"] = temp2;

	} else {
		// 如果方向不一致

		// src为启航节点
		bool srcQihangFlag = isQiHangNode(src);
		if(srcQihangFlag) {
			Ptr<Node> waypoint = findSafeNode(src);
			std::vector<Ptr<Node>> temp3;
			temp3.push_back(waypoint);
			W["src"] = temp3;
		}

		// 从dest沿两个方向找到和src运动方向不一致穿越极区的第一个节点，取到src跳数较小的那个点
		Ptr<Node> ddest = findFirstNodeOutOfPolarRegionByDoubleV(dest, src);
		bool ddestQihangFlag = isQiHangNode(ddest);
		if(ddestQihangFlag) {
			Ptr<Node> waypoint = findSafeNode(ddest);
			std::vector<Ptr<Node>> temp;
			temp.push_back(waypoint);
			W["des"] = temp;
		} else {
			std::vector<Ptr<Node>> temp;
			temp.push_back(ddest);
			W["des"] = temp;
		}
	}
	//测试，遍历航点集合
	std::map<std::string, std::vector<Ptr<Node>>>::iterator iter;
	for(iter = W.begin(); iter != W.end(); iter++) {
		std::cout << iter->first << ": ";
		std::vector<Ptr<Node>> temp = iter->second;
		for(uint i=0;i<temp.size();i++){
			std::cout<< temp[i]->GetId() << " " ;
		}
		std::cout<<std::endl;
	}
}


void
LEOSatelliteHelper::satelliteFaultDeal(Ptr<Node> dest, int faultSatelliteIndex){
	std::vector< Ptr<Node> > path = findPathFromNix(dest);//通过W构造的最新路径
	Ptr<Node> preNode = path[faultSatelliteIndex-1];
	Ptr<Node> curNode = path[faultSatelliteIndex];
	Ptr<Node> nextNode = path[faultSatelliteIndex+1];
//	std::cout<< preNode->GetId() << " " ;
//	std::cout<< curNode->GetId() << " " ;
//	std::cout<< nextNode->GetId() << " " ;
	bool flag = isInflectionPoint(preNode, curNode, nextNode);
	// 如果是拐点
	if(flag){
		std::vector<Ptr<Node>> temp = findWayPointByInflectionPoint1(preNode, curNode, nextNode);
		W["mid"] = temp;
	} else {
		//如果不是拐点
		std::vector<Ptr<Node>> temp = findWayPointByInflectionPoint2(preNode, curNode, nextNode);
		W["mid"] = temp;
	}
}

std::vector< Ptr<Node> >
LEOSatelliteHelper::findWayPointByInflectionPoint1(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next){
	std::vector< Ptr<Node> > temp;
	Ptr<LEOSatelliteMobilityModel> pre_model = pre->GetObject<LEOSatelliteMobilityModel> ();
	Ptr<LEOSatelliteMobilityModel> cur_model = cur->GetObject<LEOSatelliteMobilityModel> ();
//	int pre_cur = 0;//1:向上；2:向下；3:向左；4:向右
//	if(cur_model->m_helper.m_pos.down->GetId() == pre->GetId()){
//		pre_cur = 1;
//	} else if(cur_model->m_helper.m_pos.up->GetId() == pre->GetId()){
//		pre_cur = 2;
//	} else if(cur_model->m_helper.m_pos.right!=NULL && cur_model->m_helper.m_pos.right->GetId() == pre->GetId()){
//		pre_cur = 3;
//	} else if(cur_model->m_helper.m_pos.left!=NULL && cur_model->m_helper.m_pos.left->GetId() == pre->GetId()){
//		pre_cur = 4;
//	}

	int cur_next = 0;//1:向上；2:向下；3:向左；4:向右
	if(cur_model->m_helper.m_pos.up->GetId() == next->GetId()){
		cur_next = 1;
	} else if(cur_model->m_helper.m_pos.down->GetId() == next->GetId()){
		cur_next = 2;
	} else if(cur_model->m_helper.m_pos.left!=NULL && cur_model->m_helper.m_pos.left->GetId() == next->GetId()){
		cur_next = 3;
	} else if(cur_model->m_helper.m_pos.right!=NULL && cur_model->m_helper.m_pos.right->GetId() == next->GetId()){
		cur_next = 4;
	}

	if(cur_next == 0){
		NS_ASSERT(NULL); // 程序不应该走到这儿,说明pre,cur,next不是通过一条链路连接的
	}
	if(cur_next == 1) {
		temp.push_back(pre_model->m_helper.m_pos.up);
		return temp;
	} else if(cur_next == 2) {
		temp.push_back(pre_model->m_helper.m_pos.down);
		return temp;
	} else if(cur_next == 3) {
		Ptr<Node> pre_left = pre_model->m_helper.m_pos.left;
		if(pre_left != NULL) {
			temp.push_back(pre_left);
			return temp;
		} else {
			std::cout<< "构成不了口型的环"<<std::endl;
			NS_ASSERT(NULL);
		}
	} else {
		Ptr<Node> pre_right = pre_model->m_helper.m_pos.right;
		if(pre_right != NULL) {
			temp.push_back(pre_right);
			return temp;
		} else {
			std::cout<< "构成不了口型的环"<<std::endl;
			NS_ASSERT(NULL);
		}
	}
	return temp;
}

// 形参的3个卫星是否存在故障
bool isFaultSatellite(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next){
	Ptr<LEOSatelliteMobilityModel> pre_model = pre->GetObject<LEOSatelliteMobilityModel> ();
	Ptr<LEOSatelliteMobilityModel> cur_model = cur->GetObject<LEOSatelliteMobilityModel> ();
	Ptr<LEOSatelliteMobilityModel> next_model = next->GetObject<LEOSatelliteMobilityModel> ();
	if(pre_model->m_helper.isFault) return true;
	if(cur_model->m_helper.isFault) return true;
	if(next_model->m_helper.isFault) return true;
	return false;

}

std::vector< Ptr<Node> >
LEOSatelliteHelper::findWayPointByInflectionPoint2(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next){
	std::vector< Ptr<Node> > temp;
	Ptr<LEOSatelliteMobilityModel> pre_model = pre->GetObject<LEOSatelliteMobilityModel> ();
	Ptr<LEOSatelliteMobilityModel> cur_model = cur->GetObject<LEOSatelliteMobilityModel> ();
	Ptr<LEOSatelliteMobilityModel> next_model = next->GetObject<LEOSatelliteMobilityModel> ();
	int pre_cur_next = 0;//1:向上；2:向下；3:向左；4:向右
	if(cur_model->m_helper.m_pos.up->GetId() == next->GetId()){
		pre_cur_next = 1;
	} else if(cur_model->m_helper.m_pos.down->GetId() == next->GetId()){
		pre_cur_next = 2;
	} else if(cur_model->m_helper.m_pos.left!=NULL && cur_model->m_helper.m_pos.left->GetId() == next->GetId()){
		pre_cur_next = 3;
	} else if(cur_model->m_helper.m_pos.right!=NULL && cur_model->m_helper.m_pos.right->GetId() == next->GetId()){
		pre_cur_next = 4;
	}
	if(pre_cur_next == 0){
		std::cout<< "findWayPointByInflectionPoint2 出现bug！"<<std::endl;
		NS_ASSERT(NULL); // 程序不应该走到这儿,说明pre,cur,next不是通过一条链路连接的
	}

	// 这里没有补充是否存在故障卫星的逻辑
	if(pre_cur_next == 1 || pre_cur_next == 2) {
		// 构造左边的“日“子型的环
		if(pre_model->m_helper.m_pos.isLeftSatelliteConnection == true
				&& cur_model->m_helper.m_pos.isLeftSatelliteConnection == true
				&& next_model->m_helper.m_pos.isLeftSatelliteConnection == true){
			temp.push_back(pre_model->m_helper.m_pos.left);
			temp.push_back(next_model->m_helper.m_pos.left);
			return temp;
		}
		// 构造右边边的“日“子型的环
		if(pre_model->m_helper.m_pos.isRightSatelliteConnection == true
				&& cur_model->m_helper.m_pos.isRightSatelliteConnection == true
				&& next_model->m_helper.m_pos.isRightSatelliteConnection == true){
			temp.push_back(pre_model->m_helper.m_pos.right);
			temp.push_back(next_model->m_helper.m_pos.right);
			return temp;
		}
	}
	if(pre_cur_next == 3 || pre_cur_next == 4) {
		// 构造上边的“日“子型的环
		Ptr<Node> pre_up = pre_model->m_helper.m_pos.up;
		Ptr<Node> next_up = next_model->m_helper.m_pos.up;
		Ptr<LEOSatelliteMobilityModel> pre_up_model = pre_up->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<LEOSatelliteMobilityModel> next_up_model = next_up->GetObject<LEOSatelliteMobilityModel> ();
		// 保证上方的卫星存在左右链路且不存在故障卫星
		if((pre_cur_next == 3 && !isFaultSatellite(pre_up, next_up, pre_up_model->m_helper.m_pos.left) && pre_up_model->m_helper.m_pos.isLeftSatelliteConnection == true && next_up_model->m_helper.m_pos.isRightSatelliteConnection == true)
				|| (pre_cur_next == 4 && !isFaultSatellite(pre_up, next_up, pre_up_model->m_helper.m_pos.right) &&  pre_up_model->m_helper.m_pos.isRightSatelliteConnection == true && next_up_model->m_helper.m_pos.isLeftSatelliteConnection == true)){
			temp.push_back(pre_up);
			temp.push_back(next_up);
			return temp;
		}

		// 构造下边的“日“子型的环
		Ptr<Node> pre_down = pre_model->m_helper.m_pos.down;
		Ptr<Node> next_down = next_model->m_helper.m_pos.down;
		Ptr<LEOSatelliteMobilityModel> pre_down_model = pre_down->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<LEOSatelliteMobilityModel> next_down_model = next_down->GetObject<LEOSatelliteMobilityModel> ();
		// 保证下方的卫星存在左右链路且不存在故障卫星
		if((pre_cur_next == 3  && pre_down_model->m_helper.m_pos.isLeftSatelliteConnection == true && next_down_model->m_helper.m_pos.isRightSatelliteConnection == true)
				|| (pre_cur_next == 4 && pre_down_model->m_helper.m_pos.isRightSatelliteConnection == true && next_down_model->m_helper.m_pos.isLeftSatelliteConnection == true)){
			temp.push_back(pre_down);
			temp.push_back(next_down);
			return temp;
		}
//		if((pre_cur_next == 3 && !isFaultSatellite(pre_down, next_down, pre_down_model->m_helper.m_pos.left) && pre_down_model->m_helper.m_pos.isLeftSatelliteConnection == true && next_down_model->m_helper.m_pos.isRightSatelliteConnection == true)
//				|| (pre_cur_next == 4 && !isFaultSatellite(pre_down, next_down, pre_down_model->m_helper.m_pos.right) && pre_down_model->m_helper.m_pos.isRightSatelliteConnection == true && next_down_model->m_helper.m_pos.isLeftSatelliteConnection == true)){
//			temp.push_back(pre_down);
//			temp.push_back(next_down);
//			return temp;
//		}

	}
	std::cout<< "构成不了日型的环"<<std::endl;
	NS_ASSERT(NULL);
	return temp;

}



bool
LEOSatelliteHelper::isInflectionPoint(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next){
	bool pre_cur;
	Ptr<LEOSatelliteMobilityModel> cur_model = cur->GetObject<LEOSatelliteMobilityModel> ();
	if(cur_model->m_helper.m_pos.up->GetId() == pre->GetId() || cur_model->m_helper.m_pos.down->GetId() == pre->GetId()){
		pre_cur = true; // true代表上下方向
	} else{
		pre_cur = false; // false代表左右方向
	}
	bool cur_next;
	if(cur_model->m_helper.m_pos.up->GetId() == next->GetId() || cur_model->m_helper.m_pos.down->GetId() == next->GetId()){
		cur_next = true; // true代表上下方向
	} else{
		cur_next = false; // false代表左右方向
	}
	if(cur_next == pre_cur) return false; // 方向一致，不是拐点
	else return true;
}



Ptr<Node>
LEOSatelliteHelper::findFirstNodeByV(Ptr<Node> node){
	bool moveDir = moveDirection(node);// s的卫星运动方向
	Ptr<Node> currentNode = node;
	Ptr<Node> upNodeOutOfPR; // 从s往上走的第一个出极区且和s运动方向不一致的节点(卫星的运动方向)
	while(true){
		Ptr<LEOSatelliteMobilityModel> currentNode_model = currentNode->GetObject<LEOSatelliteMobilityModel> ();
		Ptr<Node> nextNode = currentNode_model->m_helper.m_pos.up;
//		std::cout<< nextNode->GetId() << "+1+"<<std::endl;
		bool currentMoveDirection = moveDirection(nextNode);
		if(currentMoveDirection != moveDir) { // 如果下一个卫星的运动方向和s的运动方向不一致
			bool flag = isInPolarRegion2(nextNode);
			if(!flag) {
				return nextNode; // 如果不在极区，说明下一个卫星是穿越极区的第一个卫星节点
			}
		}
		currentNode = nextNode;
	}
}

// 关闭该卫星的所有链路
void
LEOSatelliteHelper::closeLink()
{
	// 设置isLeftSatelliteConnection参数
	m_pos.isLeftSatelliteConnection = false;
	m_pos.isRightSatelliteConnection = false;


	Ptr<Ipv4> ipv4_self = m_pos.self->GetObject<Ipv4> ();
	Ptr<Ipv4> ipv4_before = m_pos.up->GetObject<Ipv4> ();
	Ptr<Ipv4> ipv4_after = m_pos.down->GetObject<Ipv4> ();

	int afterInterfaceIndex = 1;//下面的接口编号
	int beforeInterfaceIndex = 2;//上面的接口编号
	int leftInterfaceIndex = 3;//左边的接口编号
	int rightInterfaceIndex = 4;//右边的接口编号

	// 如果当前卫星是每一个轨道的第一个卫星，则上面的接口编号是1，下面的是2；其他的卫星都是下面的接口编号是1，上面的是2；
	if(m_pos.index % m_pos.satelliteNumInOnePlane == 0) {
		beforeInterfaceIndex = 1;
		afterInterfaceIndex = 2;
	}
	// 如果左边没有卫星，则接口编号3是右边的ip接口，如果右边没有接口，则用不到rightInterfaceIndex了，不需要管他
	if(m_pos.left==NULL){
		rightInterfaceIndex = 3;
	}

	// 断开上边卫星的链路
	ipv4_self->SetDown(beforeInterfaceIndex);
	// 如果当前卫星上面的卫星是每一个轨道的第一个卫星
	if((m_pos.index + 1) % m_pos.satelliteNumInOnePlane == 0){
		ipv4_before->SetDown(2);
	} else{
		ipv4_before->SetDown(1);
	}

	// 断开下边卫星的链路
	ipv4_self->SetDown(afterInterfaceIndex);
	// 如果当前卫星下面的卫星是每一个轨道的第一个卫星
	if((m_pos.index - 1) % m_pos.satelliteNumInOnePlane == 0){
		ipv4_after->SetDown(1);
	} else{
		ipv4_after->SetDown(2);
	}

	// 断开左边卫星的链路
	// 如果左卫星存在
    if(m_pos.left!=NULL){
    	Ptr<Ipv4> ipv4_left = m_pos.left->GetObject<Ipv4> ();
    	int lefttNodeRightInterfaceIndex = 4;//左边卫星的右边接口编号
    	// 如果当前轨道编号是1，则左边卫星的轨道编号是0，则左边卫星的右边接口编号是3
    	if(m_pos.plane == 1){
    		lefttNodeRightInterfaceIndex = 3;
    	}
    	ipv4_self->SetDown (leftInterfaceIndex);
    	ipv4_left->SetDown(lefttNodeRightInterfaceIndex);
    }
	// 断开右边卫星的链路
	// 如果右卫星存在
    if(m_pos.right!=NULL){
    	Ptr<Ipv4> ipv4_right = m_pos.right->GetObject<Ipv4> ();
    	int rightNodeInterfaceIndex = 3;//左边卫星的右边接口编号
    	ipv4_self->SetDown (rightInterfaceIndex);
    	ipv4_right->SetDown(rightNodeInterfaceIndex);
    }
}

void
LEOSatelliteHelper::Pause (void)
{
   NS_LOG_FUNCTION (this);
   m_paused = true;
}


void
LEOSatelliteHelper::Unpause (void)
{
   NS_LOG_FUNCTION (this);
   m_paused = false;
}
 
} // namespace ns3

