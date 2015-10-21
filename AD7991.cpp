/*
AD7991.cpp Library for reading data from AD7991 12-bit ADC (some generalizations for 799x family)
Last update 9/3/2015
John Freudenthal and Sean Kirkpatrick
*/

#include "AD7991.h"
// //LowByteShift changes across the AD799X family.
// // For AD7991, 0 shift. For AD7995, 2 shift. For AD7999, 4 shift.
// #define LowByteShift 0;
#define combine(high,low) ( ( (uint16_t)(high << 8) ) | (uint16_t)(low) )
#define lowbyte(value) ( (uint8_t)(value) )
#define highbyte(value) ( (uint8_t)(value>>8) )


AD7991::AD7991()
{
	I2CFilterMode = i2CFilterMode::On;
	ReferenceMode = AD7991ReferenceMode::Supply;
	// ReferenceMode = AD7991ReferenceMode::External;
	SampleDelayMode = sampleDelayMode::On;
	NeedToUpdateConfigByte = true;
	Address = DefaultAddress;
}
AD7991::~AD7991()
{

}
uint8_t AD7991::getAddress()
{
	return Address;
}
void AD7991::setAddress(uint8_t address)
{
	Address = address;
}
bool AD7991::isConnected()
{
	int Status = 5;
	Wire.beginTransmission(Address);
	Status = Wire.endTransmission();
	if (Status == 0)
	{
		return true;
	}	
	else
	{
		return false;
	}
}
float AD7991::getVoltageSingle(uint8_t Channel)
{
	UpdateChannelSingle(Channel);
	float CurrentData;
	RecieveI2CFloat(&CurrentData, 1);
	return CurrentData;
}
void AD7991::getVoltageMultiple(float* Data, uint8_t DACsToUse)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(DACsToUse);
	RecieveI2CFloat(Data, (size_t)NumberOfChannelsToUse);
}
void AD7991::getVoltageSingleRepeat(float* Data, uint8_t Channel, size_t NumberOfRepeats)
{
	UpdateChannelSingle(Channel);
	RecieveI2CFloat(Data, NumberOfRepeats);
}
void AD7991::getVoltageMultipleRepeat(float* Data, uint8_t DACsToUse, size_t NumberOfRepeats)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(DACsToUse);
	RecieveI2CFloat(Data, (size_t)NumberOfChannelsToUse * NumberOfRepeats);
}
uint16_t AD7991::getVoltageSingleInt(uint8_t Channel)
{
	UpdateChannelSingle(Channel);
	uint16_t CurrentData;
	RecieveI2CInt(&CurrentData, (size_t)1);
	return CurrentData;
}
void AD7991::getVoltageMultipleInt(uint16_t* Data, uint8_t DACsToUse)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(DACsToUse);
	RecieveI2CInt(Data, (size_t)NumberOfChannelsToUse);
}
void AD7991::getVoltageSingleRepeatInt(uint16_t* Data, uint8_t Channel, size_t NumberOfRepeats)
{
	UpdateChannelSingle(Channel);
	RecieveI2CInt(Data, NumberOfRepeats);
}
void AD7991::getVoltageMultipleRepeatInt(uint16_t* Data, uint8_t DACsToUse, size_t NumberOfRepeats)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(DACsToUse);
	RecieveI2CInt(Data, (size_t)NumberOfChannelsToUse * NumberOfRepeats);
}
float AD7991::getVRef()
{
	if (ReferenceMode == AD7991ReferenceMode::Supply)
	{
		return VRefInt;
	}else{
		return getVRefExt();
	}
}
bool AD7991::setI2CFilter(i2CFilterMode ModeSetting)
{
	if (ModeSetting != I2CFilterMode)
	{
		I2CFilterMode = ModeSetting;
		NeedToUpdateConfigByte = true;
	}
	return true;
}
i2CFilterMode AD7991::getI2CFilter()
{
	return I2CFilterMode;
}
bool AD7991::setSampleDelayMode(sampleDelayMode ModeSetting)
{
	if (ModeSetting != SampleDelayMode)
	{
		SampleDelayMode = ModeSetting;
		NeedToUpdateConfigByte = true;
	}
	return true;
}
sampleDelayMode AD7991::getSampleDelayMode()
{
	return SampleDelayMode;
}

