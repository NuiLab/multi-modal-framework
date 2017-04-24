#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <functional>
#include <initializer_list>
#include <memory>

struct circuit {
	enum class events
	{
		on,
		off,
		broken,
		none
	};

	class volts
	{
	public:
		volts() = default;
		volts(double voltage) :
			value(voltage)
		{}

		double value;
	};

	class amps
	{
	public:
		amps() = default;
		amps(double current) :
			value(current)
		{}

		double value;
	};

	class state
	{
	public:
		state() :
			state(false)
		{}

		state(bool state) :
			is_on(state),
			is_intact(true)
		{}

		void break_circuit()
		{
			is_intact = false;
		}

		void turn_on()
		{
			is_on = true;
		}

		bool is_on;
		bool is_intact;
	};
};


#endif
