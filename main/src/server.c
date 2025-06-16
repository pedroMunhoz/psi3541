#include "server.h"
#include "project.h" // Para wifi_debug_printf

static const char *TAG = "server"; // TAG for debug

static char *create_json(const char *key, ...) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    va_list args;
    va_start(args, key);

    const char *current_key = key;
    while (current_key != NULL) {
        json_key_type_t type = (json_key_type_t)va_arg(args, int);

        if (type == JSON_TYPE_INT) {
            int value = va_arg(args, int);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == JSON_TYPE_FLOAT) {
            double value = va_arg(args, double);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == JSON_TYPE_STRING) {
            const char *value = va_arg(args, const char *);
            cJSON_AddStringToObject(json, current_key, value);
        } else if (type == JSON_TYPE_BOOL) {
            int value = va_arg(args, int);
            cJSON_AddBoolToObject(json, current_key, value);
        } else {
            cJSON_Delete(json);
            va_end(args);
            return NULL;
        }

        current_key = va_arg(args, const char *);
    }

    va_end(args);

    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return response;
}

static esp_err_t read_request_body(httpd_req_t *req, char *buffer, size_t max_len) {
    int ret = httpd_req_recv(req, buffer, max_len - 1);
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to read request body");
        return ESP_FAIL;
    }
    buffer[ret] = '\0';
    return ESP_OK;
}

static bool extract_data_from_json(const char *json_str, const char *key, json_key_type_t type, void *out_val, size_t out_len) {
    cJSON *json = cJSON_Parse(json_str);
    if (!json) return false;

    cJSON *item = cJSON_GetObjectItem(json, key);
    bool valid = false;

    switch (type) {
        case JSON_TYPE_INT:
            if (cJSON_IsNumber(item)) {
                *(int *)out_val = item->valueint;
                valid = true;
            }
            break;

        case JSON_TYPE_FLOAT:
            if (cJSON_IsNumber(item)) {
                *(float *)out_val = (float)item->valuedouble;
                valid = true;
            }
            break;

        case JSON_TYPE_DOUBLE:
            if (cJSON_IsNumber(item)) {
                *(double *)out_val = item->valuedouble;
                valid = true;
            }
            break;

        case JSON_TYPE_STRING:
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy((char *)out_val, item->valuestring, out_len - 1);
                ((char *)out_val)[out_len - 1] = '\0';
                valid = true;
            }
            break;

        case JSON_TYPE_BOOL:
            if (cJSON_IsBool(item)) {
                *(bool *)out_val = cJSON_IsTrue(item);
                valid = true;
            }
            break;
    }

    cJSON_Delete(json);
    return valid;
}

static esp_err_t send_json_response(httpd_req_t *req, char *json_str) {
    if (!json_str) return httpd_resp_send_500(req);

    httpd_resp_set_type(req, "application/json");
    esp_err_t res = httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    free(json_str);
    return res;
}

/*####################################################################*/

