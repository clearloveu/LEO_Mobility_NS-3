/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef MOBILITY_MODEL_H
#define MOBILITY_MODEL_H

#include "ns3/vector.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/node.h"

namespace ns3 {

/**
 * \ingroup mobility
 * \brief Keep track of the current position and velocity of an object.
 *
 * All space coordinates in this class and its subclasses are
 * understood to be meters or meters/s. i.e., they are all
 * metric international units.
 *
 * This is a base class for all specific mobility models.
 */

struct LEOSatPolarPos
{
    double altitude;// 卫星海拔
    double longitude;//升交点赤经，角度制
    double alpha;// 距离近地点的角度，角度制（因为是圆轨道，没有近地点，这里假设近地点是地球正面的赤道）
    double inclination; //倾角，角度制
    int plane; // Orbital plane that this satellite resides in 轨道编号

    int index; //卫星编号，在leo-satellite-helper.cc里面起作用
    int planeNum;// 总的轨道数
    int satelliteNumInOnePlane;// 每一个轨道的卫星数量

    Ptr<Node> self;  // 该卫星
    Ptr<Node> before;  // 该卫星的同轨道中上方相邻卫星
    Ptr<Node> after;   // 该卫星的同轨道中下方相邻卫星
    Ptr<Node> left;    // 该卫星的不同轨道的左边相邻卫星
    Ptr<Node> right;   // 该卫星的不同轨道的左边相邻卫星

};

struct LEOSatSphericalPos
{
   double r;// 半径（模拟圆，代替长半轴和离心率，近地点幅角也没了）(卫星海拔+地球半径)
   double phi;//升交点赤经，弧度制
   double theta;//距离近地点的角度，弧度制（因为是圆轨道，没有近地点，这里假设近地点是地球正面的赤道）,即真近点角
   double inclination;//倾角，弧度制
   double period;// 周期
   int plane;// 轨道编号
   int index; //卫星编号，在leo-satellite-helper.cc里面起作用
   int planeNum;// 总的轨道数
   int satelliteNumInOnePlane;// 每一个轨道的卫星数量

   Ptr<Node> self;  // 该卫星
   Ptr<Node> up;  // 该卫星的同轨道中上方相邻卫星
   Ptr<Node> down;   // 该卫星的同轨道中下方相邻卫星
   Ptr<Node> left;    // 该卫星的不同轨道的左边相邻卫星
   Ptr<Node> right;   // 该卫星的不同轨道的左边相邻卫星
   bool isLeftSatelliteConnection;   // 用来标识其左右相邻卫星是否与该卫星相连（极地处发生连接断开，这里认为卫星只要纬度高于60度就断开连接）
   bool isRightSatelliteConnection;

};

//struct LEOSatSix
//{
//   double r;//长半轴
//   double e;//离心率
//   double inclination;//轨道倾角
//   double omiga;//升交点赤经
//   double w;//近地点幅角
//   double alpha;//平近地点
//};

struct TerminalPolarPos
{
  double latitude;
  double longitude;
};

struct TerminalSphericalPos
{
   double r;
   double theta;
   double phi;
   double period;
};

class MobilityModel : public Object
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  MobilityModel ();
  virtual ~MobilityModel () = 0;

  /**
   * \return the current position
   */
  Vector GetPosition (void) const;
  /**
   * \param position the position to set.
   */
  void SetPosition (const Vector &position);

  LEOSatSphericalPos GetSatSphericalPos(void) const;
  void SetSatSphericalPos(const LEOSatPolarPos &polarPos);

  TerminalSphericalPos GetTermSphericalPos(void) const;
  void SetTermSphericalPos(const TerminalPolarPos &polarPos);

  /**
   * \return the current velocity.
   */
  Vector GetVelocity (void) const;
  /**
   * \param position a reference to another mobility model
   * \return the distance between the two objects. Unit is meters.
   */
  double GetDistanceFrom (Ptr<const MobilityModel> position) const;
  /**
   * \param other reference to another object's mobility model
   * \return the relative speed between the two objects. Unit is meters/s.
   */
  double GetRelativeSpeed (Ptr<const MobilityModel> other) const;
  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);



  /**
   *  TracedCallback signature.
   *
   * \param [in] model Value of the MobilityModel.
   */
  typedef void (* TracedCallback)(Ptr<const MobilityModel> model);
  



protected:
  /**
   * Must be invoked by subclasses when the course of the
   * position changes to notify course change listeners.
   */
  void NotifyCourseChange (void) const;
private:
  /**
   * \return the current position.
   *
   * Concrete subclasses of this base class must 
   * implement this method.
   */
  virtual Vector DoGetPosition (void) const = 0;
  /**
   * \param position the position to set.
   *
   * Concrete subclasses of this base class must 
   * implement this method.
   */
  virtual void DoSetPosition (const Vector &position) = 0;
  /**
   * \return the current velocity.
   *
   * Concrete subclasses of this base class must 
   * implement this method.
   */
  virtual Vector DoGetVelocity (void) const = 0;
  /**
   * The default implementation does nothing but return the passed-in
   * parameter.  Subclasses using random variables are expected to
   * override this.
   * \param start  starting stream index
   * \return the number of streams used
   */
  virtual int64_t DoAssignStreams (int64_t start);
   
  virtual LEOSatSphericalPos DoGetSatSphericalPos(void) const;
  virtual void DoSetSatSphericalPos(const LEOSatPolarPos& polarPos);

  virtual TerminalSphericalPos DoGetTermSphericalPos(void) const;
  virtual void DoSetTermSphericalPos(const TerminalPolarPos &polarPos);

  /**
   * Used to alert subscribers that a change in direction, velocity,
   * or position has occurred.
   */
  ns3::TracedCallback<Ptr<const MobilityModel> > m_courseChangeTrace;

};

} // namespace ns3

#endif /* MOBILITY_MODEL_H */
