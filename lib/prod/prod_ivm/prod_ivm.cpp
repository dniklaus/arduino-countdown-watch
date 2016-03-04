/*
 * ProdIvm.cpp
 *
 *  Created on: 04.03.2016
 *      Author: dniklaus
 */

#include "Ivm.h"
#include "IvmSerialEeprom.h"
#include "prod_ivm.h"
#include "string.h"

const unsigned char ProdIvm::s_currentVersion = 2;

const byte ProdIvm::s_ivmAddrSecs  = 2;
const byte ProdIvm::s_ivmAddrMins  = 3;
const byte ProdIvm::s_ivmAddrHours = 4;
const byte ProdIvm::s_ivmAddrSign  = 5;


ProdIvm::ProdIvm()
: Ivm(new IvmSerialEeprom())
{
  maintainVersionChange();
}

ProdIvm::~ProdIvm()
{
  delete getIvmMemory();
  setIvmMemory(0);
}

void ProdIvm::maintainVersionChange()
{
  unsigned char ivmVersion = getIvmVersion();

  if (255 == ivmVersion || 0 == ivmVersion)
  {
    // Assume uninitialized IVM memory, bring up to current version
    unsigned char ivmVersion = getIvmVersion();

    unsigned char deviceId   = getDeviceId();
    // Initialize Device ID
    if (255 == deviceId)
    {
      // Assume version uninitialized, initialize to zero.
      setDeviceId(0);
      deviceId = 0;
    }
  }

  if ((s_currentVersion - 1) >= ivmVersion)
  {
    // IVMVersion not present, write current version
    setIvmVersion(s_currentVersion);

    if (2 > ivmVersion)
    {
      // initialize Version 2 feature: BatteryVoltageSenseFactor, set default value
      setSecs(10);
      setMins(0);
      setHours(0);
      setSign(true);
    }
  }
}

void ProdIvm::setSecs(byte secs)
{
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    ivmMemory->write(s_ivmAddrSecs, secs);
  }
}

byte ProdIvm::getSecs()
{
  byte secs = 0;
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    secs = ivmMemory->read(s_ivmAddrSecs);
  }
  return secs;
}

void ProdIvm::setMins(byte mins)
{
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    ivmMemory->write(s_ivmAddrMins, mins);
  }
}

byte ProdIvm::getMins()
{
  byte mins = 0;
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    mins = ivmMemory->read(s_ivmAddrMins);
  }
  return mins;
}

void ProdIvm::setHours(byte hours)
{
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    ivmMemory->write(s_ivmAddrHours, hours);
  }
}

byte ProdIvm::getHours()
{
  byte hours = 0;
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    hours = ivmMemory->read(s_ivmAddrHours);
  }
  return hours;
}

void ProdIvm::setSign(bool isSignSet)
{
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    ivmMemory->write(s_ivmAddrSign, isSignSet ? 1 : 0);
  }
}

bool ProdIvm::isSignSet()
{
  bool sign = false;
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    sign = (ivmMemory->read(s_ivmAddrSign) >= 1);
  }
  return sign;
}

void ProdIvm::writeToIvm(const unsigned int addr, const char* in, int length)
{
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    for(unsigned int i = 0; i < length; ++i)
    {
      ivmMemory->write(addr+i, in[i]);
    }
  }
}

int ProdIvm::readFromIvm(const unsigned int addr, char* out, int length)
{
  unsigned int i = 0;
  IF_IvmMemory* ivmMemory = getIvmMemory();
  if (0 != ivmMemory)
  {
    for(i = 0; i < length; ++i)
    {
      out[i] = ivmMemory->read(addr+i);
    }
  }
  return i;
}
