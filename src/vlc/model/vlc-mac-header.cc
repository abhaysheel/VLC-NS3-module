/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */

#include "vlc-mac-header.h"
#include <ns3/address-utils.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VlcMacHeader);

VlcMacHeader::VlcMacHeader ()
{
  SetType (VLC_MAC_DATA);
  SetSecDisable ();
  SetNoFrmPend ();
  SetNoAckReq ();
  SetFrmCtrlRes (0);
  SetDstAddrMode (NOADDR);
  SetSrcAddrMode (NOADDR);
  SetFrameVer (1);
}

VlcMacHeader::VlcMacHeader (enum VlcMacType vlcMacType,
                            uint8_t seqNum)
{
  SetType (vlcMacType);
  SetSeqNum (seqNum);
  SetSecDisable ();
  SetNoFrmPend ();
  SetNoAckReq ();
  SetFrmCtrlRes (0);
  SetDstAddrMode (NOADDR);
  SetSrcAddrMode (NOADDR);
  SetFrameVer (1);
}

VlcMacHeader::~VlcMacHeader ()
{
}

enum VlcMacHeader::VlcMacType
VlcMacHeader::GetType (void) const
{
  switch (m_fctrlFrmType)
    {
    case 0:
      return VLC_MAC_BEACON;
      break;
    case 1:
      return VLC_MAC_DATA;
      break;
    case 2:
      return VLC_MAC_ACKNOWLEDGMENT;
      break;
    case 3:
      return VLC_MAC_COMMAND;
      break;
    default:
      return VLC_MAC_RESERVED;
    }
}

uint16_t
VlcMacHeader::GetFrameControl (void) const
{
  uint16_t val = 0;

  val = m_fctrlFrmVer & (0x00);
  val |= (m_fctrlReserved << 2) & (0x0f << 2);
  val |= (m_fctrlFrmType << 6) & (0x07 <<6);
  val |= (m_fctrlSecU << 9) & (0x01 << 9);
  val |= (m_fctrlFrmPending << 10) & (0x01 << 10);
  val |= (m_fctrlAckReq << 11) & (0x01 << 11);
  val |= (m_fctrlDstAddrMode << 12) & (0x03 << 12);
  val |= (m_fctrlSrcAddrMode << 14) & (0x03 << 14);
  return val;

}

bool
VlcMacHeader::IsSecEnable (void) const
{
  return (m_fctrlSecU == 0);
}

bool
VlcMacHeader::IsFrmPend (void) const
{
  return (m_fctrlFrmPending == 1);
}

bool
VlcMacHeader::IsAckReq (void) const
{
  return (m_fctrlAckReq == 1);
}

uint8_t
VlcMacHeader::GetFrmCtrlRes (void) const
{
  return (m_fctrlReserved);
}

uint8_t
VlcMacHeader::GetDstAddrMode (void) const
{
  return m_fctrlDstAddrMode;
}

uint8_t
VlcMacHeader::GetFrameVer (void) const
{
  return m_fctrlFrmVer;
}

uint8_t
VlcMacHeader::GetSrcAddrMode (void) const
{
  return m_fctrlSrcAddrMode;
}

uint8_t
VlcMacHeader::GetSeqNum (void) const
{
  return(m_SeqNum);
}

uint16_t
VlcMacHeader::GetDstVpanId (void) const
{
  return(m_addrDstVpanId);
}

Mac16Address
VlcMacHeader::GetShortDstAddr (void) const
{
  return(m_addrShortDstAddr);
}

Mac64Address
VlcMacHeader::GetExtDstAddr (void) const
{
  return(m_addrExtDstAddr);
}

uint16_t
VlcMacHeader::GetSrcVpanId (void) const
{
  return(m_addrSrcVpanId);
}
Mac16Address
VlcMacHeader::GetShortSrcAddr (void) const
{
  return(m_addrShortSrcAddr);
}
Mac64Address
VlcMacHeader::GetExtSrcAddr (void) const
{
  return(m_addrExtSrcAddr);
}

// AUXILIARY SECURITY HEADER- START
uint8_t
VlcMacHeader::GetSecControl (void) const
{
  uint8_t val = 0;

  val = m_secctrlSecLevel & (0x00);              // Bit 0-2
  val |= (m_secctrlKeyIdMode << 3) & (0x0 << 3);  // Bit 3-4
  val |= (m_secctrlReserved << 5) & (0x7 << 5);   // Bit 5-7

  //return(val);
  return 0;
}

