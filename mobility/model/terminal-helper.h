#ifndef TERMINAL_HELPER_H
#define TERMINAL_HELPER_H
 
#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "terminal-helper.h"
#include "mobility-model.h"

namespace ns3 {



class TerminalHelper
{
public:
   TerminalHelper();
   
   // Take the polar coordinate values as input and set the position in spherical system.
   TerminalHelper(const TerminalPolarPos &polarPos);
   
   void SetPos(const TerminalPolarPos &polarPos);
      
   // Returns the position in spherical system
   TerminalSphericalPos GetCurrentPos(void) const;
 
   Vector GetVelocity (void) const;
   
   // Pause mobility at current position
   void Pause (void);
   
   // Pause mobility at current position
   void Unpause (void);

   // Pause mobility at current position
   void Update (void) const;
   TerminalSphericalPos computeCurPos(double timeAdvance) const;
   TerminalSphericalPos convertPolarToSpherical(const TerminalPolarPos &polarPos);
private:
   mutable Time m_lastUpdate; //!< time of last update
   mutable TerminalSphericalPos m_pos; //!< The position of the satellite in Spherical coordinates.
   Vector m_velocity; //!< state variable for velocity
   bool m_paused;  //!< state variable for paused
};
 
} // namespace ns3
 
#endif /* TERMINAL_HELPER_H */
