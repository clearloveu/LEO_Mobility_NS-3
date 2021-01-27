#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include <string>
#include <fstream>


using namespace ns3;

 //Prints actual position and velocity when a course change event occurs
static void
CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition ();
//  Vector vel = mobility->GetVelocity ();
//  std::cout << Simulator::Now () << ", model=" << mobility << ", POS: x=" << pos.x << ", y=" << pos.y
//            << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
//            << ", z=" << vel.z << std::endl;


  std::string file_name = "iridium-topology.txt";
  std::ofstream file_writer(file_name, std::ios::app);//打开file_name文件，以ios::app追加的方式输入
  file_writer << "time:" << Simulator::Now () << std::endl;
  file_writer << pos.x << " " << pos.y << " " << pos.z << std::endl;
  file_writer.close();

}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  const double ALTITUDE = 780;//卫星海拔
  const double INCLINATION = 86.4;//倾角
//  const double TOLERANCE = 10;//未知
  // Format: nodenumber, longitude, alpha and plane.
//  double iridiumConstellation[3][11][4] = {{{0, 0, 0, 0}, {1, 0, 32.73, 0}, {2, 0, 65.45, 0}, {3, 0, 98.18, 0},
//  {4, 0, 130.91, 0}, {5, 0, 163.64, 0}, {6, 0, 196.36, 0},{7, 0, 229.09, 0},
//  {8, 0, 261.82, 0}, {9, 0, 294.55, 0}, {10, 0, 327.27, 0}},
// {{11, 120, 16.36, 1}, {12, 120, 49.09, 1}, {13, 120, 81.82, 1}, {14, 120, 114.55, 1},
//  {15, 120, 147.27, 1}, {16, 120, 180, 1}, {17, 120, 212.73, 1}, {18, 120, 245.45, 1},
//  {19, 120, 278.18, 1}, {20, 120, 310.91, 1}, {21, 120, 343.64, 1}},
// {{22, 240, 0, 2}, {23, 240, 32.73, 2}, {24, 240, 65.45, 2}, {24, 240, 98.18, 2},
//  {25, 240, 130.91, 2}, {26, 240, 163.64, 2}, {27, 240, 196.36, 2}, {28, 240, 229.09, 2},
//  {29, 240, 261.82, 2}, {30, 240, 294.55, 2}, {31, 240, 327.27, 2}}};
  double iridiumConstellation[3][11][4] =  {{{0,0,0.0,0},{1,0,32.73,0},{2,0,65.45,0},{3,0,98.18,0},
		  {4,0,130.91,0},{5,0,163.64,0},{6,0,196.36,0},
		  {7,0,229.09,0},{8,0,261.82,0},{9,0,294.55,0},{10,0,327.27,0}},
		  {{11,120,0.0,1},{12,120,32.73,1},{13,120,65.45,1},{14,120,98.18,1},
	      {15,120,130.91,1},{16,120,163.64,1},{17,120,196.36,1},{18,120,229.09,1},
		  {19,120,261.82,1},{20,120,294.55,1},{21,120,327.27,1}},
		  {{22,240,0.0,2},{23,240,32.73,2},{24,240,65.45,2},{25,240,98.18,2},
		  {26,240,130.91,2},{27,240,163.64,2},{28,240,196.36,2},{29,240,229.09,2},
		  {30,240,261.82,2},{31,240,294.55,2},{32,240,327.27,2}}};


  NodeContainer nodes;
  nodes.Create (33);
  MobilityHelper mobility;



//  //参考：SteadyStateRandomWaypointMobilityModel
//  Ptr<RandomBoxPositionAllocator> m_position = CreateObject<RandomBoxPositionAllocator> ();
//  Ptr<UniformRandomVariable> m_x = CreateObject<UniformRandomVariable> ();
//  Ptr<UniformRandomVariable> m_y = CreateObject<UniformRandomVariable> ();
//  Ptr<ConstantRandomVariable> m_z = CreateObject<ConstantRandomVariable> ();
//  m_x->SetAttribute ("Min", DoubleValue (50));
//  m_x->SetAttribute ("Max", DoubleValue (100));
//  m_y->SetAttribute ("Min", DoubleValue (50));
//  m_y->SetAttribute ("Max", DoubleValue (100));
//  m_z->SetAttribute ("Constant", DoubleValue (100));
//  m_position->SetX (m_x);
//  m_position->SetY (m_y);
//  m_position->SetZ (m_z);
//  mobility.SetPositionAllocator (m_position);

  mobility.SetMobilityModel ("ns3::LEOSatelliteMobilityModel");

//  mobility.Install (nodes);
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
      position->SetSatSphericalPos(pPos); // @suppress("Invalid arguments")
      NS_ASSERT (position != 0);
      index++;
//      Vector pos = position->GetPosition ();
//      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }
  // 触发某种回调机制，在leo-satellite.cc 执行NotifyCourseChange()时会回调上面重写的CourseChange函数
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                   MakeCallback (&CourseChange));

//  std::string file_name = "iridium-topology.txt";
//  std::ofstream file_writer(file_name, std::ios::app);//打开file_name文件，以ios::app追加的方式输入
//  file_writer << "satellite number:" << 33 << std::endl;
//  file_writer << "altitude:" << ALTITUDE << std::endl;
//  file_writer << "inclination:" << INCLINATION << std::endl;
//  file_writer.close();

  Simulator::Stop (Seconds (7000.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