uint32_t
VlcMacHeader::GetFrmCounter (void) const
{
  return(m_auxFrmCntr);
}

uint8_t
VlcMacHeader::GetSecLevel (void) const
{
  return (m_secctrlSecLevel);
}

uint8_t
VlcMacHeader::GetKeyIdMode (void) const
{
  return(m_secctrlKeyIdMode);
}

uint8_t
VlcMacHeader::GetSecCtrlReserved (void) const
{
  return (m_secctrlReserved);
}

uint32_t
VlcMacHeader::GetKeyIdSrc32 (void) const
{
  return(m_auxKeyIdKeySrc32);
}

uint64_t
VlcMacHeader::GetKeyIdSrc64 (void) const
{

  return(m_auxKeyIdKeySrc64);
}

uint8_t
VlcMacHeader::GetKeyIdIndex (void) const
{
  return(m_auxKeyIdKeyIndex);
}
// AUXILIARY SECURITY HEADER - END

bool
VlcMacHeader::IsBeacon (void) const
{
  return(m_fctrlFrmType == VLC_MAC_BEACON);
}

bool
VlcMacHeader::IsData (void) const
{
  return(m_fctrlFrmType == VLC_MAC_DATA);
}

bool
VlcMacHeader::IsAcknowledgment (void) const
{
  return(m_fctrlFrmType == VLC_MAC_ACKNOWLEDGMENT);
}

bool
VlcMacHeader::IsCommand (void) const
{
  return(m_fctrlFrmType == VLC_MAC_COMMAND);
}

void
VlcMacHeader::SetType (enum VlcMacType vlcMacType)
{
  m_fctrlFrmType = vlcMacType;
}

void
VlcMacHeader::SetFrameControl (uint16_t frameControl)
{
  m_fctrlFrmVer = (frameControl) & (0x00);
  m_fctrlReserved = (frameControl >> 2) & (0x0f);
  m_fctrlFrmType = (frameControl >> 6) & (0x07);
  m_fctrlSecU = (frameControl >> 9) & (0x01);
  m_fctrlFrmPending = (frameControl >> 10) & (0x01);
  m_fctrlAckReq = (frameControl >> 11) & (0x01);
  m_fctrlDstAddrMode = (frameControl >> 12) & (0x03);
  m_fctrlSrcAddrMode = (frameControl >> 14) & (0x03);
 }

void
VlcMacHeader::SetSecEnable (void)
{
  m_fctrlSecU = 1;
}

void
VlcMacHeader::SetSecDisable (void)
{
  m_fctrlSecU = 0;
}

void
VlcMacHeader::SetFrmPend (void)
{
  m_fctrlFrmPending = 1;
}

void
VlcMacHeader::SetNoFrmPend (void)
{
  m_fctrlFrmPending = 0;
}

void
VlcMacHeader::SetAckReq (void)
{
  m_fctrlAckReq = 1;
}

void
VlcMacHeader::SetNoAckReq (void)
{
  m_fctrlAckReq = 0;
}

void
VlcMacHeader::SetFrmCtrlRes (uint8_t res)
{
  m_fctrlReserved = res;
}

void
VlcMacHeader::SetDstAddrMode (uint8_t addrMode)
{
  m_fctrlDstAddrMode = addrMode;
}


void
VlcMacHeader::SetFrameVer (uint8_t ver)
{
  m_fctrlFrmVer = ver;
}


void
VlcMacHeader::SetSrcAddrMode (uint8_t addrMode)
{
  m_fctrlSrcAddrMode = addrMode;
}


void
VlcMacHeader::SetSeqNum (uint8_t seqNum)
{
  m_SeqNum = seqNum;
}

void
VlcMacHeader::SetSrcAddrFields (uint16_t vpanId,
                                   Mac16Address addr)
{
  m_addrSrcVpanId = vpanId;
  m_addrShortSrcAddr = addr;
}

void
VlcMacHeader::SetSrcAddrFields (uint16_t vpanId,
                                   Mac64Address addr)
{
  m_addrSrcVpanId = vpanId;
  m_addrExtSrcAddr = addr;
}

void
VlcMacHeader::SetDstAddrFields (uint16_t vpanId,
                                   Mac16Address addr)
{
  m_addrDstVpanId = vpanId;
  m_addrShortDstAddr = addr;
}
void
VlcMacHeader::SetDstAddrFields (uint16_t vpanId,
                                   Mac64Address addr)
{
  m_addrDstVpanId = vpanId;
  m_addrExtDstAddr = addr;
}
//AUXILIARY SECURITY HEADER - START
void
VlcMacHeader::SetSecControl (uint8_t secControl)
{
  m_secctrlSecLevel = (secControl) & (0x07);            // Bit 0-2
  m_secctrlKeyIdMode = (secControl >> 3) & (0x03);      // Bit 3-4
  m_secctrlReserved = (secControl >> 5) & (0x07);       // Bit 5-7
}

