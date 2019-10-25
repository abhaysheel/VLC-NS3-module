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
 * Authors:
 *  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#include "vlc-helper.h"
#include <ns3/vlc-csmaca.h>
#include <ns3/vlc-error-model.h>
#include <ns3/vlc-net-device.h>
#include <ns3/mobility-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/log.h>
#include "ns3/names.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VlcHelper");

/**
 * @brief Output an ascii line representing the Transmit event (with context)
 * @param stream the output stream
 * @param context the context
 * @param p the packet
 */
static void
AsciiVlcMacTransmitSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
}

/**
 * @brief Output an ascii line representing the Transmit event (without context)
 * @param stream the output stream
 * @param p the packet
 */
static void
AsciiVlcMacTransmitSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
}

VlcHelper::VlcHelper (void)
{
  m_channel = CreateObject<SingleModelSpectrumChannel> ();

  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

VlcHelper::VlcHelper (bool useMultiModelSpectrumChannel)
{
  if (useMultiModelSpectrumChannel)
    {
      m_channel = CreateObject<MultiModelSpectrumChannel> ();
    }
  else
    {
      m_channel = CreateObject<SingleModelSpectrumChannel> ();
    }
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

VlcHelper::~VlcHelper (void)
{
  m_channel->Dispose ();
  m_channel = 0;
}

void
VlcHelper::EnableLogComponents (void)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnable ("VlcCsmaCa", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcErrorModel", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcInterferenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcMac", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcSpectrumSignalParameters", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcSpectrumValueHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("VlcWqiTag", LOG_LEVEL_ALL);
}

std::string
VlcHelper::VlcPhyEnumerationPrinter (VlcPhyEnumeration e)
{
  switch (e)
    {
    case IEEE_802_15_7_PHY_BUSY:
      return std::string ("BUSY");
    case IEEE_802_15_7_PHY_BUSY_RX:
      return std::string ("BUSY_RX");
    case IEEE_802_15_7_PHY_BUSY_TX:
      return std::string ("BUSY_TX");
    case IEEE_802_15_7_PHY_FORCE_TRX_OFF:
      return std::string ("FORCE_TRX_OFF");
    case IEEE_802_15_7_PHY_IDLE:
      return std::string ("IDLE");
    case IEEE_802_15_7_PHY_INVALID_PARAMETER:
      return std::string ("INVALID_PARAMETER");
    case IEEE_802_15_7_PHY_RX_ON:
      return std::string ("RX_ON");
    case IEEE_802_15_7_PHY_SUCCESS:
      return std::string ("SUCCESS");
    case IEEE_802_15_7_PHY_TRX_OFF:
      return std::string ("TRX_OFF");
    case IEEE_802_15_7_PHY_TX_ON:
      return std::string ("TX_ON");
    case IEEE_802_15_7_PHY_UNSUPPORTED_ATTRIBUTE:
      return std::string ("UNSUPPORTED_ATTRIBUTE");
    //case IEEE_802_15_4_PHY_READ_ONLY:
      //return std::string ("READ_ONLY");
    case IEEE_802_15_7_PHY_UNSPECIFIED:
      return std::string ("UNSPECIFIED");
    default:
      return std::string ("INVALID");
    }
}

std::string
VlcHelper::VlcMacStatePrinter (VlcMacState e)
{
  switch (e)
    {
    case MAC_IDLE:
      return std::string ("MAC_IDLE");
    case CHANNEL_ACCESS_FAILURE:
      return std::string ("CHANNEL_ACCESS_FAILURE");
    case CHANNEL_IDLE:
      return std::string ("CHANNEL_IDLE");
    case SET_PHY_TX_ON:
      return std::string ("SET_PHY_TX_ON");
    default:
      return std::string ("INVALID");
    }
}

void
VlcHelper::AddMobility (Ptr<VlcPhy> phy, Ptr<MobilityModel> m)
{
  phy->SetMobility (m);
}

NetDeviceContainer
VlcHelper::Install (NodeContainer c)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<VlcNetDevice> netDevice = CreateObject<VlcNetDevice> ();
      netDevice->SetChannel (m_channel);
      node->AddDevice (netDevice);
      netDevice->SetNode (node);
      // \todo add the capability to change short address, extended
      // address and vpanId. Right now they are hardcoded in VlcMac::VlcMac ()
      devices.Add (netDevice);
    }
  return devices;
}


