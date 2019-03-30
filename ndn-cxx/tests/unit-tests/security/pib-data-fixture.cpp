/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#include "pib-data-fixture.hpp"
#include "../identity-management-time-fixture.hpp"

/**
 * The test data can be generated with a TestCertDataGenerator defined as below:
 *
 * TestCertDataGenerator g;
 *
 * g.printTestDataForId(Name("/pib/interface/id/1"));
 * g.printTestDataForId(Name("/pib/interface/id/2"));
 *
 * class TestCertDataGenerator : public IdentityManagementTimeFixture
 * {
 * public:
 *   void
 *   printTestDataForId(const Name& id)
 *   {
 *     addIdentity(id, EcdsaKeyParams());
 *
 *     Name key1Name = m_keyChain.getDefaultKeyNameForIdentity(id);
 *     shared_ptr<PublicKey> key1 = m_keyChain.getPublicKey(key1Name);
 *     printBytes(key1->get());
 *
 *     Name key1Cert1Name = m_keyChain.getDefaultCertificateNameForKey(key1Name);
 *     shared_ptr<IdentityCertificate> key1Cert1 = m_keyChain.getCertificate(key1Cert1Name);
 *     printBytes(key1Cert1->wireEncode());
 *
 *     Name key2Name = m_keyChain.generateEcdsaKeyPair(id, true);
 *     shared_ptr<PublicKey> key2 = m_keyChain.getPublicKey(key2Name);
 *     printBytes(key2->get());
 *
 *     shared_ptr<IdentityCertificate> key2Cert1 = m_keyChain.selfSign(key2Name);
 *     printBytes(key2Cert1->wireEncode());
 *
 *     advanceClocks(time::seconds(20));
 *
 *     shared_ptr<IdentityCertificate> key1Cert2 = m_keyChain.selfSign(key1Name);
 *     printBytes(key1Cert2->wireEncode());
 *
 *     shared_ptr<IdentityCertificate> key2Cert2 = m_keyChain.selfSign(key2Name);
 *     printBytes(key2Cert2->wireEncode());
 *   }
 *
 *   void
 *   printBytes(const Block& block)
 *   {
 *     printBytes(block.wire(), block.size());
 *   }
 *
 *   void
 *   printBytes(const Buffer& buffer)
 *   {
 *     printBytes(buffer.buf(), buffer.size());
 *   }
 *
 *   void
 *   printBytes(const uint8_t* buf, size_t size)
 *   {
 *     using namespace CryptoPP;
 *
 *     std::string hex = toHex(buf, size);
 *
 *     for (int i = 0; i < hex.size(); i++) {
 *       if (i % 40 == 0)
 *         std::cout << std::endl;
 *
 *       std::cout << "0x" << hex[i];
 *       std::cout << hex[++i];
 *
 *       if ((i + 1) != hex.size())
 *         std::cout << ", ";
 *     }
 *     std::cout << std::endl;
 *   }
 * };
 */

uint8_t ID1_KEY1[] = {
  0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
  0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xFF, 0xF9, 0x1E, 0x85, 0x6C, 0x29, 0x5F, 0x98, 0xB1, 0x2E, 0xD5, 0x3D, 0xCA,
  0xE2, 0x00, 0x52, 0x7A, 0x55, 0x93, 0x96, 0xD1, 0x7F, 0x03, 0x20, 0x25, 0xA7, 0xE5, 0xB8, 0xF8, 0x5D, 0xF0, 0x2E, 0x3E,
  0x60, 0x40, 0x19, 0x73, 0x00, 0x4F, 0x5A, 0xA7, 0x66, 0xFB, 0x38, 0xE6, 0xEB, 0xD5, 0xA4, 0x32, 0x1F, 0x5F, 0xC6, 0x7D,
  0x4B, 0xD4, 0xBB, 0x1E, 0x15, 0x29, 0x3E, 0x40, 0x22, 0x4E, 0xE7
};

