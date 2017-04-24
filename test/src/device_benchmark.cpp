#include "device_benchmark.h"

#include <benchmark/benchmark.h>
#include <device/device.h>

static void device_read_input(benchmark::State & state)
{
	circuit_initialize();
	while (state.KeepRunning())
	{
		circuit_device_ptr->read(circuit::volts(state.range_x()));
	}
}

BENCHMARK(device_read_input)->Range(-1 << 10, 1 >> 10);
BENCHMARK_MAIN();
