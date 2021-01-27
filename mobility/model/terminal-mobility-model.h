#ifndef TERMINAL_MOBILITY_MODEL_H
#define TERMINAL_MOBILITY_MODEL_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "mobility-model.h"
#include "terminal-helper.h"
 
namespace ns3 {

class TerminalMobilityModel : public MobilityModel
{

public:
   static TypeId GetTypeId (void);
   TerminalMobilityModel ();
   virtual ~TerminalMobilityModel ();
   TerminalHelper m_helper;
   void setSphericalPos(const TerminalPolarPos& polarPos);
   virtual void DoSetTermSphericalPos(const TerminalPolarPos& polarPos);
   virtual TerminalSphericalPos DoGetTermSphericalPos(void) const;
private:
   virtual Vector DoGetPosition (void) const;
   virtual void DoSetPosition (const Vector &position);
   virtual Vector DoGetVelocity (void) const;
};

} //namespace ns3

#endif /* TERMINAL_MOBILITY_MODEL*/