uint8_t ID1_KEY1_CERT1[] = {
  0x06, 0xFD, 0x01, 0x88, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0x8C, 0xA0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x33,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x33, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x31, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xFF, 0xF9, 0x1E, 0x85, 0x6C, 0x29,
  0x5F, 0x98, 0xB1, 0x2E, 0xD5, 0x3D, 0xCA, 0xE2, 0x00, 0x52, 0x7A, 0x55, 0x93, 0x96, 0xD1, 0x7F, 0x03, 0x20, 0x25, 0xA7,
  0xE5, 0xB8, 0xF8, 0x5D, 0xF0, 0x2E, 0x3E, 0x60, 0x40, 0x19, 0x73, 0x00, 0x4F, 0x5A, 0xA7, 0x66, 0xFB, 0x38, 0xE6, 0xEB,
  0xD5, 0xA4, 0x32, 0x1F, 0x5F, 0xC6, 0x7D, 0x4B, 0xD4, 0xBB, 0x1E, 0x15, 0x29, 0x3E, 0x40, 0x22, 0x4E, 0xE7, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x47, 0x30, 0x45, 0x02, 0x21, 0x00, 0xAE, 0x85, 0xB6, 0xDE, 0x1B, 0x3B, 0xC0, 0x46, 0x93, 0xEF,
  0x49, 0x26, 0x98, 0x4B, 0x20, 0x77, 0xAB, 0xF6, 0x83, 0x41, 0x72, 0x1D, 0x99, 0xBE, 0x85, 0xC9, 0xC8, 0xA6, 0x14, 0x50,
  0xA6, 0x3E, 0x02, 0x20, 0x5A, 0xC6, 0x1F, 0xF7, 0x72, 0xB4, 0x3A, 0xA1, 0x1D, 0x5E, 0xF7, 0xF3, 0x3C, 0x83, 0xF7, 0xD8,
  0x27, 0x13, 0x65, 0x4D, 0x5D, 0x1A, 0x23, 0x5F, 0xA9, 0xFC, 0x53, 0x22, 0x86, 0xBD, 0x92, 0x01
};

uint8_t ID1_KEY1_CERT2[] = {
  0x06, 0xFD, 0x01, 0x88, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0xB3, 0xB0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x34,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x34, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x31, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xFF, 0xF9, 0x1E, 0x85, 0x6C, 0x29,
  0x5F, 0x98, 0xB1, 0x2E, 0xD5, 0x3D, 0xCA, 0xE2, 0x00, 0x52, 0x7A, 0x55, 0x93, 0x96, 0xD1, 0x7F, 0x03, 0x20, 0x25, 0xA7,
  0xE5, 0xB8, 0xF8, 0x5D, 0xF0, 0x2E, 0x3E, 0x60, 0x40, 0x19, 0x73, 0x00, 0x4F, 0x5A, 0xA7, 0x66, 0xFB, 0x38, 0xE6, 0xEB,
  0xD5, 0xA4, 0x32, 0x1F, 0x5F, 0xC6, 0x7D, 0x4B, 0xD4, 0xBB, 0x1E, 0x15, 0x29, 0x3E, 0x40, 0x22, 0x4E, 0xE7, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x47, 0x30, 0x45, 0x02, 0x20, 0x12, 0xA8, 0xF5, 0x30, 0xE1, 0x57, 0xD1, 0x39, 0xB2, 0x46, 0x0A,
  0x82, 0x58, 0x8A, 0xCD, 0xB3, 0x6F, 0x2F, 0x1B, 0xB8, 0x6A, 0x99, 0xAB, 0x0C, 0xB2, 0xB7, 0xE2, 0x01, 0xE4, 0xD8, 0xC8,
  0x9C, 0x02, 0x21, 0x00, 0xDA, 0xCD, 0x11, 0x28, 0x96, 0xC6, 0xB6, 0x31, 0x6E, 0xDF, 0xCA, 0x79, 0xDE, 0x26, 0x44, 0xCA,
  0x09, 0x74, 0xF1, 0xB1, 0x7C, 0x9B, 0xFA, 0x67, 0x22, 0x55, 0x18, 0xA5, 0x05, 0x48, 0x7D, 0x65
};

