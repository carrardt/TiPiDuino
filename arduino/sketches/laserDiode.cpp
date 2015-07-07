#include <AvrTLPin.h>
#include <AvrTLSignal.h>
//#include <HWSerialIO.h>

using namespace avrtl;

constexpr auto laserPin = 12;
constexpr auto switchPin = 2;
constexpr auto pwmHighUSecs = 10;
constexpr auto pwmCycleUSecs = 128;

constexpr uint16_t pwmCycleTicks = microsecondsToTicks(pwmCycleUSecs);
constexpr uint16_t pwmHighTicks = microsecondsToTicks(pwmHighUSecs);

auto laserOutput = StaticPin<laserPin>();
auto switchInput = StaticPin<switchPin>();

void setup()
{
	laserOutput.SetOutput();
	switchInput.SetInput();
}

void loop()
{
	loopPWM<pwmCycleTicks>(
		laserOutput,
		[]()->uint16_t{ return switchInput.Get() ? pwmHighTicks : 0 ; }
		);
}
