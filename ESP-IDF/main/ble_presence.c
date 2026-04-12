#include "ble_presence.h"

#include <stdio.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "config.h"
#include "data_logger.h"

static const char *TAG = "ble_presence";

typedef struct {
    char name[32];
    char addr[18];
} ble_whitelist_entry_t;

static const ble_whitelist_entry_t s_whitelist[] = {
    {.name = BLE_PHONE_1_NAME, .addr = BLE_PHONE_1_ADDR},
    {.name = BLE_PHONE_2_NAME, .addr = BLE_PHONE_2_ADDR},
};

static app_state_t *s_state;
static esp_timer_handle_t s_scan_start_timer;
static esp_timer_handle_t s_scan_stop_timer;

static void copy_str(char *dst, size_t dst_len, const char *src)
{
    if (!dst || dst_len == 0) {
        return;
    }
    snprintf(dst, dst_len, "%s", src ? src : "");
}

static void bda_to_string(const esp_bd_addr_t bda, char *out, size_t out_len)
{
    snprintf(out, out_len, "%02X:%02X:%02X:%02X:%02X:%02X", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
}

static const ble_whitelist_entry_t *find_whitelisted(const char *addr)
{
    for (size_t i = 0; i < sizeof(s_whitelist) / sizeof(s_whitelist[0]); ++i) {
        if (strcasecmp(s_whitelist[i].addr, addr) == 0) {
            return &s_whitelist[i];
        }
    }
    return NULL;
}

static void scan_stop_cb(void *arg)
{
    (void)arg;
    esp_ble_gap_stop_scanning();
}

static void scan_start_cb(void *arg)
{
    (void)arg;
    if (!s_state) {
        return;
    }

    app_state_lock(s_state);
    bool enabled = s_state->config.ble_enable;
    uint32_t burst_ms = s_state->config.ble_scan_burst_ms;
    app_state_unlock(s_state);

    if (!enabled) {
        return;
    }

    esp_ble_gap_start_scanning(burst_ms / 1000U);
    esp_timer_start_once(s_scan_stop_timer, burst_ms * 1000ULL);
}

static void restart_idle_timer(uint32_t idle_ms)
{
    esp_timer_stop(s_scan_start_timer);
    esp_timer_start_once(s_scan_start_timer, idle_ms * 1000ULL);
}

static void update_presence_from_scan_end(void)
{
    int64_t now = app_now_us();

    app_state_lock(s_state);
    bool seen_recently = (s_state->ble.last_seen_us > 0) &&
                         ((now - s_state->ble.last_seen_us) <= ((int64_t)s_state->config.presence_hold_time_s * 1000000LL));
    if (!seen_recently) {
        if (s_state->ble.debounce_counter > 0) {
            s_state->ble.debounce_counter--;
        }
        if (s_state->ble.debounce_counter == 0 && s_state->ble.stable_present) {
            s_state->ble.stable_present = false;
            s_state->ble.present = false;
            s_state->ble.last_lost_us = now;
            s_state->ble.strongest_rssi = -127;
            copy_str(s_state->ble.strongest_name, sizeof(s_state->ble.strongest_name), "");
            copy_str(s_state->ble.strongest_addr, sizeof(s_state->ble.strongest_addr), "");
            xEventGroupSetBits(s_state->events, APP_EVENT_PRESENCE_UPDATED);
            app_state_unlock(s_state);
            data_logger_log_event(s_state, "ble_presence", "Whitelisted BLE presence lost");
            return;
        }
    }
    app_state_unlock(s_state);
}

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        scan_start_cb(NULL);
        break;
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            app_state_lock(s_state);
            s_state->ble.scanning = true;
            s_state->ble.last_scan_start_us = app_now_us();
            app_state_unlock(s_state);
        }
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
            char addr[18];
            bda_to_string(param->scan_rst.bda, addr, sizeof(addr));
            const ble_whitelist_entry_t *entry = find_whitelisted(addr);
            if (entry) {
                app_state_lock(s_state);
                if (param->scan_rst.rssi > s_state->ble.strongest_rssi || !s_state->ble.stable_present) {
                    s_state->ble.strongest_rssi = param->scan_rst.rssi;
                    copy_str(s_state->ble.strongest_name, sizeof(s_state->ble.strongest_name), entry->name);
                    copy_str(s_state->ble.strongest_addr, sizeof(s_state->ble.strongest_addr), entry->addr);
                }
                s_state->ble.present = true;
                s_state->ble.last_seen_us = app_now_us();
                if (s_state->ble.debounce_counter < s_state->config.ble_presence_debounce_count) {
                    s_state->ble.debounce_counter++;
                }
                if (s_state->ble.debounce_counter >= s_state->config.ble_presence_debounce_count &&
                    !s_state->ble.stable_present) {
                    s_state->ble.stable_present = true;
                    s_state->ble.detections_today++;
                    xEventGroupSetBits(s_state->events, APP_EVENT_PRESENCE_UPDATED);
                    app_state_unlock(s_state);
                    data_logger_log_event(s_state, "ble_presence", "Whitelisted BLE device detected");
                    return;
                }
                app_state_unlock(s_state);
            }
        } else if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
            app_state_lock(s_state);
            s_state->ble.scanning = false;
            uint32_t idle_ms = s_state->config.ble_scan_idle_ms;
            app_state_unlock(s_state);
            update_presence_from_scan_end();
            restart_idle_timer(idle_ms);
        }
        break;
    default:
        break;
    }
}

esp_err_t ble_presence_init(app_state_t *state)
{
    s_state = state;

    app_state_lock(state);
    state->ble.strongest_rssi = -127;
    app_state_unlock(state);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }
    err = esp_bt_controller_init(&bt_cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }
    err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }
    err = esp_bluedroid_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }
    err = esp_bluedroid_enable();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    ESP_RETURN_ON_ERROR(esp_ble_gap_register_callback(gap_cb), TAG, "gap register failed");

    const esp_timer_create_args_t start_args = {
        .callback = scan_start_cb,
        .name = "ble_scan_start",
    };
    const esp_timer_create_args_t stop_args = {
        .callback = scan_stop_cb,
        .name = "ble_scan_stop",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&start_args, &s_scan_start_timer), TAG, "start timer failed");
    ESP_RETURN_ON_ERROR(esp_timer_create(&stop_args, &s_scan_stop_timer), TAG, "stop timer failed");
    return ESP_OK;
}

esp_err_t ble_presence_start(app_state_t *state)
{
    s_state = state;
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = (uint16_t)(state->config.ble_scan_interval_ms / 0.625f),
        .scan_window = (uint16_t)(state->config.ble_scan_window_ms / 0.625f),
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
    };
    return esp_ble_gap_set_scan_params(&scan_params);
}

void ble_presence_stop(void)
{
    if (s_scan_start_timer) {
        esp_timer_stop(s_scan_start_timer);
    }
    if (s_scan_stop_timer) {
        esp_timer_stop(s_scan_stop_timer);
    }
    esp_ble_gap_stop_scanning();
}
