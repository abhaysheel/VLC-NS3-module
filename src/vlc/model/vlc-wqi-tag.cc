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
#include "vlc-wqi-tag.h"
#include <ns3/integer.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VlcWqiTag);

TypeId
VlcWqiTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcWqiTag")
    .SetParent<Tag> ()
    .SetGroupName ("Vlc")
    .AddConstructor<VlcWqiTag> ()
    .AddAttribute ("Wqi", "The wqi of the last packet received",
                   IntegerValue (0),
                   MakeIntegerAccessor (&VlcWqiTag::Get),
                   MakeIntegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
VlcWqiTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

VlcWqiTag::VlcWqiTag (void)
  : m_wqi (0)
{
}

VlcWqiTag::VlcWqiTag (uint8_t wqi)
  : m_wqi (wqi)
{
}

uint32_t
VlcWqiTag::GetSerializedSize (void) const
{
  return sizeof (uint8_t);
}

void
VlcWqiTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_wqi);
}

void
VlcWqiTag::Deserialize (TagBuffer i)
{
  m_wqi = i.ReadU8 ();
}

void
VlcWqiTag::Print (std::ostream &os) const
{
  os << "wqi = " << m_wqi;
}

void
VlcWqiTag::Set (uint8_t wqi)
{
  m_wqi = wqi;
}

uint8_t
VlcWqiTag::Get (void) const
{
  return m_wqi;
}
}
