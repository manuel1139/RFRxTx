

//file globals
char timeout = 0;
uint8_t edge_cnt = 0;

#define MAX_EDGE 90

void rx_raw(uint16_t bit_time) {
    static uint16_t edge_times[MAX_EDGE];

    if (edge_cnt < MAX_EDGE && timeout == 0) {
        edge_times[edge_cnt] = bit_time;
        edge_cnt++;
    } else {
        if (edge_cnt != 0 && timeout == 1) {
            edge_cnt = 0;
        }
        timeout = 0;
    }
}