Ptr<SpectrumChannel>
VlcHelper::GetChannel (void)
{
  return m_channel;
}

void
VlcHelper::SetChannel (Ptr<SpectrumChannel> channel)
{
  m_channel = channel;
}

void
VlcHelper::SetChannel (std::string channelName)
{
  Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
  m_channel = channel;
}


int64_t
VlcHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<VlcNetDevice> vlc = DynamicCast<VlcNetDevice> (netDevice);
      if (vlc)
        {
          currentStream += vlc->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}

void
VlcHelper::AssociateToVpan (NetDeviceContainer c, uint16_t vpanId)
{
  NetDeviceContainer devices;
  uint16_t id = 1;
  uint8_t idBuf[2];

  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<VlcNetDevice> device = DynamicCast<VlcNetDevice> (*i);
      if (device)
        {
          idBuf[0] = (id >> 8) & 0xff;
          idBuf[1] = (id >> 0) & 0xff;
          Mac16Address address;
          address.CopyFrom (idBuf);

          device->GetMac ()->SetVpanId (vpanId);
          device->GetMac ()->SetShortAddress (address);
          id++;
        }
    }
  return;
}

/**
 * @brief Write a packet in a PCAP file
 * @param file the output file
 * @param packet the packet
 */
static void
PcapSniffVlc (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
  file->Write (Simulator::Now (), packet);
}

void
VlcHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  NS_LOG_FUNCTION (this << prefix << nd << promiscuous << explicitFilename);
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.
  //

  // In the future, if we create different NetDevice types, we will
  // have to switch on each type below and insert into the right
  // NetDevice type
  //
  Ptr<VlcNetDevice> device = nd->GetObject<VlcNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("VlcHelper::EnablePcapInternal(): Device " << device << " not of type ns3::VlcNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_IEEE802_15_4);

  if (promiscuous == true)
    {
      device->GetMac ()->TraceConnectWithoutContext ("PromiscSniffer", MakeBoundCallback (&PcapSniffVlc, file));

    }
  else
    {
      device->GetMac ()->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffVlc, file));
    }
}

void
VlcHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  Ptr<VlcNetDevice> device = nd->GetObject<VlcNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("VlcHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::VlcNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      // Ascii traces typically have "+", '-", "d", "r", and sometimes "t"
      // The Mac and Phy objects have the trace sources for these
      //

      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<VlcMac> (device->GetMac (), "MacRx", theStream);

      device->GetMac ()->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&AsciiVlcMacTransmitSinkWithoutContext, theStream));

      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<VlcMac> (device->GetMac (), "MacTxEnqueue", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<VlcMac> (device->GetMac (), "MacTxDequeue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<VlcMac> (device->GetMac (), "MacTxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //


  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::VlcNetDevice/Mac/MacRx";
  device->GetMac ()->TraceConnect ("MacRx", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::VlcNetDevice/Mac/MacTx";
  device->GetMac ()->TraceConnect ("MacTx", oss.str (), MakeBoundCallback (&AsciiVlcMacTransmitSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::VlcNetDevice/Mac/MacTxEnqueue";
  device->GetMac ()->TraceConnect ("MacTxEnqueue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::VlcNetDevice/Mac/MacTxDequeue";
  device->GetMac ()->TraceConnect ("MacTxDequeue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::VlcNetDevice/Mac/MacTxDrop";
  device->GetMac ()->TraceConnect ("MacTxDrop", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

}

} // namespace ns3
