#include "leo-satellite-mobility-model.h"
#include "ns3/simulator.h"
#include <fstream>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LEOSatelliteMobilityModel);

TypeId LEOSatelliteMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LEOSatelliteMobilityModel")
      .SetParent<MobilityModel> ()
      .SetGroupName ("Mobility")
      .AddConstructor<LEOSatelliteMobilityModel> ();
      return tid;
}

LEOSatelliteMobilityModel::LEOSatelliteMobilityModel ()
{
}
 
LEOSatelliteMobilityModel::~LEOSatelliteMobilityModel ()
{
}

LEOSatSphericalPos
LEOSatelliteMobilityModel::GetSatSphericalPos(void) const
{
  return DoGetSatSphericalPos();
}

LEOSatSphericalPos
LEOSatelliteMobilityModel::DoGetSatSphericalPos(void) const
{
//   m_helper.Update();  // 现在不能在这边调用update函数，因为在update函数中的computeCurPos函数中会计算卫星的相邻卫星的纬度，那里会调用这个函数，如果在这边调用这个函数，会产生死循环
   return m_helper.GetCurrentPos();
}

void
LEOSatelliteMobilityModel::DoSetSatSphericalPos(const LEOSatPolarPos& pos)
{
  m_helper.SetPos(pos);
  m_helper.Unpause();
//  NotifyCourseChange();
}

void
LEOSatelliteMobilityModel::SetSatSphericalPos(const LEOSatPolarPos& polarPos)
{
  DoSetSatSphericalPos(polarPos);
}

//add by zg
void
LEOSatelliteMobilityModel::DoDispose (void)
{
  // chain up
  MobilityModel::DoDispose ();
}
//add by zg
void
LEOSatelliteMobilityModel::DoInitialize (void)
{
    UpdatePosition ();
    MobilityModel::DoInitialize ();
}
//add by zg
void
LEOSatelliteMobilityModel::DoInitializePrivate (void)
{
	// 没有调用
	// 初始化时先通知打印最初位置，然后再安排调度器在下一个时间点执行更新位置操作
//	NotifyCourseChange();
//	Time delay = Seconds (2);
//	m_event = Simulator::Schedule (delay, &LEOSatelliteMobilityModel::UpdatePosition, this);
}

//add by zg
void
LEOSatelliteMobilityModel::UpdatePosition (void)
{
	// 更新位置
	m_helper.Update ();
	// 如果快照方式是固定间隔
	if(selectSnapShotWay == 1) {
		// 本次事件取消，可能为了本次的调度不影响下次调度，参考random-walk-2d-mobility-model.cc
		m_event.Cancel ();
		// 10s更新一次位置
		Time delay = Seconds (10);
		m_event = Simulator::Schedule (delay, &LEOSatelliteMobilityModel::UpdatePosition, this);
	}
	// 回调机制通知打印位置
	NotifyCourseChange();
}

void
LEOSatelliteMobilityModel::setRoutingAlgorithmAndSnapShotWay(int algorithmNum, int snapShotWay)
{
	routingAlgorithmNumber = algorithmNum;
	selectSnapShotWay = snapShotWay;
}

void
LEOSatelliteMobilityModel::setEnableRouting(bool enableRoutingFlag)
{
	enableRouting = enableRoutingFlag;
}

void
LEOSatelliteMobilityModel::setFileName(std::string file_name)
{
	fileName = file_name;
}

int
LEOSatelliteMobilityModel::getRoutingAlgorithmNumber() const
{
	return routingAlgorithmNumber;
}

bool
LEOSatelliteMobilityModel::getEnableRouting() const
{
	return enableRouting;
}

void
LEOSatelliteMobilityModel::NotifyCourseChange(void)
{
	Vector pos = DoGetPosition();

//	std::cout << Simulator::Now ().GetSeconds() << "s, 卫星编号: " << m_helper.m_pos.index << ", x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z <<std::endl;
//	std::cout <<  std::boolalpha<< getIsLeftSatelliteConnection() << " " <<  std::boolalpha<< getIsRightSatelliteConnection() << " " << getTheta() <<std::endl;
//	std::cout << "beforeDistance=" << getBeforeDistance() << ", afterDistance=" << getAfterDistance() << ", leftDistance=" << getLeftDistance() << ", rightDistance=" << getRightDistance() <<std::endl;


    std::ofstream file_writer(fileName, std::ios::app);//打开file_name文件，以ios::app追加的方式输入
//    std::ofstream file_writer(file_name);//打开file_name文件，默认方式是覆盖原文件的内容
    file_writer << "time:" << Simulator::Now () << std::endl;
    file_writer << pos.x << " " << pos.y << " " << pos.z << std::endl;
    // 如果启用了路由，则代表也需要记录此时该卫星的左右连接状态
    if(enableRouting == true) {
    	 file_writer <<std::boolalpha<< getIsLeftSatelliteConnection() << " " <<std::boolalpha<< getIsRightSatelliteConnection() << " " << getTheta() << std::endl;// @suppress("Invalid arguments")
    }
    file_writer.close(); // @suppress("Invalid arguments")
}

Vector
LEOSatelliteMobilityModel::getCoordinateBySatSpher(LEOSatSphericalPos pos) const
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



// modify by zg
Vector
LEOSatelliteMobilityModel::DoGetPosition (void) const
{
//	std::cout << "DoGetPosition!"  << std::endl;
//  m_helper.Update ();
//  NotifyCourseChange();
	return getCoordinateBySatSpher(m_helper.m_pos);

  // 保存是否和相邻卫星保持连接
//  double left;
//  if(m_helper.m_pos.isLeftSatelliteConnection) left = 1;
//  else left = 0;
//  double right;
//  if(m_helper.m_pos.isRightSatelliteConnection) right = 1;
//  else right = 0;
}

//左卫星是否相连
bool
LEOSatelliteMobilityModel::getIsLeftSatelliteConnection (void) const
{
	return m_helper.m_pos.isLeftSatelliteConnection;
}

//右卫星是否相连
bool
LEOSatelliteMobilityModel::getIsRightSatelliteConnection (void) const
{
	return m_helper.m_pos.isRightSatelliteConnection;
}

double
LEOSatelliteMobilityModel::getTheta(void) const
{
	return m_helper.m_pos.theta;
}


// add by zg , order to allocate position of node by PositionAllocator
void
LEOSatelliteMobilityModel::DoSetPosition (const Vector &position)
{
  m_helper.SetPos(position);

}

Vector
LEOSatelliteMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}


} // namespace ns3

