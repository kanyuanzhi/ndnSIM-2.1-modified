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

#include "ndn-producer-kan-v2.hpp"
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
#include <random>

using namespace std;

NS_LOG_COMPONENT_DEFINE( "ndn.ProducerKanV2" );

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED( ProducerKanV2 );

TypeId ProducerKanV2::GetTypeId( void ) {
  static TypeId tid =
      TypeId( "ns3::ndn::ProducerKanV2" )
          .SetGroupName( "Ndn" )
          .SetParent<App>()
          .AddConstructor<ProducerKanV2>()
          .AddAttribute( "Prefix", "Prefix, for which producer has the data",
                         StringValue( "/" ),
                         MakeNameAccessor( &ProducerKanV2::m_prefix ),
                         MakeNameChecker() )
          .AddAttribute( "Postfix",
                         "Postfix that is added to the output data (e.g., "
                         "for adding producer-uniqueness)",
                         StringValue( "/" ),
                         MakeNameAccessor( &ProducerKanV2::m_postfix ),
                         MakeNameChecker() )
          .AddAttribute(
              "PayloadSize", "Virtual payload size for Content packets",
              UintegerValue( 1024 ),
              MakeUintegerAccessor( &ProducerKanV2::m_virtualPayloadSize ),
              MakeUintegerChecker<uint32_t>() )
          .AddAttribute(
              "Freshness",
              "Freshness of data packets, if 0, then unlimited freshness",
              TimeValue( Seconds( 0 ) ),
              MakeTimeAccessor( &ProducerKanV2::m_freshness ),
              MakeTimeChecker() )
          .AddAttribute( "Signature",
                         "Fake signature, 0 valid signature (default), other "
                         "values application-specific",
                         UintegerValue( 0 ),
                         MakeUintegerAccessor( &ProducerKanV2::m_signature ),
                         MakeUintegerChecker<uint32_t>() )
          .AddAttribute( "KeyLocator",
                         "Name to be used for key locator.  If root, then "
                         "key locator is not used",
                         NameValue(),
                         MakeNameAccessor( &ProducerKanV2::m_keyLocator ),
                         MakeNameChecker() )
          .AddAttribute(
              "AverageUpdateTime", "内容平均更新时间", StringValue( "20" ),
              MakeUintegerAccessor( &ProducerKanV2::m_average_update_time ),
              MakeUintegerChecker<uint32_t>() )
          .AddAttribute(
              "MaxPITStoreSize", "PITStore最大容量", StringValue( "100" ),
              MakeUintegerAccessor( &ProducerKanV2::m_max_pitstore_size ),
              MakeUintegerChecker<uint32_t>() )
          .AddAttribute(
              "ExprimentTime", "实验时长", StringValue( "150" ),
              MakeUintegerAccessor( &ProducerKanV2::m_expriment_time ),
              MakeUintegerChecker<uint32_t>() )
          .AddAttribute( "EligibleContentsNumber", "主动推送的内容数量",
                         StringValue( "100" ),
                         MakeUintegerAccessor(
                             &ProducerKanV2::m_eligible_contents_number ),
                         MakeUintegerChecker<uint32_t>() );
  return tid;
}

ProducerKanV2::ProducerKanV2() { NS_LOG_FUNCTION_NOARGS(); }

// inherited from Application base class.
void ProducerKanV2::StartApplication() {
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute( GetNode(), m_prefix, m_face, 0 );
}

void ProducerKanV2::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
  App::StopApplication();
}

