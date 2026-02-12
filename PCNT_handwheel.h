#include <Arduino.h>
#include "driver/pulse_cnt.h"

// Handwheel-friendly limits (avoid overflow complexity)
static constexpr int LOW_LIM = -30000;
static constexpr int HIGH_LIM = 30000;


static pcnt_unit_handle_t    pcnt_unit_hdwl = nullptr;
static pcnt_channel_handle_t pcnt_ch_0_hdwl = nullptr;
static pcnt_channel_handle_t pcnt_ch_1_hdwl = nullptr;


static void setup_handwheel_pcnt()
{
    // 1) Create unit
    pcnt_unit_config_t ucfg = {
        .low_limit = LOW_LIM,
        .high_limit = HIGH_LIM,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &pcnt_unit_hdwl));

    // 2) Create ONE channel: A is edge source, B is level (direction)
    pcnt_chan_config_t c0cfg = 
    {
        .edge_gpio_num = RTR0_A,
        .level_gpio_num = RTR0_B,       
    };
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_hdwl, &c0cfg, &pcnt_ch_0_hdwl));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        pcnt_ch_0_hdwl,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, // rising edge
        PCNT_CHANNEL_EDGE_ACTION_DECREASE  // falling edge
    ));
    pcnt_chan_config_t c1cfg =
    {
        .edge_gpio_num = RTR0_B,
        .level_gpio_num = RTR0_A,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_hdwl, &c1cfg, &pcnt_ch_1_hdwl));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        pcnt_ch_1_hdwl,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, // rising edge
        PCNT_CHANNEL_EDGE_ACTION_DECREASE  // falling edge
    ));    

    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        pcnt_ch_0_hdwl,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,    // when B is LOW
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE  // when B is HIGH
    ));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        pcnt_ch_1_hdwl,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE, // when A is LOW
        PCNT_CHANNEL_LEVEL_ACTION_KEEP     // when A is HIGH
    ));



    // Optional glitch filter (time-based, in ns)
    pcnt_glitch_filter_config_t fcfg = {
        .max_glitch_ns = 12000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit_hdwl, &fcfg));

    
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit_hdwl));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_hdwl));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit_hdwl));
}

static int32_t read_handwheel()
{
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit_hdwl, &count));
    return count / 4;
}
