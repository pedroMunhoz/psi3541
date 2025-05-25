#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "esp_system.h"
#include "spi_flash_mmap.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include <sys/stat.h>
#include "esp_vfs.h"
#include "esp_log.h"

typedef struct {
    esp_vfs_spiffs_conf_t conf;
    size_t total;
    size_t used;
} Filesystem;

void filesystem_start(Filesystem* fs);
size_t filesystem_getTotal(Filesystem* fs);
size_t filesystem_getUsed(Filesystem* fs);

#endif