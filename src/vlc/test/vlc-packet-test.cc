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
 * Author:  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#include <ns3/test.h>
#include <ns3/packet.h>
#include <ns3/vlc-mac-header.h>
#include <ns3/vlc-mac-trailer.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/log.h>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("vlc-packet-test");

/**
 * \ingroup vlc-test
 * \ingroup tests
 *
 * \brief Vlc header and footer Test
 */
class VlcPacketTestCase : public TestCase
{
public:
  VlcPacketTestCase ();
  virtual ~VlcPacketTestCase ();

private:
  virtual void DoRun (void);
};

VlcPacketTestCase::VlcPacketTestCase ()
  : TestCase ("Test the 802.15.7 MAC header and trailer classes")
{
}

VlcPacketTestCase::~VlcPacketTestCase ()
{
}

void
VlcPacketTestCase::DoRun (void)
{

  VlcMacHeader macHdr (VlcMacHeader::VLC_MAC_BEACON, 0);        //sequence number set to 0
  macHdr.SetSrcAddrMode (VlcMacHeader::SHORTADDR);                    // short addr
  macHdr.SetDstAddrMode (VlcMacHeader::NOADDR);
  macHdr.SetSecDisable ();
  //macHdr.SetNoVpanIdComp ();
  // ... other setters

  uint16_t srcVpanId = 100;
  Mac16Address srcVpanAddr ("00:11");
  macHdr.SetSrcAddrFields (srcVpanId, srcVpanAddr);

  VlcMacTrailer macTrailer;


  Ptr<Packet> p = Create<Packet> (20);  // 20 bytes of dummy data
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 20, "Packet created with unexpected size");
  p->AddHeader (macHdr);
  std::cout << " <--Mac Header added " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 27, "Packet wrong size after macHdr addition");
  p->AddTrailer (macTrailer);
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 29, "Packet wrong size after macTrailer addition");

  // Test serialization and deserialization
  uint32_t size = p->GetSerializedSize ();
  uint8_t buffer[size];
  p->Serialize (buffer, size);
  Ptr<Packet> p2 = Create<Packet> (buffer, size, true);


  p2->Print (std::cout);
  std::cout << " <--Packet P2 " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 29, "Packet wrong size after deserialization");

  VlcMacHeader receivedMacHdr;
  p2->RemoveHeader (receivedMacHdr);

  receivedMacHdr.Print (std::cout);
  std::cout << " <--P2 Mac Header " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 22, "Packet wrong size after removing machdr");

  VlcMacTrailer receivedMacTrailer;
  p2->RemoveTrailer (receivedMacTrailer);
  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 20, "Packet wrong size after removing headers and footers");
  // Compare macHdr with receivedMacHdr, macFooter with receivedMacFooter,...

}

/**
 * \ingroup vlc-test
 * \ingroup tests
 *
 * \brief Vlc header and footer TestSuite
 */
class VlcPacketTestSuite : public TestSuite
{
public:
  VlcPacketTestSuite ();
};

VlcPacketTestSuite::VlcPacketTestSuite ()
  : TestSuite ("vlc-packet", UNIT)
{
  AddTestCase (new VlcPacketTestCase, TestCase::QUICK);
}

static VlcPacketTestSuite g_vlcPacketTestSuite; //!< Static variable for test initialization