static esp_err_t static_file_handler(httpd_req_t *req) {
    const static_route_t *route = (const static_route_t *)req->user_ctx;

    struct stat file_stat;
    if (stat(route->filepath, &file_stat) != 0) {
        ESP_LOGE(TAG, "File %s not found", route->filepath);
        return httpd_resp_send_404(req);
    }

    FILE *file = fopen(route->filepath, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", route->filepath);
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_type(req, route->mime);

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/*####################################################################*/

static esp_err_t car_handler(httpd_req_t *req) {
    myServer *server = (myServer *)req->user_ctx;
    char buffer[MAX_REQ_LEN];
    int received_state, received_ref;
    bool valid_state = false, valid_ref = false;

    wifi_debug_printf("[car_handler] Nova requisição recebida\n");

    if (req->content_len > 0) {
        if (read_request_body(req, buffer, sizeof(buffer)) != ESP_OK) {
            wifi_debug_printf("[car_handler] Erro ao ler body\n");
            return httpd_resp_send_500(req);
        }

        wifi_debug_printf("[car_handler] Body recebido: %s\n", buffer);

        valid_state = extract_data_from_json(buffer, "state", JSON_TYPE_INT, &received_state, sizeof(int));
        valid_ref = extract_data_from_json(buffer, "ref", JSON_TYPE_INT, &received_ref, sizeof(int));

        if (!valid_state || !valid_ref) {
            wifi_debug_printf("[car_handler] Erro: 'state' ou 'ref' inválido ou ausente\n");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'state' or 'ref'");
        }
    } else {
        wifi_debug_printf("[car_handler] Erro: Body vazio\n");
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
    }

    wifi_debug_printf("[car_handler] state=%d, ref=%d\n", received_state, received_ref);

    Action action = {
        .state = received_state,
        .ref = received_ref
    };

    int result;
    esp_err_t err = messenger_send_with_response_generic(
        server->messenger,
        MESSAGE_CAR_MOVE,
        (void*)&action,
        &result,
        sizeof(int)
    );

    if (err != ESP_OK) {
        wifi_debug_printf("[car_handler] Erro ao enviar mensagem para o messenger\n");
        return httpd_resp_send_500(req);
    }

    wifi_debug_printf("[car_handler] Comando enviado com sucesso\n");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t car_config_handler(httpd_req_t *req) {
    myServer *server = (myServer *)req->user_ctx;
    char buffer[MAX_REQ_LEN];
    float Kp, Kp_total, Kd, Ki;
    bool valid_Kp, valid_Kp_total, valid_Kd, valid_Ki = false;

    wifi_debug_printf("[car_config_handler] Nova requisição recebida\n");

    if (req->content_len > 0) {
        if (read_request_body(req, buffer, sizeof(buffer)) != ESP_OK) {
            wifi_debug_printf("[car_config_handler] Erro ao ler body\n");
            return httpd_resp_send_500(req);
        }

        wifi_debug_printf("[car_config_handler] Body recebido: %s\n", buffer);

        valid_Kp = extract_data_from_json(buffer, "Kp", JSON_TYPE_FLOAT, &Kp, sizeof(float));
        valid_Kp_total = extract_data_from_json(buffer, "Kp_total", JSON_TYPE_FLOAT, &Kp_total, sizeof(float));
        valid_Kd = extract_data_from_json(buffer, "Kd", JSON_TYPE_FLOAT, &Kd, sizeof(float));
        valid_Ki = extract_data_from_json(buffer, "Ki", JSON_TYPE_FLOAT, &Ki, sizeof(float));

        if (!valid_Kd) Kd = KD_DIR;
        if (!valid_Kp) Kp = KP_DIR;
        if (!valid_Kp_total) Kp_total = KP_DIR_TOTAL;
        if (!valid_Ki) Ki = KI_DIR;

        if (!valid_Kd && !valid_Kp && !valid_Kp_total && !valid_Ki) {
            wifi_debug_printf("[car_config_handler] Erro: 'Kp', 'Kd' ou 'Kp_total' inválido ou ausente\n");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'Kp', 'Kd' or 'Kp_total'");
        }

        wifi_debug_printf("[car_config_handler] Kp=%f, Kp_total=%f, Kd=%f, Ki=%f\n", Kp, Kp_total, Kd, Ki);

        CarConfig config = {
            .Kd = Kd,
            .Kp = Kp,
            .Kp_total = Kp_total,
            .Ki = Ki
        };

        CarConfig resultConfig;
        bool ok = messenger_send_with_response_generic(server->messenger, 
                                                       MESSAGE_CAR_SET_CONFIG, 
                                                       (void*)&config,
                                                       &resultConfig,
                                                       sizeof(CarConfig)
                                                    );
        
        if (ok) {
            wifi_debug_printf("[car_config_handler] Configuração enviada com sucesso\n");
            char* json = create_json("Kp", JSON_TYPE_FLOAT, resultConfig.Kp,
                                     "Kp_total", JSON_TYPE_FLOAT, resultConfig.Kp_total,
                                     "Kd", JSON_TYPE_FLOAT, resultConfig.Kd,
                                     "Ki", JSON_TYPE_FLOAT, resultConfig.Ki,
                                     NULL);
            return send_json_response(req, json);
        } else {
            wifi_debug_printf("[car_config_handler] Erro ao enviar mensagem para o messenger\n");
            return httpd_resp_send_500(req);
        }
        
    } else {
        wifi_debug_printf("[car_config_handler] Requisição GET de configuração\n");
        CarConfig resultConfig;
        bool ok = messenger_send_with_response_generic(server->messenger, 
                                                       MESSAGE_CAR_GET_CONFIG, 
                                                       NULL,
                                                       &resultConfig,
                                                       sizeof(CarConfig)
                                                    );
        
        if (ok) {
            wifi_debug_printf("[car_config_handler] Configuração obtida com sucesso\n");
            char* json = create_json("Kp", JSON_TYPE_FLOAT, resultConfig.Kp,
                                     "Kp_total", JSON_TYPE_FLOAT, resultConfig.Kp_total,
                                     "Kd", JSON_TYPE_FLOAT, resultConfig.Kd,
                                     "Ki", JSON_TYPE_FLOAT, resultConfig.Ki,
                                     NULL);
            return send_json_response(req, json);
        } else {
            wifi_debug_printf("[car_config_handler] Erro ao obter configuração do messenger\n");
            return httpd_resp_send_500(req);
        }
    }
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t car_status_handler(httpd_req_t *req) {
    myServer *server = (myServer *)req->user_ctx;
    char buffer[MAX_REQ_LEN];

    wifi_debug_printf("[car_status_handler] Nova requisição recebida\n");

    CarStatus status;
    bool ok = messenger_send_with_response_generic(server->messenger, 
                                                   MESSAGE_CAR_GET_STATUS, 
                                                   NULL,
                                                   &status,
                                                   sizeof(CarStatus)
                                                );
    if (ok) {
        wifi_debug_printf("[car_status_handler] Status: state=%d, ref=%d, cur=%d, done=%d\n",
            status.state, status.ref, status.cur, status.done);
        char* json = create_json("state", JSON_TYPE_INT, status.state,
                                 "ref", JSON_TYPE_INT, status.ref,
                                 "cur", JSON_TYPE_INT, status.cur,
                                 "done", JSON_TYPE_BOOL, status.done,
                                 NULL);
        return send_json_response(req, json);
    } else {
        return httpd_resp_send_500(req);
    }
}

static esp_err_t mqtt_tasks_handler(httpd_req_t* req) {
    myServer* server = (myServer* ) req->user_ctx;

    bool status[MQTT_MAX_PUB_TASKS];

    bool ok = messenger_send_with_response_generic(
        server->messenger,
        MESSAGE_MQTT_STATUS,
        NULL,
        status,
        sizeof(status)
    );

    if (ok) {
        cJSON *json_obj = cJSON_CreateObject();
        if (!json_obj) return httpd_resp_send_500(req);

        cJSON_AddNumberToObject(json_obj, "num_tasks", MQTT_MAX_PUB_TASKS);
        for (int i = 0; i < MQTT_MAX_PUB_TASKS; ++i) {
            char key[8];
            snprintf(key, sizeof(key), "%d", i + 1);
            cJSON_AddBoolToObject(json_obj, key, status[i]);
        }
        char *json = cJSON_PrintUnformatted(json_obj);
        cJSON_Delete(json_obj);
        return send_json_response(req, json);
    } else {
        return httpd_resp_send_500(req);
    }
}

static esp_err_t mqtt_task_control_handler(httpd_req_t *req) {
    myServer *server = (myServer *)req->user_ctx;

    char buffer[MAX_REQ_LEN];
    int taskIndex;
    char action[8];
    bool data_valid = false;

    if (req->content_len > 0) {
        if (read_request_body(req, buffer, sizeof(buffer)) != ESP_OK) {
            return httpd_resp_send_500(req);
        }

        data_valid = extract_data_from_json(buffer, "idx", JSON_TYPE_INT, &taskIndex, sizeof(int));
        data_valid = data_valid && extract_data_from_json(buffer, "action", JSON_TYPE_STRING, action, sizeof(action));
        if (!data_valid) {
            ESP_LOGE(TAG, "Invalid or missing 'state' in JSON");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid or missing 'state'");
        }
    }

    messenger_message_type_t type;
    if (strcasecmp(action, "start") == 0) {
        type = MESSAGE_MQTT_START;
    } else if (strcasecmp(action, "stop") == 0) {
        type = MESSAGE_MQTT_STOP;
    } else {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid action");
    }

    int success = 0;
    bool ok = messenger_send_with_response_generic(
        server->messenger,
        type,
        &taskIndex,
        &success,
        sizeof(int)
    );

    if (ok) {
        char *json = create_json("success", JSON_TYPE_INT, success, NULL);
        return send_json_response(req, json);
    } else {
        return httpd_resp_send_500(req);
    }
}
/*####################################################################*/

static static_route_t static_routes[] = {
    { "/",           "/spiffs/index.html",      "text/html" },
    { "/style.css",  "/spiffs/style.css",       "text/css" },
    { "/main.js",    "/spiffs/main.js",         "application/javascript" }
};

static api_route_t api_routes[] = {
    { "/carMove",               HTTP_POST, car_handler },
    { "/carConfig",             HTTP_POST, car_config_handler},
    { "/carStatus",            HTTP_POST, car_status_handler},
    { "/mqtt_task_control",     HTTP_POST, mqtt_task_control_handler },
    { "/mqtt_tasks",            HTTP_POST, mqtt_tasks_handler }
};

/*####################################################################*/

void server_init(myServer* server) {
    ESP_LOGI(TAG, "Initializing HTTP server...");
    server->config = (httpd_config_t)HTTPD_DEFAULT_CONFIG();
    server->config.max_uri_handlers = 10;
    server->handle = NULL;

    if (httpd_start(&(server->handle), &(server->config)) == ESP_OK) {
        ESP_LOGI(TAG, "HTTP server started.");
        for (int i = 0; i < sizeof(static_routes) / sizeof(static_routes[0]); i++) {
            httpd_uri_t uri_config = {
                .uri = static_routes[i].uri,
                .method = HTTP_GET,
                .handler = static_file_handler,
                .user_ctx = (void *)&static_routes[i]
            };
            httpd_register_uri_handler(server->handle, &uri_config);
        }

        for (size_t i = 0; i < sizeof(api_routes) / sizeof(api_routes[0]); i++) {
            httpd_uri_t uri_config = {
                .uri = api_routes[i].uri,
                .method = api_routes[i].method,
                .handler = api_routes[i].handler,
                .user_ctx = (void *)server
            };
            httpd_register_uri_handler(server->handle, &uri_config);
        }
        ESP_LOGI(TAG, "HTTP server URI handlers registered.");
    }
}

void server_setMessenger(myServer* server, Messenger* messenger) {
    server->messenger = messenger;
}