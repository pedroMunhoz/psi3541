#include "server.h"

static const char *TAG = "server"; // TAG for debug

httpd_uri_t uri_list[] = {
    {
        .uri = "/",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/style.css",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/main.js",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/led",
        .method = HTTP_POST,
        .handler = led_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/dht11",
        .method = HTTP_POST,
        .handler = dht11_handler,
        .user_ctx = NULL
    }
};

static void start_spiffs(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

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

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS Partition Size: Total: %d, Used: %d", total, used);
    }
}

httpd_handle_t setup_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    // Start SPIFFS
    start_spiffs();

    if (httpd_start(&server, &config) == ESP_OK) {
        // Register all URI handlers from the list
        for (int i = 0; i < sizeof(uri_list) / sizeof(uri_list[0]); i++) {
            httpd_register_uri_handler(server, &uri_list[i]);
        }
    }

    return server;
}

static esp_err_t send_json_response(httpd_req_t *req, const char *key, int value) {
    char response[64];
    snprintf(response, sizeof(response), "{\"%s\": %d}", key, value);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

esp_err_t static_file_handler(httpd_req_t *req) {
    char filepath[128];
    if (strcmp(req->uri, "/") == 0) {
        strcpy(filepath, "/spiffs/index.html");
        httpd_resp_set_type(req, "text/html");
    } else if (strcmp(req->uri, "/style.css") == 0) {
        strcpy(filepath, "/spiffs/style.css");
        httpd_resp_set_type(req, "text/css");
    } else if (strcmp(req->uri, "/main.js") == 0) {
        strcpy(filepath, "/spiffs/main.js");
        httpd_resp_set_type(req, "application/javascript");
    } else {
        ESP_LOGE(TAG, "Unsupported URI: %s", req->uri);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    struct stat file_stat;
    if (stat(filepath, &file_stat) != 0)
    {
        ESP_LOGE(TAG, "File %s not found", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }
    fclose(file);

    httpd_resp_send_chunk(req, NULL, 0); // End response
    return ESP_OK;
}

esp_err_t led_handler(httpd_req_t *req) {
    char buffer[32];
    int received = -1;

    // Log the request URI
    ESP_LOGI(TAG, "Received request for URI: %s", req->uri);

    // Check if the request has a body
    int content_len = req->content_len;
    if (content_len > 0) {
        // Read the request body
        int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1); // Leave space for null-terminator
        if (ret <= 0) {
            ESP_LOGE(TAG, "Failed to read request body");
            return httpd_resp_send_500(req);
        }

        buffer[ret] = '\0'; // Null-terminate the buffer

        // Log the request body
        ESP_LOGI(TAG, "Request body: %s", buffer);

        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            ESP_LOGE(TAG, "Failed to parse JSON");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to parse JSON");
        }

        cJSON *state = cJSON_GetObjectItem(json, "state");
        if (!cJSON_IsNumber(state)) {
            ESP_LOGE(TAG, "Invalid state value");
            cJSON_Delete(json);
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid state value");
        }

        received = state->valueint;
        cJSON_Delete(json);

    } else {
        // Log that no body was provided
        ESP_LOGI(TAG, "No request body provided");
    }

    post_office_message_t message = {
        .type = MESSAGE_LED,
        .data = (received != -1) ? (int*)&received : NULL,
        .response_queue = xQueueCreate(1, sizeof(int))
    };

    post_office_send_message(&message);

    int led_state;
    if (xQueueReceive(message.response_queue, &led_state, portMAX_DELAY)) {
        ESP_LOGI(TAG, "Received LED state: %d", led_state);
        vQueueDelete(message.response_queue); // Clean up the response queue
        return send_json_response(req, "state", led_state);
    } else {
        ESP_LOGE(TAG, "Failed to receive LED state");
        vQueueDelete(message.response_queue); // Clean up the response queue
        return httpd_resp_send_500(req);
    }
}

esp_err_t dht11_handler(httpd_req_t *req) {
    // Create a JSON response with dummy temperature and humidity values
    const char *response = "{\"temperature\": 25, \"humidity\": 60}";

    // Set the response type to JSON
    httpd_resp_set_type(req, "application/json");

    // Send the response
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}