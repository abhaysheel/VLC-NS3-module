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
#ifndef VLC_HELPER_H
#define VLC_HELPER_H

#include <ns3/node-container.h>
#include <ns3/vlc-phy.h>
#include <ns3/vlc-mac.h>
#include <ns3/trace-helper.h>

namespace ns3 {

class SpectrumChannel;
class MobilityModel;

/**
 * \ingroup vlc
 *
 * \brief helps to manage and create IEEE 802.15.7 NetDevice objects
 *
 * This class can help to create IEEE 802.15.7 NetDevice objects
 * and to configure their attributes during creation.  It also contains
 * additional helper functions used by client code.
 *
 * Only one channel is created, and all devices attached to it.  If
 * multiple channels are needed, multiple helper objects must be used,
 * or else the channel object must be replaced.
 */

class VlcHelper : public PcapHelperForDevice,
                     public AsciiTraceHelperForDevice
{
public:
  /**
   * \brief Create a Vlc helper in an empty state.  By default, a
   * SingleModelSpectrumChannel is created, with a
   * LogDistancePropagationLossModel and a ConstantSpeedPropagationDelayModel.
   *
   * To change the channel type, loss model, or delay model, the Get/Set
   * Channel methods may be used.
   */
  VlcHelper (void);

  /**
   * \brief Create a Vlc helper in an empty state with either a
   * SingleModelSpectrumChannel or a MultiModelSpectrumChannel.
   * \param useMultiModelSpectrumChannel use a MultiModelSpectrumChannel if true, a SingleModelSpectrumChannel otherwise
   *
   * A LogDistancePropagationLossModel and a
   * ConstantSpeedPropagationDelayModel are added to the channel.
   */
  VlcHelper (bool useMultiModelSpectrumChannel);

  virtual ~VlcHelper (void);

  /**
   * \brief Get the channel associated to this helper
   * \returns the channel
   */
  Ptr<SpectrumChannel> GetChannel (void);

  /**
   * \brief Set the channel associated to this helper
   * \param channel the channel
   */
  void SetChannel (Ptr<SpectrumChannel> channel);

  /**
   * \brief Set the channel associated to this helper
   * \param channelName the channel name
   */
  void SetChannel (std::string channelName);

  /**
   * \brief Add mobility model to a physical device
   * \param phy the physical device
   * \param m the mobility model
   */
  void AddMobility (Ptr<VlcPhy> phy, Ptr<MobilityModel> m);

  /**
   * \brief Install a VlcNetDevice and the associated structures (e.g., channel) in the nodes.
   * \param c a set of nodes
   * \returns A container holding the added net devices.
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * \brief Associate the nodes to the same VPAN
   *
   * \param c a set of nodes
   * \param vpanId the VPAN Id
   */
  void AssociateToVpan (NetDeviceContainer c, uint16_t vpanId);

  /**
   * Helper to enable all Vlc log components with one statement
   */
  void EnableLogComponents (void);

  /**
   * \brief Transform the VlcPhyEnumeration enumeration into a printable string.
   * \param e the VlcPhyEnumeration
   * \return a string
   */
  static std::string VlcPhyEnumerationPrinter (VlcPhyEnumeration e);

  /**
   * \brief Transform the VlcMacState enumeration into a printable string.
   * \param e the VlcMacState
   * \return a string
   */
  static std::string VlcMacStatePrinter (VlcMacState e);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams that have been
   * assigned. The Install() method should have previously been
   * called by the user.
   *
   * \param c NetDeviceContainer of the set of net devices for which the
   *          CsmaNetDevice should be modified to use a fixed stream
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this helper
   */
  int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

private:
  // Disable implicit constructors
  /**
   * \brief Copy constructor - defined and not implemented.
   */
  VlcHelper (VlcHelper const &);
  /**
   * \brief Copy constructor - defined and not implemented.
   * \returns
   */
  VlcHelper& operator= (VlcHelper const &);
  /**
   * \brief Enable pcap output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                    std::string prefix,
                                    Ptr<NetDevice> nd,
                                    bool explicitFilename);

private:
  Ptr<SpectrumChannel> m_channel; //!< channel to be used for the devices

};

}

#endif /* VLC_HELPER_H */