void
VlcMacHeader::SetFrmCounter (uint32_t frmCntr)
{
  m_auxFrmCntr = frmCntr;
}

void
VlcMacHeader::SetSecLevel (uint8_t secLevel)
{
  m_secctrlSecLevel = secLevel;
}

void
VlcMacHeader::SetKeyIdMode (uint8_t keyIdMode)
{
  m_secctrlKeyIdMode = keyIdMode;
}

void
VlcMacHeader::SetSecCtrlReserved (uint8_t res)
{
  m_secctrlReserved = res;
}

void
VlcMacHeader::SetKeyId (uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
}


void
VlcMacHeader::SetKeyId (uint32_t keySrc,
                           uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
  m_auxKeyIdKeySrc32 = keySrc;
}


void
VlcMacHeader::SetKeyId (uint64_t keySrc,
                           uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
  m_auxKeyIdKeySrc64 = keySrc;
}
// AUXILIARY SECURITY HEADER - END

TypeId
VlcMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcMacHeader")
    .SetParent<Header> ()
    .SetGroupName ("Vlc")
    .AddConstructor<VlcMacHeader> ();
  return tid;
}

TypeId
VlcMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
VlcMacHeader::Print (std::ostream &os) const
{
  os << "  Frame Type = " << (uint32_t) m_fctrlFrmType << ", Sec Enable = " << (uint32_t) m_fctrlSecU
     << ", Frame Pending = " << (uint32_t) m_fctrlFrmPending << ", Ack Request = " << (uint32_t) m_fctrlAckReq
     << ", Frame Vers = " << (uint32_t) m_fctrlFrmVer
     << ", Dst Addrs Mode = " << (uint32_t) m_fctrlDstAddrMode << ", Src Addr Mode = " << (uint32_t) m_fctrlSrcAddrMode;

  os << ", Sequence Num = " << static_cast<uint16_t> (m_SeqNum);

  switch (m_fctrlDstAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      os << ", Dst Addr Vpan ID = " << static_cast<uint16_t> (m_addrDstVpanId)
         << ", m_addrShortDstAddr = " << m_addrShortDstAddr;
      break;
    case EXTADDR:
      os << ", Dst Addr Vpan ID = " << static_cast<uint16_t> (m_addrDstVpanId)
         << ", m_addrExtDstAddr = " << m_addrExtDstAddr;
      break;
    }

  switch (m_fctrlSrcAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      os << ", Src Addr Vpan ID = " << static_cast<uint16_t> (m_addrSrcVpanId)
         << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
      break;
    case EXTADDR:
      os << ", Src Addr Vpan ID = " << static_cast<uint32_t> (m_addrSrcVpanId)
         << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
      break;
    }

  if (IsSecEnable ())
    {
      os << "  Security Level = " << static_cast<uint32_t> (m_secctrlSecLevel)
         << ", Key Id Mode = " << static_cast<uint32_t> (m_secctrlKeyIdMode)
         << ", Frame Counter = " << static_cast<uint32_t> (m_auxFrmCntr);

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          os << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        case SHORTKEYSOURCE:
          os << ", Key Id - Key Source 32 =" << static_cast<uint32_t> (m_auxKeyIdKeySrc32)
             << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        case LONGKEYSOURCE:
          os << ", Key Id - Key Source 64 =" << static_cast<uint64_t> (m_auxKeyIdKeySrc64)
             << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        }
    }
}

uint32_t
VlcMacHeader::GetSerializedSize (void) const
{
  /*
   * Each mac header will have
   * Frame Control      : 2 octet
   * Sequence Number    : 1 Octet
   * Dst VPAN Id         : 0/2 Octet
   * Dst Address        : 0/2/8 octet
   * Src PAN Id         : 0/2 octet
   * Src Address        : 0/2/8 octet
   * Aux Sec Header     : 0/5/6/10/14 octet
   */

  uint32_t size = 3;

  switch (m_fctrlDstAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      size += 4;
      break;
    case EXTADDR:
      size += 10;
      break;
    }

  switch (m_fctrlSrcAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      // check if PAN Id compression is enabled
      /*if (!IsPanIdComp ())
        {
          size += 4;
        }
      else
        {
          size += 2;
        } */
      break;
    case EXTADDR:
      // check if PAN Id compression is enabled
      /*if (!IsPanIdComp ())
        {
          size += 10;
        }
      else
        {
          size += 8;
        } */
      break;
    }


  // check if security is enabled
  if (IsSecEnable ())
    {
      size += 5;
      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          size += 1;
          break;
        case SHORTKEYSOURCE:
          size += 5;
          break;
        case LONGKEYSOURCE:
          size += 9;
          break;
        }
    }
  return (size);
}


