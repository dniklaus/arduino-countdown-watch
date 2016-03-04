/*
 * ProdIvm.h
 *
 *  Created on: 04.03.2016
 *      Author: dniklaus
 */

#ifndef PRODIVM_H_
#define PRODIVM_H_

#include <Ivm.h>

/**
 * Product Inventory Management Capabilities (cumulative)
 * - Version 0: DeviceId (addr 0)
 * - Version 1: IVMVersion (addr 1)
 " - Version 2: secs (addr 2), mins (addr 3), hours (addr 4), sign (addr 5)
 */
class ProdIvm: public Ivm
{
public:
  ProdIvm();
  virtual ~ProdIvm();

  void setSecs(byte secs);
  byte getSecs();

  void setMins(byte mins);
  byte getMins();

  void setHours(byte hours);
  byte getHours();

  void setSign(bool isSignSet);
  bool isSignSet();

private:
  void maintainVersionChange();
  void writeToIvm(const unsigned int addr, const char* in, int length);
  int readFromIvm(const unsigned int addr, char* out, int length);

private:
  /**
   * Current Version of the Product Inventory Management Capabilities.
   */
  const static byte s_currentVersion;

  const static byte s_ivmAddrSecs;
  const static byte s_ivmAddrMins;
  const static byte s_ivmAddrHours;
  const static byte s_ivmAddrSign;

private: // forbidden default functions
  ProdIvm& operator = (const ProdIvm& );  // assignment operator
  ProdIvm(const ProdIvm& src);            // copy constructor
};

#endif /* PRODIVM_H_ */
