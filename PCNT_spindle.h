#pragma once
#include <Arduino.h>
#include "driver/pulse_cnt.h"


static pcnt_unit_handle_t    pcnt_unit_spndl = nullptr;
static pcnt_channel_handle_t pcnt_ch_0_spndl = nullptr;
static pcnt_channel_handle_t pcnt_ch_1_spndl = nullptr;

// One revolution in raw quadrature counts. Set at runtime in setup_spindle_pcnt().
// Hardware requires |limit| <= 32767, so max supported encoder is 8191 PPR.
static int PCNT_LIMIT_SPNDL = 2400; // default for 600 PPR; overwritten at setup
static volatile int32_t pcnt_accum_spndl = 0;

// Task handle registered by the threading task before it blocks waiting for the
// index pulse. Written by the task, read by the ISR. 32-bit writes are atomic on LX6.
static volatile TaskHandle_t spndl_index_task_handle = nullptr;

static bool IRAM_ATTR on_pcnt_watch_spndl(pcnt_unit_handle_t unit,
	const pcnt_watch_event_data_t* edata,
	void* user_ctx)
{
	const int wp = edata->watch_point_value;

	if (wp == +PCNT_LIMIT_SPNDL) {
		pcnt_accum_spndl += PCNT_LIMIT_SPNDL;
		pcnt_unit_clear_count(unit);
	}
	else if (wp == -PCNT_LIMIT_SPNDL) {
		pcnt_accum_spndl -= PCNT_LIMIT_SPNDL;
		pcnt_unit_clear_count(unit);
	}

	// Overflow = one full revolution = index pulse. Notify any waiting task.
	TaskHandle_t h = spndl_index_task_handle;
	if (h != nullptr) {
		BaseType_t hp = pdFALSE;
		vTaskNotifyGiveFromISR(h, &hp);
		portYIELD_FROM_ISR(hp);
	}

	return true;
}

static void setup_spindle_pcnt()
{
	// Set limit to exactly one revolution so the overflow ISR fires as the index pulse.
	PCNT_LIMIT_SPNDL = spindlePulsesPerRev * 4;

	// sub 32768 limits (avoid overflow complexity)
	pcnt_unit_config_t ucfg = {
		.low_limit = -PCNT_LIMIT_SPNDL,
		.high_limit = +PCNT_LIMIT_SPNDL,
	};
	ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &pcnt_unit_spndl));

	// 2) Create ONE channel: A is edge source, B is level (direction)
	pcnt_chan_config_t c0cfg =
	{
		.edge_gpio_num = SPNDL_ROT_A,
		.level_gpio_num = SPNDL_ROT_B,
	};
	ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_spndl, &c0cfg, &pcnt_ch_0_spndl));
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
		pcnt_ch_0_spndl,
		PCNT_CHANNEL_EDGE_ACTION_INCREASE, // rising edge
		PCNT_CHANNEL_EDGE_ACTION_DECREASE  // falling edge
	));
	pcnt_chan_config_t c1cfg =
	{
		.edge_gpio_num = SPNDL_ROT_B,
		.level_gpio_num = SPNDL_ROT_A,
	};
	ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_spndl, &c1cfg, &pcnt_ch_1_spndl));
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
		pcnt_ch_1_spndl,
		PCNT_CHANNEL_EDGE_ACTION_INCREASE, // rising edge
		PCNT_CHANNEL_EDGE_ACTION_DECREASE  // falling edge
	));

	ESP_ERROR_CHECK(pcnt_channel_set_level_action(
		pcnt_ch_0_spndl,
		PCNT_CHANNEL_LEVEL_ACTION_KEEP,    // when B is LOW
		PCNT_CHANNEL_LEVEL_ACTION_INVERSE  // when B is HIGH
	));
	ESP_ERROR_CHECK(pcnt_channel_set_level_action(
		pcnt_ch_1_spndl,
		PCNT_CHANNEL_LEVEL_ACTION_INVERSE, // when A is LOW
		PCNT_CHANNEL_LEVEL_ACTION_KEEP     // when A is HIGH
	));



	// Optional glitch filter (time-based, in ns)
	pcnt_glitch_filter_config_t fcfg =
	{
		.max_glitch_ns = 12000,
	};
	ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit_spndl, &fcfg));


	// Watch points at limits
	ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit_spndl, +PCNT_LIMIT_SPNDL));
	ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit_spndl, -PCNT_LIMIT_SPNDL));

	pcnt_event_callbacks_t cbs =
	{
		.on_reach = on_pcnt_watch_spndl,
	};
	ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit_spndl, &cbs, nullptr));


	ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit_spndl));
	ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_spndl));
	ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit_spndl));
}

static int32_t read_spindle()
{
	int count = 0;
	ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit_spndl, &count));
	count += pcnt_accum_spndl;

	return count / 4;
}