bool AD7991::setReference(AD7991ReferenceMode ModeSetting)
{
	if (ModeSetting != ReferenceMode)
	{
		ReferenceMode = ModeSetting;
		NeedToUpdateConfigByte = true;
	}
	return true;
}
AD7991ReferenceMode AD7991::getReference()
{
	return ReferenceMode;
}
void AD7991::setVRefExt(float VRef)
{
	VRefExt = VRef;
}
float AD7991::getVRefExt()
{
	return VRefExt;
}
void AD7991::UpdateChannelSingle(uint8_t Channel)
// Channel is an int, 0 <= i <=3.  must be translated to the 
{
	uint8_t NewDACsActive = B00000000;
	Channel = CheckChannel(Channel);
	bitWrite(NewDACsActive, Channel, 1);
	if ( (NewDACsActive != DACsActive) | NeedToUpdateConfigByte)
	{
		NeedToUpdateConfigByte = true;
		DACsActive = NewDACsActive;
		UpdateConfigByte();
	}
}
uint8_t AD7991::UpdateChannelDACsActive(uint8_t NewDACsActive)
{
	if (bitRead(NewDACsActive,VRefChannel) & (ReferenceMode == AD7991ReferenceMode::External))
	{
		Serial.println("Can not use external reference with VRef channel on AD799X.");
		bitWrite(NewDACsActive,VRefChannel,0);
	}
	if ( (NewDACsActive != DACsActive) | NeedToUpdateConfigByte)
	{
		NeedToUpdateConfigByte = true;
		DACsActive = NewDACsActive;
		UpdateConfigByte();
	}
	return (uint8_t)(bitRead(DACsActive,0) + bitRead(DACsActive,1) + bitRead(DACsActive,2) + bitRead(DACsActive,3));
}
uint8_t AD7991::CheckChannel(uint8_t Channel)
//Channel should be int between 0 and 3.  External reference voltage is on channel 3, so this function
//	checks for out of bounds channel request and conflicts on channel 3
{
	Channel = constrain(Channel,0,MaxChannels-1);
	if (Channel == VRefChannel)
	{
		if (ReferenceMode == AD7991ReferenceMode::External)
		{
			Serial.println("Can not use external reference with VRef channel on AD799X.");
			return VRefChannel-1;	//return next lower channel
		}
	}
	return Channel;
}
void AD7991::UpdateConfigByte()
{
	if (NeedToUpdateConfigByte)
	{
		SetConfigByte();
		SendI2C();
		NeedToUpdateConfigByte = false;
	}
}
void AD7991::SetConfigByte()
{
	ConfigByte = DACsActive << 4;
	switch(ReferenceMode)
	{
		case AD7991ReferenceMode::External:
			bitWrite(ConfigByte,3,1);
			break;
		case AD7991ReferenceMode::Supply:
			break;
		case AD7991ReferenceMode::Unknown:
			break;
		default:
			break;
	}
	switch(I2CFilterMode)
	{
		case i2CFilterMode::Off:
			bitWrite(ConfigByte,2,1);
			break;
		case i2CFilterMode::On:
			break;
		case i2CFilterMode::Unknown:
			break;
		default:
			break;
	}
	switch(SampleDelayMode)
	{
		case sampleDelayMode::Off:
			bitWrite(ConfigByte,1,1);
			bitWrite(ConfigByte,0,1);
			break;
		case sampleDelayMode::On:
			break;
		case sampleDelayMode::Unknown:
			break;
		default:
			break;
	}
}
void AD7991::SendI2C()
{
  bool MoveOn = false;
  const int MaxAttempts = 16;
  int CurrentAttempt = 0;
  int SendSuccess = 5;
  while (!MoveOn)
  {
	Wire.beginTransmission(Address);
	Wire.write(ConfigByte);
	SendSuccess = Wire.endTransmission(I2C_STOP, I2CTimeout);
    if(SendSuccess != 0)
    {
      Wire.finish();
      Wire.resetBus();
      CurrentAttempt++;
      if (CurrentAttempt > MaxAttempts)
      {
        MoveOn = true;
        Serial.println("Unrecoverable I2C transmission error with AD799X.");
      }
    }
    else
    {
      MoveOn = true;
    }
  }
}
size_t AD7991::RecieveI2CInt(uint16_t* Data, size_t NumberOfSamples)
{
  if(NumberOfSamples > 0)
  {
    Wire.requestFrom(Address, 2*NumberOfSamples, I2C_STOP, I2CTimeout*2*NumberOfSamples);
    int CurrentIndex = 0;
    while (Wire.available())
    {
	    MSBByte = Wire.readByte();
	    LSBByte = Wire.readByte();
	    uint16_t CurrentValue = combine( ((MSBByte << 4)>>4), LSBByte ) >> LowByteShift;
	    Data[CurrentIndex] = CurrentValue;
	    CurrentIndex++;
	}
	return CurrentIndex;
  }
  else
  {
  	return 0;
  }
}
size_t AD7991::RecieveI2CFloat(float* Data, size_t NumberOfSamples)
{
  if(NumberOfSamples > 0)
  {
  	float MaxValue = (float)(pow(2,12-LowByteShift)-1);
    Wire.requestFrom(Address, 2*NumberOfSamples, I2C_STOP, I2CTimeout*2*NumberOfSamples);
    int CurrentIndex = 0;
    while (Wire.available())
    {
	    MSBByte = Wire.readByte();
	    MSBByte = MSBByte & B00001111;
	    LSBByte = Wire.readByte();
	    uint16_t CurrentValue = combine(MSBByte, LSBByte ) >> LowByteShift;
	    Data[CurrentIndex] = getVRef()*((float)CurrentValue)/MaxValue;
	    CurrentIndex++;
	}
	return CurrentIndex;
  }
  else
  {
  	return 0;
  }
}