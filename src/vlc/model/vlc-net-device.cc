/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author:
 *  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#include "vlc-net-device.h"
#include "vlc-phy.h"
#include "vlc-csmaca.h"
#include "vlc-error-model.h"
#include <ns3/abort.h>
#include <ns3/node.h>
#include <ns3/log.h>
#include <ns3/spectrum-channel.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VlcNetDevice");

NS_OBJECT_ENSURE_REGISTERED (VlcNetDevice);

TypeId
VlcNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcnNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("Vlc")
    .AddConstructor<VlcNetDevice> ()
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::DoGetChannel),
                   MakePointerChecker<SpectrumChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::GetPhy,
                                        &VlcNetDevice::SetPhy),
                   MakePointerChecker<VlcPhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&VlcNetDevice::GetMac,
                                        &VlcNetDevice::SetMac),
                   MakePointerChecker<VlcMac> ())
    .AddAttribute ("UseAcks", "Request acknowledgments for data frames.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&VlcNetDevice::m_useAcks),
                   MakeBooleanChecker ())
  ;
  return tid;
}

VlcNetDevice::VlcNetDevice ()
  : m_configComplete (false)
{
  NS_LOG_FUNCTION (this);
  m_mac = CreateObject<VlcMac> ();
  m_phy = CreateObject<VlcPhy> ();
  m_csmaca = CreateObject<VlcCsmaCa> ();
  CompleteConfig ();
}

VlcNetDevice::~VlcNetDevice ()
{
  NS_LOG_FUNCTION (this);
}


void
VlcNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_mac->Dispose ();
  m_phy->Dispose ();
  m_csmaca->Dispose ();
  m_phy = 0;
  m_mac = 0;
  m_csmaca = 0;
  m_node = 0;
  // chain up.
  NetDevice::DoDispose ();

}

void
VlcNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_phy->Initialize ();
  m_mac->Initialize ();
  NetDevice::DoInitialize ();
}


void
VlcNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (m_mac == 0
      || m_phy == 0
      || m_csmaca == 0
      || m_node == 0
      || m_configComplete)
    {
      return;
    }
  m_mac->SetPhy (m_phy);
  m_mac->SetCsmaCa (m_csmaca);
  m_mac->SetMcpsDataIndicationCallback (MakeCallback (&VlcNetDevice::McpsDataIndication, this));
  m_csmaca->SetMac (m_mac);

  Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel> ();
  if (!mobility)
    {
      NS_LOG_WARN ("VlcNetDevice: no Mobility found on the node, probably it's not a good idea.");
    }
  m_phy->SetMobility (mobility);
  Ptr<VlcErrorModel> model = CreateObject<VlcErrorModel> ();
  m_phy->SetErrorModel (model);
  m_phy->SetDevice (this);

  m_phy->SetPdDataIndicationCallback (MakeCallback (&VlcMac::PdDataIndication, m_mac));
  m_phy->SetPdDataConfirmCallback (MakeCallback (&VlcMac::PdDataConfirm, m_mac));
// TODO : CHECK
//  m_phy->SetPlmeEdConfirmCallback (MakeCallback (&VlcMac::PlmeEdConfirm, m_mac));
  m_phy->SetPlmeGetAttributeConfirmCallback (MakeCallback (&VlcMac::PlmeGetAttributeConfirm, m_mac));
  m_phy->SetPlmeSetTRXStateConfirmCallback (MakeCallback (&VlcMac::PlmeSetTRXStateConfirm, m_mac));
  m_phy->SetPlmeSetAttributeConfirmCallback (MakeCallback (&VlcMac::PlmeSetAttributeConfirm, m_mac));

  m_csmaca->SetVlcMacStateCallback (MakeCallback (&VlcMac::SetVlcMacState, m_mac));
  m_phy->SetPlmeCcaConfirmCallback (MakeCallback (&VlcCsmaCa::PlmeCcaConfirm, m_csmaca));
  m_configComplete = true;
}

void
VlcNetDevice::SetMac (Ptr<VlcMac> mac)
{
  NS_LOG_FUNCTION (this);
  m_mac = mac;
  CompleteConfig ();
}

void
VlcNetDevice::SetPhy (Ptr<VlcPhy> phy)
{
  NS_LOG_FUNCTION (this);
  m_phy = phy;
  CompleteConfig ();
}

void
VlcNetDevice::SetCsmaCa (Ptr<VlcCsmaCa> csmaca)
{
  NS_LOG_FUNCTION (this);
  m_csmaca = csmaca;
  CompleteConfig ();
}

void
VlcNetDevice::SetChannel (Ptr<SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_phy->SetChannel (channel);
  channel->AddRx (m_phy);
  CompleteConfig ();
}

