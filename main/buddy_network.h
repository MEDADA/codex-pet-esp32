#pragma once
#include <stdbool.h>
#include <stddef.h>
typedef struct { char ssid[33],password[65],url[160],token[129]; } buddy_network_config_t;
void buddy_network_init(void);
bool buddy_network_connected(void);
const buddy_network_config_t *buddy_network_config(void);
bool buddy_network_setup(char *hint,size_t size);
