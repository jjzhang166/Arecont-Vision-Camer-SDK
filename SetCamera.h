//---------------------------------------------------------------------------
#ifndef SetCameraH
#define SetCameraH
//---------------------------------------------------------------------------
#include "GIdUDPBase.h"
#include "AV2000Types.h"
#include <set>
#include <vector>
//---------------------------------------------------------------------------
struct AV2000AddrInherit : public AV2000Addr{
   AV2000AddrInherit();
   GIdString IpAsString() const;
   GIdString MacAsString() const;
};
//---------------------------------------------------------------------------
class CompareAV2000Addr{
public:
   bool operator()(const AV2000Addr& left, const AV2000Addr& right) const;
};
//---------------------------------------------------------------------------
class SetCamera{
public:
   typedef std::set<AV2000AddrInherit, CompareAV2000Addr> CAMERAS;
private:
   const GIdString ident;
   int port;
   CAMERAS cameras;
private:
   void OnDetectCamera(std::vector<char>&);
   bool RemoveIdent(std::vector<char>& data);
   AV2000AddrInherit ExtractAddr(std::vector<char>&);
public:
   SetCamera();
   unsigned Find(unsigned attempts, unsigned timeout);
   void Set(const AV2000Addr&);
   GIdString AV2000Ident() const;
   const CAMERAS& Cameras() const;
};
//---------------------------------------------------------------------------
#endif
