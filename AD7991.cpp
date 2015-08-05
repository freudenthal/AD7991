#include "Arduino.h"
#include "i2c_t3.h"
#define MaxChannels 4
#define InternalReference 2.5f
#define DefaultAddress 40
using namespace std;
enum class sampleDelayMode{Unknown, On, Off};
enum class i2CFilterMode{Unknown, On, Off};
enum class referenceMode{Unknown, Internal, External};
class AD7991
{
	public:
		AD7991();
		~AD7991();
		bool isConnected();
		uint8_t getAddress();
		void setAddress(uint8_t address);
		float getVoltage(int Channel);
		float* getVoltage(int* Channels, size_t NumberOfChannels);
		int getVoltageInt(int Channel);
		int* getVoltageInt(int* Channels, size_t NumberOfChannels);
		bool setPower(int Channel, bool Active);
		bool getPower(int Channel);
		bool setI2CFilter(i2CFilterMode ModeSetting);
		i2CFilterMode getI2CFilter();
		bool setSampleDelayMode(sampleDelayMode ModeSetting);
		sampleDelayMode getSampleDelayMode();
		bool setReference(referenceMode ModeSetting);
		referenceMode getReference();
		void setVRefExt(float VRef);
		float getVRefExt(); 
	private:
		uint8_t Address;
		bool Power[MaxChannels];
		float Voltage[MaxChannels];
		int VoltageInt[MaxChannels];
		referenceMode ReferenceMode;
		i2CFilterMode I2CFilterMode;
		sampleDelayMode SampleDelayMode;
		uint8_t CommandByte;
		uint8_t MSBByte;
		uint8_t LSBByte;
		const float VRefInt = InternalReference;
		float VRefExt;
};
AD7991::AD7991()
{
	I2CFilterMode = i2CFilterMode::Unknown;
	ReferenceMode = referenceMode::Unknown;
	SampleDelayMode = sampleDelayMode::Unknown;
	Address = DefaultAddress;
	for (int Index = 0; Index < NumberOfChannels; Index++)
	{
		Power[Index] = true;
		Voltage[Index] = 0.0f;
	}
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
float* AD7991::getVoltage(int* Channels, size_t NumberOfChannels)
{
	
}
float AD7991::getVoltage(int Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels);
}
bool AD7991::setPower(int Channel, bool Active)
{
	Channel = constrain(Channel,0,NumberOfChannels);
}
bool AD7991::getPower(int Channel)
{
	Channel = constrain(Channel,0,NumberOfChannels);
	return Power[Channel];
}
bool AD7991::setI2CFilter(i2CFilterMode ModeSetting)
{

}
i2CFilterMode AD7991::getI2CFilter()
{

}
bool AD7991::setSampleDelayMode(sampleDelayMode ModeSetting)
{

}
sampleDelayMode AD7991::getSampleDelayMode()
{

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
void AD7991::ResetCommandByte()
{
	CommandByte = 0;
}
void AD7991::SetCommandByteAddress(uint8_t DACAddress)
{
	CommandByte = DACAddress;
}
void AD7991::SetCommandByteCommand(commandMode Command)
{
	CommandByte = ( (uint8_t)Command ) << 3;
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
	Wire.write(CommandByte);
	Wire.write(MSBByte);
	Wire.write(LSBByte);
	SendSuccess = Wire.endTransmission(I2C_STOP,I2CTimeout);
    if(SendSuccess != 0)
    {
      Wire.getError();
      Wire.finish();
      Wire.resetBus();
      CurrentAttempt++;
      if (CurrentAttempt > MaxAttempts)
      {
        MoveOn = true;
        Serial.println("Unrecoverable I2C transmission error.");
      }
    }
    else
    {
      MoveOn = true;
    }
  }
}
void AD7991::RecieveI2C(byte Address, int NumberOfBytes)
{
  if(NumberOfBytes > I2CRecieveBufferSize)
  {
    NumberOfBytes = I2CRecieveBufferSize;
  }
  if(NumberOfBytes > 0)
  {
    digitalWrite(LEDPin,HIGH);
    //Wire.beginTransmission(Address);
    //Wire.endTransmission(I2C_NOSTOP,I2CTimeout);
    Wire.requestFrom(Address, NumberOfBytes, I2C_STOP,I2CTimeout);
    int BytesToCollect = Wire.available();
    for (int RecieveByteNumber=0; RecieveByteNumber < BytesToCollect; RecieveByteNumber++)
    {
      if(RecieveByteNumber<I2CRecieveBufferSize)
      {
        I2CRecieveBuffer[RecieveByteNumber] = Wire.readByte();
      }
      else
      {
        Wire.readByte();
      }
    }
    digitalWrite(LEDPin,LOW);
  }
}