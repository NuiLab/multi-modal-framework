#include <memory>
#include <initializer_list>

#include <device/device.h>
#include <device/event.h>

#include "circuit.h"

typedef tamgef::device::GenericDevice <
	circuit::volts,
	circuit::amps,
	circuit::state,
	circuit::events
	> circuit_device;

typedef tamgef::device::Event<circuit::events> circuit_device_event;

std::shared_ptr<circuit_device> circuit_device_ptr;

void circuit_initialize()
{
	circuit_device_event::registerType(
		std::initializer_list < circuit::events >
		({
		circuit::events::on,
		circuit::events::off,
		circuit::events::broken,
		circuit::events::none
	}));

	circuit_device_ptr = std::make_shared<circuit_device>(
	[](circuit::volts voltage)
	{
		const auto input_voltage_lower_limit = 0;
		return (voltage.value >= input_voltage_lower_limit);
	},

	[](circuit::amps current)
	{
		const auto output_current_upper_limit = 2;
		return (current.value <= output_current_upper_limit);
	},

	[](circuit::volts voltage)
	{
		auto switch_resistance = 100;
		return voltage.value / switch_resistance;
	},

	[]( circuit::state current_state,
		circuit::volts input_voltage,
		circuit::amps output_current)
	{
		circuit::state new_state(false); // initially off
		const auto switch_voltage_threshold = 2;
		const auto switch_current_limit = 1;

		// if broken, can't change state
		if (!current_state.is_intact)
			return current_state;

		if (input_voltage.value < switch_voltage_threshold)
			return new_state; // turn off

		if (output_current.value > switch_current_limit) {
			new_state.break_circuit(); // break ciruit
			return new_state;
		}

		// input and output are valid
		if (!current_state.is_on) {
			new_state.turn_on();
			return new_state;
		}

		// if on, stay on
		return current_state;
	},
	std::initializer_list < circuit_device::EventFunction >
	({
		[=](circuit::state current_state) -> circuit_device_event
		{
			if (!current_state.is_intact)
				return circuit_device_event(circuit::events::broken);

			return circuit::events::none;
		},
			[=](circuit::state current_state) -> circuit_device_event
		{
			if (current_state.is_on)
				return circuit_device_event(circuit::events::on);

			return circuit_device_event(circuit::events::off);
		}
	}));
}