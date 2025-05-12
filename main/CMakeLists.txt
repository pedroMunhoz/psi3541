file(GLOB srcs "src/*.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include")

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)