uint8_t ID1_KEY2[] = {
  0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
  0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xC2, 0xE0, 0xEC, 0xC7, 0xED, 0x65, 0xDE, 0x0A, 0x46, 0xCE, 0x38, 0xC2, 0x68,
  0x77, 0x4F, 0xE3, 0xE1, 0xDF, 0x37, 0x7D, 0xA3, 0x56, 0x0D, 0xF9, 0x66, 0x43, 0x37, 0x60, 0x42, 0x7E, 0x96, 0x93, 0x7E,
  0x35, 0xA0, 0xD5, 0xC8, 0x59, 0x8F, 0x36, 0x85, 0x11, 0xBF, 0xFA, 0x85, 0x1A, 0x7B, 0x61, 0xE6, 0xEB, 0xD1, 0x46, 0x99,
  0x67, 0x6B, 0xDB, 0x83, 0x26, 0x1F, 0x75, 0x7A, 0x93, 0xA2, 0xAE
};

uint8_t ID1_KEY2_CERT1[] = {
  0x06, 0xFD, 0x01, 0x88, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0xDA, 0xC0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x35,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x35, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x31, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xC2, 0xE0, 0xEC, 0xC7, 0xED, 0x65,
  0xDE, 0x0A, 0x46, 0xCE, 0x38, 0xC2, 0x68, 0x77, 0x4F, 0xE3, 0xE1, 0xDF, 0x37, 0x7D, 0xA3, 0x56, 0x0D, 0xF9, 0x66, 0x43,
  0x37, 0x60, 0x42, 0x7E, 0x96, 0x93, 0x7E, 0x35, 0xA0, 0xD5, 0xC8, 0x59, 0x8F, 0x36, 0x85, 0x11, 0xBF, 0xFA, 0x85, 0x1A,
  0x7B, 0x61, 0xE6, 0xEB, 0xD1, 0x46, 0x99, 0x67, 0x6B, 0xDB, 0x83, 0x26, 0x1F, 0x75, 0x7A, 0x93, 0xA2, 0xAE, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x47, 0x30, 0x45, 0x02, 0x20, 0x24, 0x26, 0x28, 0xA1, 0xD2, 0xCA, 0x46, 0xDB, 0x15, 0x5A, 0xB8,
  0x15, 0x58, 0x3B, 0x1C, 0xEC, 0xDF, 0x9E, 0xF9, 0x35, 0x32, 0x61, 0x0B, 0xC3, 0x9B, 0xA2, 0x1F, 0x05, 0xAA, 0x04, 0xE4,
  0x40, 0x02, 0x21, 0x00, 0xEA, 0x37, 0xA3, 0x1E, 0xD6, 0x20, 0x73, 0xD8, 0x55, 0xE6, 0x62, 0xB1, 0x23, 0xBC, 0x32, 0x08,
  0x1A, 0x0F, 0x4B, 0x94, 0xBE, 0x28, 0xCE, 0xE7, 0x0A, 0x8A, 0xB4, 0xD5, 0xEA, 0x8D, 0x20, 0x95
};

