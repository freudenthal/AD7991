#include "Arduino.h"
#include "i2c_t3.h"
#define combine(high,low) ( ( (uint16_t)(high << 8) ) | (uint16_t)(low) )
#define lowbyte(value) ( (uint8_t)(value) )
#define highbyte(value) ( (uint8_t)(value>>8) )
#define MaxChannels 4
#define InternalReference 2.5f
#define DefaultAddress 40
#define VoltageReferenceChannel 3
#define I2CTimeout 1000;
//LowByteShift changes across the AD799X family.
// For AD7991, 0 shift. For AD7995, 2 shift. For AD7999, 4 shift.
#define LowByteShift 0;
using namespace std;
enum class sampleDelayMode{Unknown, On, Off};
enum class i2CFilterMode{Unknown, On, Off};
enum class referenceMode{Unknown, Supply, External};
class AD7991
{
	public:
		AD7991();
		~AD7991();
		bool isConnected();
		uint8_t getAddress();
		void setAddress(uint8_t address);
		float getVoltageSingle(uint8_t Channel);
		void getVoltageMultiple(float* Data, uint8_t DACsToUse);
		void getVoltageSingleRepeat(float* Data, uint8_t Channel, size_t NumberOfRepeats);
		void getVoltageMultipleRepeat(float* Data, uint8_t DACsToUse, size_t NumberOfRepeats);
		uint16_t getVoltageSingleInt(uint8_t Channel);
		void getVoltageMultipleInt(uint16_t* Data, uint8_t DACsToUse);
		void getVoltageSingleRepeatInt(uint16_t* Data, uint8_t Channel, size_t NumberOfRepeats);
		void getVoltageMultipleRepeatInt(uint16_t* Data, uint8_t DACsToUse, size_t NumberOfRepeats);
		bool setI2CFilter(i2CFilterMode ModeSetting);
		i2CFilterMode getI2CFilter();
		bool setSampleDelayMode(sampleDelayMode ModeSetting);
		sampleDelayMode getSampleDelayMode();
		bool setReference(referenceMode ModeSetting);
		referenceMode getReference();
		void setVRefExt(float VRef);
		float getVRefExt();
		float getVRef();
	private:
		uint8_t Address;
		referenceMode ReferenceMode;
		i2CFilterMode I2CFilterMode;
		sampleDelayMode SampleDelayMode;
		uint8_t DACsActive;
		uint8_t ConfigByte;
		uint8_t PriorConfigByte;
		uint8_t MSBByte;
		uint8_t LSBByte;
		bool NeedToUpdateConfigByte;
		const uint8_t VRefChannel = VoltageReferenceChannel;
		const float VRefInt = InternalReference;
		float VRefExt;
};
AD7991::AD7991()
{
	I2CFilterMode = i2CFilterMode::On;
	ReferenceMode = referenceMode::Supply;
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
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(uint8_t NewDACsActive);
	RecieveI2CFloat(Data, (size_t)NumberOfChannelsToUse);
}
void AD7991::getVoltageSingleRepeat(float* Data, uint8_t Channel, size_t NumberOfRepeats)
{
	UpdateChannelSingle(Channel);
	RecieveI2CFloat(Data, NumberOfRepeats);
}
void AD7991::getVoltageMultipleRepeat(float* Data, uint8_t DACsToUse, size_t NumberOfRepeats)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(uint8_t NewDACsActive);
	RecieveI2CFloat(Data, (size_t)NumberOfChannelsToUse * NumberOfRepeats);
}
uint16_t AD7991::getVoltageSingleInt(uint8_t Channel)
{
	UpdateChannelSingle(Channel);
	int CurrentData;
	RecieveI2CInt(&CurrentData, 1);
	return CurrentData;
}
void AD7991::getVoltageMultipleInt(uint16_t* Data, uint8_t DACsToUse)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(uint8_t NewDACsActive);
	RecieveI2CInt(Data, (size_t)NumberOfChannelsToUse);
}
void AD7991::getVoltageSingleRepeatInt(uint16_t* Data, uint8_t Channel, size_t NumberOfRepeats)
{
	UpdateChannelSingle(Channel);
	RecieveI2CInt(Data, NumberOfRepeats);
}
void AD7991::getVoltageMultipleRepeatInt(uint16_t* Data, uint8_t DACsToUse, size_t NumberOfRepeats)
{
	uint8_t NumberOfChannelsToUse = UpdateChannelDACsActive(uint8_t NewDACsActive);
	RecieveI2CInt(Data, (size_t)NumberOfChannelsToUse * NumberOfRepeats);
}
float AD7991::getVRef()
{
	switch(ReferenceMode)
	{
		case referenceMode::Internal:
			return VRefInt;
		case referenceMode::Supply:
		case default:
			return VRefExt;
	}
}
bool AD7991::setI2CFilter(i2CFilterMode ModeSetting)
{
	if (ModeSetting != I2CFilterMode)
	{
		I2CFilterMode = ModeSetting;
		NeedToUpdateConfigByte = true;
	}
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
}
sampleDelayMode AD7991::getSampleDelayMode()
{
	return SampleDelayMode;
}
bool AD7991::setOutputMode(outputMode ModeSetting)
{

}
outputMode AD7991::getOutputMode()
{
	return OutputMode;
}
bool AD7991::setReference(referenceMode ModeSetting)
{
	if (ModeSetting != ReferenceMode)
	{
		ReferenceMode = ModeSetting;
		NeedToUpdateConfigByte = true;
	}
}
referenceMode AD7991::getReference()
{
	return ReferenceMode;
}
void AD7991::setVRefExt(float VRef)
{
	VRefExt = Vref;
}
float AD7991::getVRefExt()
{
	return VRefExt;
}
void AD7991::UpdateChannelSingle(uint8_t Channel)
{
	uint8_t NewDACsActive = 0;
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
	if (bitRead(NewDACsActive,VRefChannel) & (ReferenceMode == referenceMode::External))
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
{
	Channel = constrain(Channel,0,NumberOfChannels);
	if (Channel == VRefChannel)
	{
		if (ReferenceMode::External)
		{
			Serial.println("Can not use external reference with VRef channel on AD799X.");
			return VRefChannel-1;
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
		case referenceMode::External:
			bitWrite(ConfigByte,3,1);
			break;
		case referenceMode::Supply:
		case referenceMode::Unknown:
		case default:
			break;
	}
	switch(I2CFilterMode)
	{
		case i2CFilterMode::Off:
			bitWrite(ConfigByte,2,1);
			break;
		case i2CFilterMode::On:
		case i2CFilterMode::Unknown:
		case default:
			break;
	}
	switch(SampleDelayMode)
	{
		case sampleDelayMode::Off:
			bitWrite(ConfigByte,1,1);
			bitWrite(ConfigByte,0,1);
			break;
		case sampleDelayMode::On:
		case sampleDelayMode::Unknown:
		case default:
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
    int BytesToCollect = Wire.available();
    int CurrentIndex = 0;
    while (Wire.available())
    {
	    MSBByte = Wire.readByte();
	    LSBByte = Wire.readByte();
	    //uint8_t CurrentChannel = MSBByte >> 4;
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
  	float MaxValue = (float)((((uint16_t)-1)>>4)>>LowByteShift);
    Wire.requestFrom(Address, 2*NumberOfSamples, I2C_STOP, I2CTimeout*2*NumberOfSamples);
    int BytesToCollect = Wire.available();
    int CurrentIndex = 0;
    while (Wire.available())
    {
	    MSBByte = Wire.readByte();
	    LSBByte = Wire.readByte();
	    //uint8_t CurrentChannel = MSBByte >> 4;
	    uint16_t CurrentValue = combine( ((MSBByte << 4)>>4), LSBByte ) >> LowByteShift;
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