Ptr<VlcMac>
VlcNetDevice::GetMac (void) const
{
  // NS_LOG_FUNCTION (this);
  return m_mac;
}

Ptr<VlcPhy>
VlcNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

Ptr<VlcCsmaCa>
VlcNetDevice::GetCsmaCa (void) const
{
  NS_LOG_FUNCTION (this);
  return m_csmaca;
}
void
VlcNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t
VlcNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

Ptr<Channel>
VlcNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
VlcNetDevice::LinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChanges ();
}

void
VlcNetDevice::LinkDown (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = false;
  m_linkChanges ();
}

Ptr<SpectrumChannel>
VlcNetDevice::DoGetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
VlcNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this);
  m_mac->SetShortAddress (Mac16Address::ConvertFrom (address));
}

Address
VlcNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac->GetShortAddress ();
}

bool
VlcNetDevice::SetMtu (const uint16_t mtu)
{
  NS_ABORT_MSG ("Unsupported");
  return false;
}

uint16_t
VlcNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  // Maximum payload size is: max psdu - frame control - seqno - addressing - security - fcs
  //                        = 127      - 2             - 1     - (2+2+2+2)  - 0        - 2
  //                        = 114
  // assuming no security and addressing with only 16 bit addresses without pan id compression.
  return 114;
}

bool
VlcNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy != 0 && m_linkUp;
}

void
VlcNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
VlcNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
VlcNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac16Address ("ff:ff");
}

bool
VlcNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
VlcNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_ABORT_MSG ("Unsupported");
  return Address ();
}

Address
VlcNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this);
  /* Implementation based on RFC 4944 Section 9.
   * An IPv6 packet with a multicast destination address (DST),
   * consisting of the sixteen octets DST[1] through DST[16], is
   * transmitted to the following 802.15.4 16-bit multicast address:
   *           0                   1
   *           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *          |1 0 0|DST[15]* |   DST[16]     |
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * Here, DST[15]* refers to the last 5 bits in octet DST[15], that is,
   * bits 3-7 within DST[15].  The initial 3-bit pattern of "100" follows
   * the 16-bit address format for multicast addresses (Section 12). */

  // \todo re-add this once Lr-Wpan will be able to accept these multicast addresses
  //  uint8_t buf[16];
  //  uint8_t buf2[2];
  //
  //  addr.GetBytes(buf);
  //
  //  buf2[0] = 0x80 | (buf[14] & 0x1F);
  //  buf2[1] = buf[15];
  //
  //  Mac16Address newaddr = Mac16Address();
  //  newaddr.CopyFrom(buf2);
  //  return newaddr;

  return Mac16Address ("ff:ff");
}

bool
VlcNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
VlcNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
VlcNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataRequest parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (packet->GetSize () > GetMtu ())
    {
      NS_LOG_ERROR ("Fragmentation is needed for this packet, drop the packet ");
      return false;
    }

  McpsDataRequestParams m_mcpsDataRequestParams;
  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
  m_mcpsDataRequestParams.m_dstVpanId = m_mac->GetVpanId ();
  m_mcpsDataRequestParams.m_srcAddrMode = SHORT_ADDR;
  // Using ACK requests for broadcast destinations is ok here. They are disabled
  // by the MAC.
  if (m_useAcks)
    {
      m_mcpsDataRequestParams.m_txOptions = TX_OPTION_ACK;
    }
  m_mcpsDataRequestParams.m_msduHandle = 0;
  m_mac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  return true;
}

bool
VlcNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_ABORT_MSG ("Unsupported");
  // TODO: To support SendFrom, the MACs McpsDataRequest has to use the provided source address, instead of to local one.
  return false;
}

Ptr<Node>
VlcNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
VlcNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
  CompleteConfig ();
}

bool
VlcNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
VlcNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_receiveCallback = cb;
}

void
VlcNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataIndication parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_WARN ("Unsupported; use Vlc MAC APIs instead");
}

void
VlcNetDevice::McpsDataIndication (McpsDataIndicationParams params, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this);
  // TODO: Use the PromiscReceiveCallback if the MAC is in promiscuous mode.
  m_receiveCallback (this, pkt, 0, params.m_srcAddr);
}

bool
VlcNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

int64_t
VlcNetDevice::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (stream);
  int64_t streamIndex = stream;
  streamIndex += m_csmaca->AssignStreams (stream);
  streamIndex += m_phy->AssignStreams (stream);
  NS_LOG_DEBUG ("Number of assigned RV streams:  " << (streamIndex - stream));
  return (streamIndex - stream);
}

} // namespace ns3