uint8_t ID1_KEY2_CERT2[] = {
  0x06, 0xFD, 0x01, 0x88, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x5A, 0x01, 0xD0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x36, 0x30,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x36, 0x30, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x31, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xC2, 0xE0, 0xEC, 0xC7, 0xED, 0x65,
  0xDE, 0x0A, 0x46, 0xCE, 0x38, 0xC2, 0x68, 0x77, 0x4F, 0xE3, 0xE1, 0xDF, 0x37, 0x7D, 0xA3, 0x56, 0x0D, 0xF9, 0x66, 0x43,
  0x37, 0x60, 0x42, 0x7E, 0x96, 0x93, 0x7E, 0x35, 0xA0, 0xD5, 0xC8, 0x59, 0x8F, 0x36, 0x85, 0x11, 0xBF, 0xFA, 0x85, 0x1A,
  0x7B, 0x61, 0xE6, 0xEB, 0xD1, 0x46, 0x99, 0x67, 0x6B, 0xDB, 0x83, 0x26, 0x1F, 0x75, 0x7A, 0x93, 0xA2, 0xAE, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x31, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x47, 0x30, 0x45, 0x02, 0x20, 0x3B, 0x82, 0xAE, 0xFD, 0x8B, 0x6D, 0xBA, 0x1D, 0x77, 0x70, 0x86,
  0xAB, 0xF1, 0x37, 0x38, 0x90, 0x04, 0x83, 0x33, 0xF8, 0xF2, 0x2C, 0xD6, 0x50, 0x32, 0x19, 0xEA, 0xD6, 0xBB, 0x40, 0x58,
  0x8F, 0x02, 0x21, 0x00, 0x93, 0x01, 0x54, 0x5C, 0x8C, 0x60, 0x81, 0x89, 0xBE, 0x5E, 0x42, 0x31, 0x39, 0xF8, 0x12, 0xFD,
  0x31, 0x48, 0xB0, 0x96, 0x41, 0x40, 0x98, 0x68, 0xF9, 0x7C, 0x01, 0x94, 0xD0, 0xA3, 0xF3, 0xC7
};

uint8_t ID2_KEY1[] = {
  0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
  0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xA4, 0x25, 0xDB, 0xB0, 0xD7, 0xC6, 0x0D, 0xC6, 0x95, 0x97, 0x79, 0xFA, 0xE3,
  0xC7, 0x90, 0xFB, 0x97, 0xAF, 0xCE, 0xDB, 0xC3, 0x50, 0x99, 0x1E, 0x39, 0xF5, 0x9A, 0xB6, 0xC9, 0x37, 0x1A, 0xE5, 0x0A,
  0x56, 0xE0, 0x0C, 0x0D, 0x81, 0xC7, 0x29, 0xE4, 0x69, 0x06, 0xD1, 0x4A, 0x14, 0x75, 0x05, 0x95, 0xBE, 0xE7, 0x01, 0x45,
  0x3C, 0xA7, 0x99, 0x09, 0x05, 0x9F, 0x65, 0x9A, 0xA5, 0x9C, 0xD5
};

uint8_t ID2_KEY1_CERT1[] = {
  0x06, 0xFD, 0x01, 0x89, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0x8C, 0xA0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x33,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x33, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x32, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xA4, 0x25, 0xDB, 0xB0, 0xD7, 0xC6,
  0x0D, 0xC6, 0x95, 0x97, 0x79, 0xFA, 0xE3, 0xC7, 0x90, 0xFB, 0x97, 0xAF, 0xCE, 0xDB, 0xC3, 0x50, 0x99, 0x1E, 0x39, 0xF5,
  0x9A, 0xB6, 0xC9, 0x37, 0x1A, 0xE5, 0x0A, 0x56, 0xE0, 0x0C, 0x0D, 0x81, 0xC7, 0x29, 0xE4, 0x69, 0x06, 0xD1, 0x4A, 0x14,
  0x75, 0x05, 0x95, 0xBE, 0xE7, 0x01, 0x45, 0x3C, 0xA7, 0x99, 0x09, 0x05, 0x9F, 0x65, 0x9A, 0xA5, 0x9C, 0xD5, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x48, 0x30, 0x46, 0x02, 0x21, 0x00, 0xC4, 0x40, 0x87, 0xA7, 0xAA, 0xA7, 0x12, 0xE2, 0x7C, 0xE2,
  0xA8, 0x27, 0x66, 0xCF, 0x45, 0x3E, 0x3D, 0xEC, 0x2C, 0x5C, 0x03, 0xD6, 0xB0, 0xD6, 0x5E, 0xE8, 0xB7, 0x03, 0x9C, 0x38,
  0x75, 0xEA, 0x02, 0x21, 0x00, 0x8D, 0xF3, 0x9E, 0xC0, 0x18, 0xBD, 0xA1, 0x2B, 0xDB, 0x3D, 0xE0, 0x34, 0x14, 0x34, 0x65,
  0x23, 0x37, 0x6E, 0x3C, 0xC0, 0xC4, 0x2F, 0xED, 0xBB, 0x9B, 0xB2, 0xEC, 0x2A, 0x96, 0xE6, 0xD2, 0x98
};

