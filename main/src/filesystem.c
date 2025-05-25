#include "filesystem.h"

static const char *TAG = "FileSystem";

void filesystem_start(Filesystem* fs) {
    fs->conf.base_path = "/spiffs";
    fs->conf.partition_label = NULL;
    fs->conf.max_files = 5;
    fs->conf.format_if_mount_failed = true;

    esp_err_t ret = esp_vfs_spiffs_register(&(fs->conf));

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    ret = esp_spiffs_info(NULL, &(fs->total), &(fs->used));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
}

size_t filesystem_getTotal(Filesystem* fs) {
    return fs->total;
}

size_t filesystem_getUsed(Filesystem* fs) {
    return fs->used;
}