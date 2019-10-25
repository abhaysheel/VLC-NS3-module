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
 *  Author: Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */

 /*
 * the following classes implements the 802.15.7 Mac Header
 * There are 4 types of 802.15.4 Mac Headers Frames, and they have these fields
 *
 *    Headers Frames  : Fields
 *    -------------------------------------------------------------------------------------------
 *    Beacon          : Frame Control, Sequence Number, Address Fields+, Auxiliary Security Header++.
 *    Data            : Frame Control, Sequence Number, Address Fields++, Auxiliary Security Header++.
 *    Acknowledgment  : Frame Control, Sequence Number.
 *    Command         : Frame Control, Sequence Number, Address Fields++, Auxiliary Security Header++.
 *
 *    + - The Address fields in Beacon frame is made up of the Source PAN Id and address only and size
 *        is  4 or 8 octets whereas the other frames may contain the Destination PAN Id and address as
 *        well. (see specs).
 *    ++ - These fields are optional and of variable size
 */

#ifndef VLC_MAC_HEADER_H
#define VLC_MAC_HEADER_H

#include <ns3/header.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>


namespace ns3 {

/**
 * \ingroup vlc
 * Represent the Mac Header with the Frame Control and Sequence Number fields
 */
class VlcMacHeader : public Header
{
public:


  /**
     * The possible MAC types, see IEEE 802.15.7.
     */

  enum VlcMacType
  {
    VLC_MAC_BEACON = 0,
    VLC_MAC_DATA = 1,
    VLC_MAC_ACKNOWLEDGMENT = 2,
    VLC_MAC_COMMAND = 3,
    VLC_MAC_RESERVED
  };

  /**
   * The addressing mode types, see IEEE 802.15.7.
   */
   enum AddrModeType
   {
     NOADDR = 0,
     RESADDR = 1,
     SHORTADDR = 2,
     EXTADDR = 3
   };
   /**
   * The addressing mode types, see IEEE 802.15.7.
   */
   enum KeyIdModeType
   {
     IMPLICIT = 0,
     NOKEYSOURCE = 1,
     SHORTKEYSOURCE = 2,
     LONGKEYSOURCE = 3
   };

   VlcMacHeader (void);

   VlcMacHeader (enum VlcMacType vlcMacType,   //Constructor
                 uint8_t seqNum);

   ~VlcMacHeader (void);

   enum VlcMacType GetType (void) const;

   uint16_t GetFrameControl (void) const;

   uint8_t GetFrameVer (void) const;

   uint8_t GetFrmCtrlRes (void) const;

   //uint8_t GetFrameType (void) const;

   bool IsSecEnable (void) const;

   bool IsFrmPend (void) const;

   bool IsAckReq (void) const;

   uint8_t GetDstAddrMode (void) const;

   uint8_t GetSrcAddrMode (void) const;

   uint8_t GetSeqNum (void) const;

   uint16_t GetDstVpanId (void) const;

   Mac16Address GetShortDstAddr (void) const;

   Mac64Address GetExtDstAddr (void) const;

   uint16_t GetSrcVpanId (void) const;

   Mac16Address GetShortSrcAddr (void) const;

   Mac64Address GetExtSrcAddr (void) const;

   // AUXILIARY SECURITY HEADER- START

   uint8_t GetSecControl (void) const;

   uint32_t GetFrmCounter (void) const;

   uint8_t GetSecLevel (void) const;

   uint8_t GetKeyIdMode (void) const;

   uint8_t GetSecCtrlReserved (void) const;

   uint32_t GetKeyIdSrc32 (void) const;

   uint64_t GetKeyIdSrc64 (void) const;

   uint8_t GetKeyIdIndex (void) const;
   // AUXILIARY SECURITY HEADER - END

   bool IsBeacon (void) const;

   bool IsData (void) const;

   bool IsAcknowledgment (void) const;

   bool IsCommand (void) const;

   void SetType (enum VlcMacType vlcMacType);

   void SetFrameControl (uint16_t frameControl);

   void SetSecEnable (void);

   void SetSecDisable (void);

   void SetFrmPend (void);

   void SetNoFrmPend (void);

   void SetAckReq (void);

   void SetNoAckReq (void);

   //void SetVpanIdComp (void);

   //void SetNoVpanIdComp (void);

   void SetFrmCtrlRes (uint8_t res);

   void SetDstAddrMode (uint8_t addrMode);

   void SetSrcAddrMode (uint8_t addrMode);

   void SetFrameVer (uint8_t ver);

   void SetSeqNum (uint8_t seqNum);

   void SetSrcAddrFields (uint16_t vpanId, Mac16Address addr);

   void SetSrcAddrFields (uint16_t vpanId, Mac64Address addr);

   void  SetDstAddrFields (uint16_t vpanId, Mac16Address addr);

   void SetDstAddrFields (uint16_t vpanId, Mac64Address addr);

   // SECURITY HEADER
   void SetSecControl (uint8_t secLevel);

   void SetFrmCounter (uint32_t frmCntr);

   void SetSecLevel (uint8_t keyIdMode);

   void SetKeyIdMode (uint8_t keyIdMode);

   void SetSecCtrlReserved (uint8_t res);

   void SetKeyId (uint8_t keyIndex);

   void SetKeyId (uint32_t keySrc, uint8_t keyIndex);

   void SetKeyId (uint64_t keySrc, uint8_t keyIndex);
   // AUXILIARY SECURITY HEADER

   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;

   void Print (std::ostream &os) const;
   uint32_t GetSerializedSize (void) const;
   void Serialize (Buffer::Iterator start) const;
   uint32_t Deserialize (Buffer::Iterator start);

private:
  /* Frame Control 2 Octets */
  uint8_t m_fctrlFrmVer;
  uint8_t m_fctrlReserved;
  uint8_t m_fctrlFrmType;
  uint8_t m_fctrlSecU;
  uint8_t m_fctrlFrmPending;
  uint8_t m_fctrlAckReq;
  uint8_t m_fctrlDstAddrMode;
  uint8_t m_fctrlSrcAddrMode;

  /* Sequence Number */
  uint8_t m_SeqNum;

  /* Addressing fields */
  uint16_t m_addrDstVpanId;              //!< Dst PAN id (0 or 2 Octets)
  Mac16Address m_addrShortDstAddr;      //!< Dst Short addr (0 or 2 Octets)
  Mac64Address m_addrExtDstAddr;        //!< Dst Ext addr (0 or 8 Octets)
  uint16_t m_addrSrcVpanId;              //!< Src PAN id (0 or 2 Octets)
  Mac16Address m_addrShortSrcAddr;      //!< Src Short addr (0 or 2 Octets)
  Mac64Address m_addrExtSrcAddr;

  /* Auxiliary Security Header - See 7.6.2 - 0, 5, 6, 10 or 14 Octets */
  uint32_t m_auxFrmCntr;                //!< Auxiliary security header - Frame Counter (4 Octets)

  /* Security Control fields - See 7.6.2.2 */
  uint8_t m_secctrlSecLevel;
  uint8_t m_secctrlKeyIdMode;
  uint8_t m_secctrlReserved;
  union
  {
    uint32_t m_auxKeyIdKeySrc32;        //!< Auxiliary security header - Key Source (4 Octets)
    uint64_t m_auxKeyIdKeySrc64;        //!< Auxiliary security header - Key Source (8 Octets)
  }; //!< Auxiliary security header

  uint8_t m_auxKeyIdKeyIndex;

};  //VlcMacHeader

};  // namespace ns-3

#endif /* VLC_MAC_HEADER */