uint8_t ID2_KEY1_CERT2[] = {
  0x06, 0xFD, 0x01, 0x88, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0xB3, 0xB0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x34,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x34, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x32, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xA4, 0x25, 0xDB, 0xB0, 0xD7, 0xC6,
  0x0D, 0xC6, 0x95, 0x97, 0x79, 0xFA, 0xE3, 0xC7, 0x90, 0xFB, 0x97, 0xAF, 0xCE, 0xDB, 0xC3, 0x50, 0x99, 0x1E, 0x39, 0xF5,
  0x9A, 0xB6, 0xC9, 0x37, 0x1A, 0xE5, 0x0A, 0x56, 0xE0, 0x0C, 0x0D, 0x81, 0xC7, 0x29, 0xE4, 0x69, 0x06, 0xD1, 0x4A, 0x14,
  0x75, 0x05, 0x95, 0xBE, 0xE7, 0x01, 0x45, 0x3C, 0xA7, 0x99, 0x09, 0x05, 0x9F, 0x65, 0x9A, 0xA5, 0x9C, 0xD5, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x33, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x47, 0x30, 0x45, 0x02, 0x21, 0x00, 0xAA, 0xFE, 0x76, 0xBE, 0x2C, 0x4D, 0xDA, 0x41, 0x27, 0x93,
  0x11, 0x70, 0xEE, 0x33, 0x57, 0x37, 0xA2, 0x54, 0x01, 0x97, 0x82, 0x16, 0xB5, 0x8A, 0xFD, 0xE2, 0x5F, 0x2E, 0x05, 0x0A,
  0xF5, 0xE6, 0x02, 0x20, 0x4F, 0xEF, 0x85, 0xC6, 0x91, 0xF3, 0x55, 0x2C, 0x4E, 0x98, 0x81, 0xD7, 0xF0, 0x63, 0x36, 0x91,
  0xB3, 0x88, 0x9D, 0x99, 0x7E, 0x49, 0xD5, 0x72, 0x7F, 0x6F, 0x92, 0xCF, 0x0A, 0x56, 0xA6, 0xF9
};

uint8_t ID2_KEY2[] = {
  0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
  0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x6C, 0x49, 0x20, 0x7E, 0x59, 0xAF, 0x48, 0x1C, 0x9B, 0xCB, 0x67, 0xD4, 0x6F,
  0x43, 0x9D, 0xD8, 0xB5, 0x36, 0xDB, 0x72, 0xDA, 0x37, 0x55, 0x4B, 0x8C, 0x69, 0x17, 0x87, 0xF6, 0x06, 0xAB, 0x06, 0x70,
  0x91, 0xBE, 0x02, 0x79, 0xB1, 0x14, 0x9C, 0xD0, 0x0B, 0x92, 0xD5, 0xC4, 0xF1, 0xEC, 0x23, 0x90, 0x95, 0xCB, 0x7D, 0x59,
  0x62, 0x3B, 0x30, 0xFE, 0xCF, 0x05, 0xBE, 0x04, 0xC9, 0x78, 0x5C
};