void
VlcMacHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint16_t frameControl = GetFrameControl ();

  i.WriteHtolsbU16 (frameControl);
  i.WriteU8 (GetSeqNum ());

  switch (m_fctrlDstAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      i.WriteHtolsbU16 (GetDstVpanId ());
      WriteTo (i, m_addrShortDstAddr);
      break;
    case EXTADDR:
      i.WriteHtolsbU16 (GetDstVpanId ());
      WriteTo (i, m_addrExtDstAddr);
      break;
    }

  switch (m_fctrlSrcAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      /*if (!IsPanIdComp ())
        {
          i.WriteHtolsbU16 (GetSrcPanId ());
        }*/
      i.WriteHtolsbU16 (GetSrcVpanId());
      WriteTo (i, m_addrShortSrcAddr);
      break;
    case EXTADDR:
      /*if (!IsPanIdComp ())
        {
          i.WriteHtolsbU16 (GetSrcPanId ());
        }*/
      i.WriteHtolsbU16 (GetSrcVpanId ());
      WriteTo (i, m_addrExtSrcAddr);
      break;
    }

  if (IsSecEnable ())
    {
      i.WriteU8 (GetSecControl ());
      i.WriteHtolsbU32 (GetFrmCounter ());

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          i.WriteU8 (GetKeyIdIndex ());
          break;
        case SHORTKEYSOURCE:
          i.WriteHtolsbU32 (GetKeyIdSrc32 ());
          i.WriteU8 (GetKeyIdIndex ());
          break;
        case LONGKEYSOURCE:
          i.WriteHtolsbU64 (GetKeyIdSrc64 ());
          i.WriteU8 (GetKeyIdIndex ());
          break;
        }
    }
}


uint32_t
VlcMacHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  uint16_t frameControl = i.ReadLsbtohU16 ();
  SetFrameControl (frameControl);

  SetSeqNum (i.ReadU8 ());
  switch (m_fctrlDstAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      m_addrDstVpanId = i.ReadLsbtohU16 ();
      ReadFrom (i, m_addrShortDstAddr);
      break;
    case EXTADDR:
      m_addrDstVpanId = i.ReadLsbtohU16 ();
      ReadFrom (i, m_addrExtDstAddr);
      break;
    }

  switch (m_fctrlSrcAddrMode)
    {
    case NOADDR:
      break;
    case SHORTADDR:
      /*if (!IsPanIdComp ())
        {
          m_addrSrcPanId = i.ReadLsbtohU16 ();
        } */
      if (m_fctrlDstAddrMode > 0)
        {
          m_addrSrcVpanId = m_addrDstVpanId;
        }
      else
        {
          m_addrSrcVpanId = i.ReadLsbtohU16 ();
        }
      ReadFrom (i, m_addrShortSrcAddr);
      break;
    case EXTADDR:
      /*if (!IsPanIdComp ())
        {
          m_addrSrcPanId = i.ReadLsbtohU16 ();
        }*/
      if (m_fctrlDstAddrMode > 0)
        {
          m_addrSrcVpanId = m_addrDstVpanId;
        }
      else
        {
          m_addrSrcVpanId = i.ReadLsbtohU16 ();
        }
      ReadFrom (i, m_addrExtSrcAddr);
      break;
    }

  if (IsSecEnable ())
    {
      SetSecControl (i.ReadU8 ());
      SetFrmCounter (i.ReadLsbtohU32 ());

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          SetKeyId (i.ReadU8 ());
          break;
        case SHORTKEYSOURCE:
          SetKeyId (i.ReadLsbtohU32 (),i.ReadU8 ());
          break;
        case LONGKEYSOURCE:
          SetKeyId (i.ReadLsbtohU64 (),i.ReadU8 ());
          break;
        }
    }
  return i.GetDistanceFrom (start);
}
 // ----------------------------------------------------------------------------


}; // namespace ns-3
