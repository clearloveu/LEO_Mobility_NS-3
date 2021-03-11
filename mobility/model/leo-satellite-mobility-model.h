#ifndef LEO_SATELLITE_MOBILITY_MODEL_H
#define LEO_SATELLITE_MOBILITY_MODEL_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "mobility-model.h"
#include "leo-satellite-helper.h"
#include "ns3/event-id.h"

#include <iostream>

 
namespace ns3 {

class LEOSatelliteMobilityModel : public MobilityModel
{

public:
   static TypeId GetTypeId (void);
   LEOSatelliteMobilityModel ();
   virtual ~LEOSatelliteMobilityModel ();
   LEOSatelliteHelper m_helper;
   void SetSatSphericalPos(const LEOSatPolarPos& polarPos);
   LEOSatSphericalPos GetSatSphericalPos(void) const;
   virtual LEOSatSphericalPos DoGetSatSphericalPos(void) const;
   virtual void DoSetSatSphericalPos(const LEOSatPolarPos& polarPos);
   bool getIsLeftSatelliteConnection (void) const;//左卫星是否相连
   bool getIsRightSatelliteConnection (void) const;//右卫星是否相连
   double getTheta(void) const;
   void setRoutingAlgorithmAndSnapShotWay(int algorithmNum, int snapShotWay);
   void setEnableRouting(bool enableRoutingFlag);// 此函数要比SetSatSphericalPos先调用，因为SetSatSphericalPos函数会调用enableRouting变量
   void setFileName(std::string file_name);
   int getRoutingAlgorithmNumber() const;
   bool getEnableRouting() const;


   // 为了保存和左右2个节点是否连接的bool值
   // 之所以重写这个方法，而不是交给父类去调用ns3的回调函数，是因为如果是父类的话，只能在回调函数中执行父类的函数，想要取出子类的值，如果按照java的思想：
   // 1，父类强转子类，再调用方法。（c++禁止）2，父类定义接口，让子类去实现，但是MobilityModel这个类有很多子类，需要给这些子类都去实现，太麻烦。
   // 这样直接在子类输出想要的数据或者保存到txt中
   void NotifyCourseChange(void);
   void UpdatePosition(void);//!< update the position of satellite itself by using Event


private:
   // add by zg , refer to random-walk-2d-mobility-model.h
   virtual void DoDispose (void);
   virtual void DoInitialize (void);
   void DoInitializePrivate (void);
   EventId m_event; //!< stored event ID
   int routingAlgorithmNumber = 2; // 1:全局路由（Ipv4GlobalRoutingHelper::PopulateRoutingTables ()）;  2:分段路由（nix）;  3:OLSR
   int selectSnapShotWay = 1; // 1：按固定间隔进行快照  2：按拓扑状态的变化进行快照，当该变量的值为2时，这个功能由测例的newSnapshotWay函数完成

   // 设计这个变量的原因是因为我写的leo-mobility包含了移动模型和路由模型，而如果测例只有卫星系统的拓扑运动（如OneWeb和StartLink），
   // 路由模块不需要启用（如果启用了，在路由计算处会报错，因为测例没有路由），这个变量就是用来控制路由模块是否启用
   bool enableRouting = true; // true: 启用路由模块；false：不启用路由模块
   std::string fileName = "fileName.txt"; // 记录卫星运动的文件名

   Vector getCoordinateBySatSpher(LEOSatSphericalPos pos) const;// 根据球形坐标获得3维坐标


   virtual Vector DoGetPosition (void) const;
   virtual void DoSetPosition (const Vector &position);
   virtual Vector DoGetVelocity (void) const;

};

} //namespace ns3



#endif /* LEO_SATELLITE_MOBILITY_MODEL*/