uint8_t ID2_KEY2_CERT1[] = {
  0x06, 0xFD, 0x01, 0x89, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x59, 0xDA, 0xC0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x35, 0x35,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x35, 0x35, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x32, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x6C, 0x49, 0x20, 0x7E, 0x59, 0xAF,
  0x48, 0x1C, 0x9B, 0xCB, 0x67, 0xD4, 0x6F, 0x43, 0x9D, 0xD8, 0xB5, 0x36, 0xDB, 0x72, 0xDA, 0x37, 0x55, 0x4B, 0x8C, 0x69,
  0x17, 0x87, 0xF6, 0x06, 0xAB, 0x06, 0x70, 0x91, 0xBE, 0x02, 0x79, 0xB1, 0x14, 0x9C, 0xD0, 0x0B, 0x92, 0xD5, 0xC4, 0xF1,
  0xEC, 0x23, 0x90, 0x95, 0xCB, 0x7D, 0x59, 0x62, 0x3B, 0x30, 0xFE, 0xCF, 0x05, 0xBE, 0x04, 0xC9, 0x78, 0x5C, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x48, 0x30, 0x46, 0x02, 0x21, 0x00, 0xB6, 0x8E, 0x31, 0xFB, 0x08, 0x34, 0xF3, 0x1C, 0xB5, 0x09,
  0x7D, 0xD4, 0x17, 0x45, 0xC7, 0x6A, 0x81, 0xEE, 0x6F, 0x16, 0x76, 0xEE, 0xDC, 0x44, 0xB4, 0xD7, 0x1A, 0xD5, 0x61, 0x57,
  0x80, 0xBD, 0x02, 0x21, 0x00, 0xF7, 0xDB, 0xDF, 0x89, 0xBC, 0xE8, 0x28, 0x26, 0xF8, 0xEE, 0x74, 0x2B, 0x9C, 0xF0, 0x7F,
  0xB8, 0x3A, 0xEE, 0xBF, 0x1F, 0x51, 0x14, 0x6A, 0x8F, 0x2E, 0x5A, 0x60, 0xD8, 0x45, 0x87, 0x62, 0x51
};

