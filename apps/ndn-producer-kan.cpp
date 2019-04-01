/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors
 *and contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the
 *terms of the GNU General Public License as published by the Free Software
 *Foundation, either version 3 of the License, or (at your option) any later
 *version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY
 *WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 *A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see
 *<http://www.gnu.org/licenses/>.
 **/

#include "ndn-producer-kan.hpp"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "helper/ndn-fib-helper.hpp"
#include "model/ndn-app-face.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "model/ndn-ns3.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE( "ndn.ProducerKan" );

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED( ProducerKan );

TypeId ProducerKan::GetTypeId( void ) {
  static TypeId tid =
      TypeId( "ns3::ndn::ProducerKan" )
          .SetGroupName( "Ndn" )
          .SetParent<App>()
          .AddConstructor<ProducerKan>()
          .AddAttribute( "Prefix", "Prefix, for which producer has the data",
                         StringValue( "/" ),
                         MakeNameAccessor( &ProducerKan::m_prefix ),
                         MakeNameChecker() )
          .AddAttribute(
              "Postfix", "Postfix that is added to the output data (e.g., "
                         "for adding producer-uniqueness)",
              StringValue( "/" ), MakeNameAccessor( &ProducerKan::m_postfix ),
              MakeNameChecker() )
          .AddAttribute(
              "PayloadSize", "Virtual payload size for Content packets",
              UintegerValue( 1024 ),
              MakeUintegerAccessor( &ProducerKan::m_virtualPayloadSize ),
              MakeUintegerChecker<uint32_t>() )
          .AddAttribute(
              "Freshness",
              "Freshness of data packets, if 0, then unlimited freshness",
              TimeValue( Seconds( 0 ) ),
              MakeTimeAccessor( &ProducerKan::m_freshness ), MakeTimeChecker() )
          .AddAttribute( "Signature",
                         "Fake signature, 0 valid signature (default), other "
                         "values application-specific",
                         UintegerValue( 0 ),
                         MakeUintegerAccessor( &ProducerKan::m_signature ),
                         MakeUintegerChecker<uint32_t>() )
          .AddAttribute(
              "KeyLocator", "Name to be used for key locator.  If root, then "
                            "key locator is not used",
              NameValue(), MakeNameAccessor( &ProducerKan::m_keyLocator ),
              MakeNameChecker() );
  return tid;
}

ProducerKan::ProducerKan() { NS_LOG_FUNCTION_NOARGS(); }

// inherited from Application base class.
void ProducerKan::StartApplication() {
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute( GetNode(), m_prefix, m_face, 0 );
}

void ProducerKan::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void ProducerKan::OnInterest( shared_ptr<const Interest> interest ) {
  App::OnInterest( interest ); // tracing inside

  NS_LOG_FUNCTION( this << interest );

  if ( !m_active ) return;

  // add by kan 20190331
  if ( interest->getValidationFlag() == 1 ) {
    struct PITListEntry pe;
    pe.name    = interest->getName();
    pe.PITList = interest->getPITList();
    int tnow   = (int) ns3::Simulator::Now().GetSeconds();
    pe.ttl     = tnow;
    std::cout << pe.name << "=====" << pe.PITList << "====" << pe.ttl
              << std::endl;
    if ( !PITListStore.empty() ) {
      std::list<PITListEntry>::iterator it;
      std::list<PITListEntry>::iterator next = PITListStore.end();
      for ( it = PITListStore.begin(); it != PITListStore.end(); it = next ) {
        next = ++it;
        --it;
        if ( tnow - it->ttl > 5 ) {
          PITListStore.erase( it );
        }
        if ( it->PITList == interest->getPITList() &&
             it->name == interest->getName() ) {
          PITListStore.erase( it );
          // struct PITListEntry peTemp;
          // peTemp.name = it->name;
          // peTemp.PITList = it->PITList;read

          // PITListStore.push_front(peTemp);
          // break;
        }
      }
    }
    PITListStore.push_front( pe );
    // std::cout << PITListStore.size() << std::endl;
  }

  // 每隔frequency秒把PITListStore内的信息发布一次
  int    frequency = 5;
  double tnow      = ns3::Simulator::Now().GetSeconds();
  if ( (int) tnow % frequency != 0 ) {
    published = false;
  }
  std::cout << "false " << tnow << std::endl;
  if ( (int) tnow % frequency == 0 && (int) tnow != 0 ) {
    if ( !published ) {
      std::cout << "true " << tnow << std::endl;
      published = true;
      for ( std::list<PITListEntry>::iterator it = PITListStore.begin();
            it != PITListStore.end(); it++ ) {
        Name dataName( it->name );
        auto data = make_shared<Data>();
        data->setName( dataName );
        data->setFreshnessPeriod(
            ::ndn::time::milliseconds( m_freshness.GetMilliSeconds() ) );

        data->setContent( make_shared<::ndn::Buffer>( m_virtualPayloadSize ) );

        Signature     signature;
        SignatureInfo signatureInfo(
            static_cast<::ndn::tlv::SignatureTypeValue>( 255 ) );

        if ( m_keyLocator.size() > 0 ) {
          signatureInfo.setKeyLocator( m_keyLocator );
        }

        signature.setInfo( signatureInfo );
        signature.setValue(::ndn::nonNegativeIntegerBlock(
            ::ndn::tlv::SignatureValue, m_signature ) );

        data->setSignature( signature );
        data->wireEncode();

        m_transmittedDatas( data, this, m_face );
        m_face->onReceiveData( *data );
      }
    }
  }
  // end add

  Name dataName( interest->getName() );
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  auto data = make_shared<Data>();
  data->setName( dataName );
  data->setFreshnessPeriod(
      ::ndn::time::milliseconds( m_freshness.GetMilliSeconds() ) );

  data->setContent( make_shared<::ndn::Buffer>( m_virtualPayloadSize ) );

  Signature     signature;
  SignatureInfo signatureInfo(
      static_cast<::ndn::tlv::SignatureTypeValue>( 255 ) );

  if ( m_keyLocator.size() > 0 ) {
    signatureInfo.setKeyLocator( m_keyLocator );
  }

  signature.setInfo( signatureInfo );
  signature.setValue(::ndn::nonNegativeIntegerBlock(::ndn::tlv::SignatureValue,
                                                    m_signature ) );

  data->setSignature( signature );

  NS_LOG_INFO( "node(" << GetNode()->GetId()
                       << ") responding with Data: " << data->getName() );

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas( data, this, m_face );
  m_face->onReceiveData( *data );
}

} // namespace ndn
} // namespace ns3
