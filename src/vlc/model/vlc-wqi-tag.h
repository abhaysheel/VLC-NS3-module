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
 * Author:
 *  Abhay Sheel Anand <abhaysheelanand@gmail.com>
 */
#ifndef VLC_WQI_TAG_H
#define VLC_WQI_TAG_H

#include <ns3/tag.h>

namespace ns3 {

class VlcWqiTag : public Tag
{
public:
 /**
  * Get the type ID.
  *
  * \return the object TypeId
  */
 static TypeId GetTypeId (void);

 virtual TypeId GetInstanceTypeId (void) const;

 /**
  * Create a VlcWqiTag with the default WQI 0.
  */
 VlcWqiTag (void);


 VlcWqiTag (uint8_t wqi);

 virtual uint32_t GetSerializedSize (void) const;
 virtual void Serialize (TagBuffer i) const;
 virtual void Deserialize (TagBuffer i);
 virtual void Print (std::ostream &os) const;

 /**
  * Set the WQI to the given value.
  *
  * \param wqi the value of the WQI to set
  */
 void Set (uint8_t wqi);

 /**
  * Get the WQI value.
  *
  * \return the WQI value
  */
 uint8_t Get (void) const;
private:
 /**
  * The current WQI value of the tag.
  */
 uint8_t m_wqi;
};


}
#endif /* VLC_WQI_TAG_H */