void ProducerKanV2::OnInterest( shared_ptr<const Interest> interest ) {
  App::OnInterest( interest ); // tracing inside

  // cout<< m_face->getId()<<endl;
  // cout<< m_average_update_time <<endl;
  // cout<< m_max_pitstore_size <<endl;
  int LocationRegistration = interest->getLocationRegistration();

  NS_LOG_FUNCTION( this << interest );
  if ( !m_active ) return;

  // add by kan 20190416
  double tnow = ns3::Simulator::Now().GetSeconds();
  // int      average_update_time = 20;
  // uint32_t maxSize             = max_pitstore_size;

  // 更新因子，范围[0,1]，表示内容到期后，更新时间变化的概率。
  double update_factor = 1.0;

  random_device r;
  string        eligible_contents;

  switch ( m_eligible_contents_number ) {
  case 50:
    eligible_contents = eligible_contents_50;
    break;
  case 60:
    eligible_contents = eligible_contents_60;
    break;
  case 70:
    eligible_contents = eligible_contents_70;
    break;
  case 80:
    eligible_contents = eligible_contents_80;
    break;
  case 90:
    eligible_contents = eligible_contents_90;
    break;
  case 100:
    eligible_contents = eligible_contents_100;
    break;
  case 110:
    eligible_contents = eligible_contents_110;
    break;
  case 120:
    eligible_contents = eligible_contents_120;
    break;
  case 130:
    eligible_contents = eligible_contents_130;
    break;
  case 140:
    eligible_contents = eligible_contents_140;
    break;
  case 150:
    eligible_contents = eligible_contents_150;
    break;
  case 200:
    eligible_contents = eligible_contents_200;
    break;
  case 300:
    eligible_contents = eligible_contents_300;
    break;
  case 400:
    eligible_contents = eligible_contents_400;
    break;
  case 500:
    eligible_contents = eligible_contents_500;
    break;
  case 1000:
    eligible_contents = eligible_contents_1000;
    break;

  default:
    break;
  }

  bool              eligibility;
  string::size_type idx;
  idx = eligible_contents.find( interest->getName().toUri() );
  if ( idx == string::npos ) {
    eligibility = false;
  } else {
    eligibility = true;
  }

  if ( interest->getValidationFlag() == 1 && eligibility &&
       LocationRegistration == 0 ) {
    // 把具有有效性要求的内容存入PITListStore中

    int                           tnow_int = (int) tnow;
    default_random_engine         update_time_e( r() );
    uniform_int_distribution<int> update_time_u( 1, 2 * m_average_update_time -
                                                        1 );

    struct PITListEntry pe;
    pe.name        = interest->getName();
    pe.PITList     = interest->getPITList();
    pe.insert_time = tnow_int;
    pe.update_time = update_time_u( update_time_e );
    default_random_engine         last_update_time_e( r() );
    uniform_int_distribution<int> last_update_time_u(
        tnow_int - pe.update_time + 1, tnow_int );
    pe.last_update_time = last_update_time_u( last_update_time_e );

    // cout << pe.insert_time << " : " << pe.update_time << " : "
    //      << pe.last_update_time << endl;

    if ( !PITListStore.empty() ) {
      // 检查是否有可以合并的PITlist
      std::list<PITListEntry>::iterator it;
      std::list<PITListEntry>::iterator next = PITListStore.end();
      for ( it = PITListStore.begin(); it != PITListStore.end(); it = next ) {
        next = ++it;
        --it;
        // cout << it->PITList+"PITList" << " ---- " <<
        // interest->getPITList()+"PITList" << endl;
        if ( it->PITList == interest->getPITList() &&
             it->name == interest->getName() ) {
          pe.insert_time      = it->insert_time;
          pe.update_time      = it->update_time;
          pe.last_update_time = it->last_update_time;
          PITListStore.erase( it );
          // cout << "!" << endl;
        } else if ( ( it->PITList + "PITList" )
                            .find( interest->getPITList() + "PITList" ) !=
                        string::npos &&
                    it->name == interest->getName() ) {
          // cout << "!!" << endl;
          // cout << it->PITList << " ---- " << pe.PITList << endl;
          pe.PITList          = it->PITList;
          pe.insert_time      = it->insert_time;
          pe.update_time      = it->update_time;
          pe.last_update_time = it->last_update_time;
          PITListStore.erase( it );
          //
          // cout<< "!!"<<endl;
        } else if ( ( interest->getPITList() + "PITList" )
                            .find( it->PITList + "PITList" ) != string::npos &&
                    it->name == interest->getName() ) {
          // cout << "!!!" << endl;
          // cout << it->PITList << " ---- " << pe.PITList << endl;
          pe.insert_time      = it->insert_time;
          pe.update_time      = it->update_time;
          pe.last_update_time = it->last_update_time;
          PITListStore.erase( it );
          // cout<< "!!!"<<endl;
        }
      }
    }
    // add by kan 20190411
    // PITListStore达到最大缓存时，删除末尾记录，在头部插入新记录
    if ( PITListStore.size() == m_max_pitstore_size ) {
      struct PITListEntry temp = PITListStore.back();
      Name                dataName( temp.name );
      auto                data = make_shared<Data>();
      data->setName( dataName );
      data->setFreshnessPeriod(
          ::ndn::time::milliseconds( m_freshness.GetMilliSeconds() ) );

      data->setContent( make_shared<::ndn::Buffer>( m_virtualPayloadSize ) );

      // 设置有有效性要求的数据包字段
      data->setValidationDataFlag( 1 );
      data->setExpiration(
          1 ); // PITListStore不再发布此数据包，用户此后请求到的可能是过期内容
      data->setPITListBack( temp.PITList );
      data->setValidationPublishment( 1 );
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
      NS_LOG_INFO( "expirationMessage" );
      // cout << "ROOT: " <<data->getName() << " : " << data->getPITListBack()
      // <<endl;

      PITListStore.pop_back(); // 删除表尾元素
      if ( (int) tnow >= 41 && (int) tnow <= ( m_expriment_time - 10 ) )
        expiration_count++;
      // cout << expiration_count << endl;

      // if ( (int) tnow >= 41 && (int) tnow <= 340 ) expiration_count++;
    }
    PITListStore.push_front( pe );
    // std::cout << PITListStore.size() << std::endl;
  }
  if ( LocationRegistration == 0 ) {
    // std::cout << tnow << std::endl;
    std::cout << PITListStore.size() << std::endl;
    // std::cout << PITListStore.back().name << std::endl;
    if ( (int) ( tnow * 10 ) % 10 != 0 ) {
      published = false;
    }
    // std::cout << "false " << tnow << std::endl;
    if ( (int) ( tnow * 10 ) % 10 == 0 && (int) tnow != 0 ) {
      // 每秒检测一次PITList中是否有过期内容（即需要更新的内容），并重新分配内容的更新时间
      // std::cout << published << std::endl;
      if ( !published ) {
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << tnow
                  << std::endl;
        // std::cout << PITListStore.size() << std::endl;
        published = true;
        for ( std::list<PITListEntry>::iterator it = PITListStore.begin();
              it != PITListStore.end(); it++ ) {
          if ( ( (int) tnow - it->last_update_time >= it->update_time ) ) {
            // 当前时间-上次更新时间若大于等于更新时间，则需要重新发布一次，
            // 并将上次更新时间置为当前时间。
            it->last_update_time = (int) tnow;

            Name dataName( it->name );
            auto data = make_shared<Data>();
            data->setName( dataName );
            data->setFreshnessPeriod(
                ::ndn::time::milliseconds( m_freshness.GetMilliSeconds() ) );

            data->setContent(
                make_shared<::ndn::Buffer>( m_virtualPayloadSize ) );

            // 设置有有效性要求的数据包字段
            data->setValidationDataFlag( 1 );
            data->setExpiration( 0 );
            data->setPITListBack( it->PITList );
            data->setValidationPublishment( 1 );
            data->setEligibility( 1 );

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
            NS_LOG_INFO( "publication data" );

            default_random_engine             update_factor_e( r() );
            uniform_real_distribution<double> update_factor_u( 0, 1 );
            if ( update_factor >= update_factor_u( update_factor_e ) ) {
              // cout<<"update!!!"<<endl;
              // 随机因子越靠近1，内容的更新时间越有可能发生变化
              default_random_engine         update_time_e2( r() );
              uniform_int_distribution<int> update_time_u2(
                  1, 2 * m_average_update_time - 1 );
              it->update_time = update_time_u2( update_time_e2 );
            }
          }
        }
      }
    }
  }
  // end add
  if ( LocationRegistration != 1 ) {
    // 服务器不响应位置注册型的兴趣包
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
    signature.setValue(::ndn::nonNegativeIntegerBlock(
        ::ndn::tlv::SignatureValue, m_signature ) );

    data->setSignature( signature );

    if ( interest->getValidationFlag() == 1 ) {
      // 设置有有效性要求的数据包字段
      data->setValidationDataFlag( 1 );
      data->setExpiration( 0 );
      data->setPITListBack( interest->getPITList() );
      data->setValidationPublishment( 0 );
      if ( eligibility )
        data->setEligibility( 1 );
      else
        data->setEligibility( 0 );
    } else {
      data->setValidationDataFlag( 0 );
      data->setExpiration( 0 );
      data->setPITListBack( "" );
      data->setValidationPublishment( 0 );
      data->setEligibility( 0 );
    }
    // data->setValidationDataFlag( 0 );
    // data->setExpiration( 0 );
    // data->setPITListBack( "" );

    NS_LOG_INFO( "node(" << GetNode()->GetId()
                         << ") responding with Data: " << data->getName() );

    // to create real wire encoding
    data->wireEncode();

    m_transmittedDatas( data, this, m_face );
    m_face->onReceiveData( *data );
  } else {
    // cout<<" **NOT** responding!!!"<<endl;
    NS_LOG_INFO( "node(" << GetNode()->GetId()
                         << ") **NOT** responding with Data: "
                         << interest->getName() );
  }
}

} // namespace ndn
} // namespace ns3
