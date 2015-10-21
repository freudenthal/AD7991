/*
AD7991.h Library for reading data from AD7991 12-bit ADC
Last update 9/1/2015
John Freudenthal and Sean Kirkpatrick
*/

#ifndef AD7991_h	//check for multiple inclusions
#define AD7991_h

#include "Arduino.h"
#include "i2c_t3.h"

#define MaxChannels 4
#define InternalReference 2.5f
#define DefaultAddress 0x28
#define VoltageReferenceChannel 3
#define I2CTimeout 1000
#define LowByteShift 0	//LowByteShift changes across the AD799X family.
	// For AD7991, 0 shift. For AD7995, 2 shift. For AD7999, 4 shift.
enum class sampleDelayMode{Unknown, On, Off};
enum class i2CFilterMode{Unknown, On, Off};
enum class AD7991ReferenceMode{Unknown, Supply, External};

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
		bool setReference(AD7991ReferenceMode ModeSetting);
		AD7991ReferenceMode getReference();
		void setVRefExt(float VRef);
		float getVRefExt();
		float getVRef();

	private:
		uint8_t Address;
		AD7991ReferenceMode ReferenceMode;
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
		void UpdateChannelSingle(uint8_t Channel);
		uint8_t UpdateChannelDACsActive(uint8_t NewDACsActive);
		uint8_t CheckChannel(uint8_t Channel);
		void UpdateConfigByte();
		void SetConfigByte();
		void SendI2C();
		size_t RecieveI2CInt(uint16_t* Data, size_t NumberOfSamples);
		size_t RecieveI2CFloat(float* Data, size_t NumberOfSamples);
};

#endif