uint8_t ID2_KEY2_CERT2[] = {
  0x06, 0xFD, 0x01, 0x89, 0x07, 0x43, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66, 0x61,
  0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B, 0x2D,
  0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43, 0x45,
  0x52, 0x54, 0x08, 0x09, 0xFD, 0x00, 0x00, 0x01, 0x49, 0x9D, 0x5A, 0x01, 0xD0, 0x14, 0x03, 0x18, 0x01, 0x02, 0x15, 0xB2,
  0x30, 0x81, 0xAF, 0x30, 0x22, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x35, 0x33, 0x36, 0x30,
  0x32, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x33, 0x34, 0x31, 0x31, 0x30, 0x36, 0x30, 0x35, 0x33, 0x36, 0x30, 0x32, 0x5A, 0x30,
  0x2E, 0x30, 0x2C, 0x06, 0x03, 0x55, 0x04, 0x29, 0x13, 0x25, 0x2F, 0x70, 0x69, 0x62, 0x2F, 0x69, 0x6E, 0x74, 0x65, 0x72,
  0x66, 0x61, 0x63, 0x65, 0x2F, 0x69, 0x64, 0x2F, 0x32, 0x2F, 0x6B, 0x73, 0x6B, 0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38,
  0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
  0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x6C, 0x49, 0x20, 0x7E, 0x59, 0xAF,
  0x48, 0x1C, 0x9B, 0xCB, 0x67, 0xD4, 0x6F, 0x43, 0x9D, 0xD8, 0xB5, 0x36, 0xDB, 0x72, 0xDA, 0x37, 0x55, 0x4B, 0x8C, 0x69,
  0x17, 0x87, 0xF6, 0x06, 0xAB, 0x06, 0x70, 0x91, 0xBE, 0x02, 0x79, 0xB1, 0x14, 0x9C, 0xD0, 0x0B, 0x92, 0xD5, 0xC4, 0xF1,
  0xEC, 0x23, 0x90, 0x95, 0xCB, 0x7D, 0x59, 0x62, 0x3B, 0x30, 0xFE, 0xCF, 0x05, 0xBE, 0x04, 0xC9, 0x78, 0x5C, 0x16, 0x3F,
  0x1B, 0x01, 0x03, 0x1C, 0x3A, 0x07, 0x38, 0x08, 0x03, 0x70, 0x69, 0x62, 0x08, 0x09, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0x08, 0x02, 0x69, 0x64, 0x08, 0x01, 0x32, 0x08, 0x03, 0x4B, 0x45, 0x59, 0x08, 0x11, 0x6B, 0x73, 0x6B,
  0x2D, 0x31, 0x34, 0x31, 0x35, 0x36, 0x38, 0x34, 0x31, 0x35, 0x32, 0x30, 0x30, 0x30, 0x08, 0x07, 0x49, 0x44, 0x2D, 0x43,
  0x45, 0x52, 0x54, 0x17, 0x48, 0x30, 0x46, 0x02, 0x21, 0x00, 0x9E, 0xA1, 0x49, 0xCB, 0x99, 0xB8, 0xB9, 0xD0, 0x86, 0x93,
  0xB1, 0x5A, 0xD5, 0xAE, 0x2D, 0x32, 0xE8, 0xC1, 0x3F, 0x9E, 0x35, 0x7A, 0x45, 0xD7, 0x2F, 0x79, 0xE3, 0x76, 0x04, 0xCD,
  0x66, 0x70, 0x02, 0x21, 0x00, 0x99, 0x04, 0x0A, 0x35, 0x3B, 0x53, 0x1B, 0x13, 0x14, 0xAC, 0xB5, 0x8E, 0x6F, 0x1A, 0x72,
  0x2C, 0x3D, 0x86, 0xCF, 0xF6, 0x8D, 0x2F, 0x13, 0x60, 0x28, 0xB1, 0x13, 0xFE, 0x49, 0x3B, 0xA4, 0xAD
};

namespace ndn {
namespace security {

PibDataFixture::PibDataFixture()
  : id1("/pib/interface/id/1")
  , id2("/pib/interface/id/2")
  , id1Key1Name("/pib/interface/id/1/ksk-1415684132000")
  , id1Key2Name("/pib/interface/id/1/ksk-1415684152000")
  , id2Key1Name("/pib/interface/id/2/ksk-1415684132000")
  , id2Key2Name("/pib/interface/id/2/ksk-1415684152000")
  , id1Key1(ID1_KEY1, sizeof(ID1_KEY1))
  , id1Key2(ID1_KEY2, sizeof(ID1_KEY2))
  , id2Key1(ID2_KEY1, sizeof(ID2_KEY1))
  , id2Key2(ID2_KEY2, sizeof(ID2_KEY2))
  , id1Key1Cert1(Block(ID1_KEY1_CERT1, sizeof(ID1_KEY1_CERT1)))
  , id1Key1Cert2(Block(ID1_KEY1_CERT2, sizeof(ID1_KEY1_CERT2)))
  , id1Key2Cert1(Block(ID1_KEY2_CERT1, sizeof(ID1_KEY2_CERT1)))
  , id1Key2Cert2(Block(ID1_KEY2_CERT2, sizeof(ID1_KEY2_CERT2)))
  , id2Key1Cert1(Block(ID2_KEY1_CERT1, sizeof(ID2_KEY1_CERT1)))
  , id2Key1Cert2(Block(ID2_KEY1_CERT2, sizeof(ID2_KEY1_CERT2)))
  , id2Key2Cert1(Block(ID2_KEY2_CERT1, sizeof(ID2_KEY2_CERT1)))
  , id2Key2Cert2(Block(ID2_KEY2_CERT2, sizeof(ID2_KEY2_CERT2)))
{
}

} // namespace security
} // namespace ndn