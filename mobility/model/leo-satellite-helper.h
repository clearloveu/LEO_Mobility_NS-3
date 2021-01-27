#ifndef LEO_SATELLITE_HELPER_H
#define LEO_SATELLITE_HELPER_H
 
#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "leo-satellite-helper.h"
#include "mobility-model.h"
#include <map>
#include <string>

namespace ns3 {

//struct LEOSatPolarPos;
//struct  LEOSatSphericalPos;

class LEOSatelliteHelper
{
public:
   mutable LEOSatSphericalPos m_pos; //卫星的轨道六要素和拓扑特性

   LEOSatelliteHelper();
   LEOSatelliteHelper(const LEOSatPolarPos &polarPos);
   void SetPos(const LEOSatPolarPos &polarPos);
   void SetPos(const Vector &position);
   LEOSatSphericalPos GetCurrentPos(void) const;
   Vector GetVelocity (void) const;
   void Pause (void); // 暂停移动，未使用到
   void Unpause (void); // 继续运动，未使用到
   void Update (void) const; // 更新卫星位置，调用computeCurPos、handle函数
   LEOSatSphericalPos computeCurPos(double timeAdvance) const; // 计算卫星当前位置
   LEOSatSphericalPos convertPolarToSpherical(const LEOSatPolarPos &polarPos);// 极坐标转换成球坐标，未使用到

   double getBeforeDistance(void) const;// 获得本卫星和上方卫星的距离
   double getAfterDistance(void) const;// 获得本卫星和下方卫星的距离
   double getLeftDistance(void) const;// 获得本卫星和左边卫星的距离
   double getRightDistance(void) const;// 获得本卫星和右边卫星的距离
   double getEnterNorthPoleTime(void) const;// 获得进入北极圈的时间（以第一个周期的时间为例）
   double getLeaveNorthPoleTime(void) const;// 离开北极圈
   double getEnterSorthPoleTime(void) const;// 进入南极圈
   double getLeaveSorthPoleTime(void) const;// 离开南极圈
   Vector getCoordinateBySatSpher(LEOSatSphericalPos pos) const;// 根据球形坐标获得3维坐标


   bool isInOrNearPolarRegion(void) const;// 判断当前卫星是否在极圈内部或者在极圈边缘,在分段路由算法中会被调用(废弃)
   Ptr<Node> findAfterNodeNotInPolarRegion(void) const;//在调用isInOrNearPolarRegion得到true的情况下，调用此函数将得到和该卫星同一轨道且向下的第一个不在极圈的卫星(废弃)
   Ptr<Node> findBeforeNodeNotInPolarRegion(void) const;//在调用isInOrNearPolarRegion得到true的情况下，调用此函数将得到和该卫星同一轨道且向上的第一个不在极圈的卫星(废弃)



   // key有3种取值src，des，mid；src对应的value是源节点对应的航点集合，des对应的value是目的节点对应的航点集合，mid对应的value是中间某些故障节点对应的航点集合
   std::map<std::string, std::vector<Ptr<Node>>> W; // 航点集合
   bool isFault = false; // 标记是否是故障卫星

   std::vector< Ptr<Node> > SWSMainDeal(Ptr<Node> dest); // SWS算法主程序
   void setIsFault(bool flag); // 设置isFault参数，并根据isFault对链路进行设置（第二个参数详见具体逻辑）





private:
   mutable Time m_lastUpdate; //!< time of last update
   Vector m_velocity; //!< state variable for velocity
   bool m_paused = false;  //!< state variable for paused
   double enterNorthPoleTime = 0; // 该卫星进入北极区的时间
   double leaveNorthPoleTime = 0; // 该卫星离开北极区的时间
   double enterSorthPoleTime = 0;
   double leaveSorthPoleTime = 0;


   // 移动模型内部调用的一些函数
   void handle() const; //!< 设置接口权值，设置链路通断，调用计算路由表
   bool isInPolarRegion1(double theta) const; //!< 判断卫星是否在极地区域(通过和近地点的角度)，废弃
   bool isInPolarRegion2(LEOSatSphericalPos m_pos) const; //!< 判断卫星是否在极地区域(通过纬度)
   bool isInPolarRegion2(Ptr<Node> node) const; // 和上面不同的是，这个函数会更新当前卫星的位置，因为快照的原因，位置信息不准确
   bool isInOrNearPolarRegion(Ptr<Node> node) const;// 判断卫星是否在极圈内部或者在极圈边缘
   void conculateTime(LEOSatSphericalPos temp); // 用来计算enterNorthPoleTime、leaveNorthPoleTime、enterSorthPoleTime、leaveSorthPoleTime 4个变量
   double getDistanceFromOtherNode(Ptr<Node> node) const; // 根据其他卫星获得本卫星和其他卫星的距离
   void UpdateOnlyComputeCurPos (void) const; // 内部的更新卫星位置函数(因为不是快照切换，所以链路通断状态不会改变)，为了每次内部更新位置时，不纪录位置信息到文件，以免影响python动画（有bug）


   std::vector< Ptr<Node> > findPathFromNix(Ptr<Node> dest);// 论文中主程序流程图中找新路径

   // SWS算法的将断链路处理部分
   bool isLinkInterruption (Ptr<Node> dest) const;// 从s到d的链路是否存在将断链路
   bool moveDirection (Ptr<Node> node) const; // 当前卫星的运动方向，true代表从南向北，false代表从北向南
   // bool packetMoveDirection (Ptr<Node> dest) const; // 当前分组的传输方向，true代表在卫星轨道上逆时针方向，即在近地点处是从南向北，false代表在卫星轨道上顺时针方向，即在近地点处是从北向南
   Ptr<Node> findFirstNodeOutOfPolarRegionByDoubleV(Ptr<Node> src, Ptr<Node> dest) const; // 从src沿两个方向找到和src运动方向不一致穿越极区的第一个节点，取到dest跳数较小的那个点
   bool isQiHangNode(Ptr<Node> node) const; // 判断某个卫星是否是启航节点
   bool isSafeNode(Ptr<Node> node) const; // 判断某个卫星是否是安全节点
   void findWayPoint(Ptr<Node> src, Ptr<Node> ssrc, Ptr<Node> dest); // 寻找航点,其中ssrc为论文中将断链路流程图中的s`
   Ptr<Node> findSafeNode(Ptr<Node> node) const;// 逆着运动方向找到第一个安全节点
   void jiangDuanLinkDeal(Ptr<Node> dest); // 论文中将断链路的处理逻辑

   // SWS算法的故障卫星保护处理逻辑部分
   void closeLink(void); //关闭该卫星的所有链路
   void wayPointFaultDeal(Ptr<Node> dest);// 航点故障处理逻辑
   void satelliteFaultDeal(Ptr<Node> dest, int faultSatelliteIndex);// 非航点故障处理逻辑
   Ptr<Node> findFirstNodeByV(Ptr<Node> node); // 从node出发，沿卫星运动方向，找到第一个出极圈的点
   bool isInflectionPoint(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next); // 判断cur是否是拐点
   std::vector< Ptr<Node> > findWayPointByInflectionPoint1(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next);// 是拐点的情况下
   std::vector< Ptr<Node> > findWayPointByInflectionPoint2(Ptr<Node> pre, Ptr<Node> cur, Ptr<Node> next);// 不是拐点的情况下

};
 
} // namespace ns3
 
#endif /* LEO_SATELLITE_HELPER_H */
