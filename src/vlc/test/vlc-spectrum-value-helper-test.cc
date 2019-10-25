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
#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/vlc-spectrum-value-helper.h>
#include <ns3/spectrum-value.h>

#include <cmath>

using namespace ns3;

/**
 * \ingroup vlc-test
 * \ingroup tests
 *
 * \brief Vlc SpectrumValue Helper TestSuite
 */
class VlcSpectrumValueHelperTestCase : public TestCase
{
public:
  VlcSpectrumValueHelperTestCase ();
  virtual ~VlcSpectrumValueHelperTestCase ();

private:
  virtual void DoRun (void);
};

VlcSpectrumValueHelperTestCase::VlcSpectrumValueHelperTestCase ()
  : TestCase ("Test the 802.15.7 SpectrumValue helper class")
{
}

VlcSpectrumValueHelperTestCase::~VlcSpectrumValueHelperTestCase ()
{
}

void
VlcSpectrumValueHelperTestCase::DoRun (void)
{
  VlcSpectrumValueHelper helper;
  Ptr<SpectrumValue> value;
  double pwrWatts;
  for (uint32_t chan = 11; chan <= 26; chan++)
    {
      // 50dBm = 100 W, -50dBm = 0.01 mW
      for (double pwrdBm = -50; pwrdBm < 50; pwrdBm += 10)
        {
          value = helper.CreateTxPowerSpectralDensity (pwrdBm, chan);
          pwrWatts = pow (10.0, pwrdBm / 10.0) / 1000;
          // Test that average power calculation is within +/- 25% of expected
          NS_TEST_ASSERT_MSG_EQ_TOL (helper.TotalAvgPower (value, chan), pwrWatts, pwrWatts / 4.0, "Not equal for channel " << chan << " pwrdBm " << pwrdBm);
        }
    }
}

/**
 * \ingroup vlc-test
 * \ingroup tests
 *
 * \brief Vlc SpectrumValue Helper TestSuite
 */
class VlcSpectrumValueHelperTestSuite : public TestSuite
{
public:
  VlcSpectrumValueHelperTestSuite ();
};

VlcSpectrumValueHelperTestSuite::VlcSpectrumValueHelperTestSuite ()
  : TestSuite ("vlc-spectrum-value-helper", UNIT)
{
  AddTestCase (new VlcSpectrumValueHelperTestCase, TestCase::QUICK);
}

static VlcSpectrumValueHelperTestSuite g_vlcSpectrumValueHelperTestSuite; //!< Static variable for test initialization
