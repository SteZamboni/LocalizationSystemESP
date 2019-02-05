#include <esp_log.h>
#include <string>
#include <vector>
#include <string.h>
#include "sdkconfig.h"
#include "stdlib.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <freertos/task.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <sys/time.h>
#include <string.h>
#include "SensorData.h"
#include <iostream>
#include <sstream>

//Timer includes
#include "esp_types.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"


//Client includes
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


/*Set the SSID, password and channel via "make menuconfig"*/
#define ID_BOARD CONFIG_ID_BOARD
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PWD CONFIG_WIFI_PASSWORD
#define SERVER_PORT CONFIG_SERVER_PORT
#define SERVER_ADDR CONFIG_SERVER_ADDR

#define	LED_GPIO_PIN			GPIO_NUM_4

#define DIM_SSID 32
#define DIM_SEQ 4
#define DIM_ADDR 17

#define DIM_SR 3


//TIMER
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
// #define TIMER_INTERVAL0_SEC   (3.4179) // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC   (60)   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

//SCAN AND SNIFFER
#if CONFIG_WIFI_ALL_CHANNEL_SCAN
#define DEFAULT_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#elif CONFIG_WIFI_FAST_SCAN
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#else
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#endif /*CONFIG_SCAN_METHOD*/

#if CONFIG_WIFI_CONNECT_AP_BY_SIGNAL
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_WIFI_CONNECT_AP_BY_SECURITY
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#else
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#endif /*CONFIG_SORT_METHOD*/

#if CONFIG_FAST_SCAN_THRESHOLD
#define DEFAULT_RSSI CONFIG_FAST_SCAN_MINIMUM_SIGNAL
#if CONFIG_EXAMPLE_OPEN
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#elif CONFIG_EXAMPLE_WEP
#define DEFAULT_AUTHMODE WIFI_AUTH_WEP
#elif CONFIG_EXAMPLE_WPA
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA_PSK
#elif CONFIG_EXAMPLE_WPA2
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA2_PSK
#else
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif
#else
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif /*CONFIG_FAST_SCAN_THRESHOLD*/

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

xQueueHandle timer_queue;

typedef struct {
	unsigned version:2;
	unsigned type:2;
	unsigned subtype:4;
	unsigned ToDSFromDS:2;
	unsigned MoreFrag:1;
	unsigned Retry:1;
	unsigned otherFlags:4;
	unsigned duration_id:16;
	uint8_t destination[6]; /* receiver address */
	uint8_t source[6]; /* sender address */
	uint8_t BSSID[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	//uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

#ifdef __cplusplus
extern "C" {
#endif

//Functions scan and sniffer
static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer();
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_set_channel(uint8_t channel);

//Functions timer
static void IRAM_ATTR timer_group0_isr(void *para);
static void example_tg0_timer_init(timer_idx_t timer_idx, bool auto_reload, double timer_interval_sec);
static void timer_example_evt_task(void *arg);

//Client functions
static int SendAuthentication();
static void wifi_client();
static int CheckResponse();
static void SendData();

//Time sync functions
static int GetStarted();
void TimeParser(char *buffer);
vector<string> split(string str, string sep);

static const char *WIFI = "wifi";
static const char *CLIENT = "client";

vector<SensorData> ProbeVector;
int s;  //socket s
struct timeval time_st;

using namespace std;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(WIFI, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
        	ESP_LOGI(WIFI, "CONNECTION SUCCESSFUL");
        	break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(WIFI, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(WIFI, "Got IP: %s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            //Socket connection with PC-collector
            wifi_client();
            // Set starting time
			GetStarted();
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(WIFI, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;

        default:
            break;
    }
    return ESP_OK;
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = { };
    strcpy((char *)wifi_config.sta.ssid, (const char*)WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, (const char*)WIFI_PWD);
    wifi_config.sta.scan_method = DEFAULT_SCAN_METHOD;
    wifi_config.sta.sort_method = DEFAULT_SORT_METHOD;
	wifi_config.sta.threshold.rssi = DEFAULT_RSSI;
	wifi_config.sta.threshold.authmode = DEFAULT_AUTHMODE;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

	// Initialize Timer
	timer_queue = xQueueCreate(10, sizeof(timer_event_t));
	// example_tg0_timer_init(TIMER_0, TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC);
	example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD, TIMER_INTERVAL1_SEC);
	xTaskCreate(timer_example_evt_task, "timer_evt_task", 4096, NULL, 5, NULL);

	// Set WiFi scanner & handler
    wifi_scan();
	wifi_sniffer();
}

void wifi_sniffer(){
	//uint8_t level = 0;
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
	char SSID[DIM_SSID+1]="";   //+1 to include the '\0'
	char source[DIM_ADDR+1]="";
	char seq[DIM_SEQ+1]="";

	struct timeval time;

	if (type != WIFI_PKT_MGMT)
		return;

	if(gettimeofday(&time,NULL)==-1){
		//err-cannot retrieve information about time
	}
	time.tv_usec += time_st.tv_usec;
	if(time.tv_usec > 1000000){
		time.tv_usec -= 1000000;
		time.tv_sec ++;
	}
	time.tv_sec += time_st.tv_sec;

	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

	if((hdr->type==0) && (hdr->subtype==4) && (hdr->ToDSFromDS!=3) && (hdr->Retry==0)){  //filter packet probe request, retry and packet without IPv4

		if(ipkt->payload[0]==0){  //direct probe request
			int len=ipkt->payload[1];  //payload[1] contains the number of bytes used to store SSID
			memcpy(SSID,ipkt->payload+2,len);
			SSID[len]='\0';
		}

		sprintf(source,"%02x:%02x:%02x:%02x:%02x:%02x",hdr->source[0],hdr->source[1],hdr->source[2],hdr->source[3],hdr->source[4],hdr->source[5]);

		sprintf(seq,"%04x",hdr->sequence_ctrl);

		SensorData SD(ppkt->rx_ctrl.channel, ppkt->rx_ctrl.rssi,time, source, seq, SSID);

		//SD.printData();

		ProbeVector.insert(ProbeVector.end(),SD);

		//cout << ProbeVector.back().serialize();
	}
}

void wifi_sniffer_set_channel(uint8_t channel){
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
static void IRAM_ATTR timer_group0_isr(void *para){
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value =
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32 | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
       if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
        evt.type = TEST_WITH_RELOAD;
        TIMERG0.int_clr_timers.t1 = 1;
    } else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
static void example_tg0_timer_init(timer_idx_t timer_idx, bool auto_reload, double timer_interval_sec){
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

/*
 * The main task of this example program
 */
static void timer_example_evt_task(void *arg){
    while (1) {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

       printf("Group[%d], timer[%d] alarm event: Test\n", evt.timer_group, evt.timer_idx);

        /* Print the timer values passed by event */
        // printf("------- EVENT TIME —------\n");
        // print_timer_counter(evt.timer_counter_value);

        /* Print the timer values as visible by this task */
        printf("\n\n------— TASK TIME —------\n");
        uint64_t task_counter_value;
        timer_get_counter_value((timer_group_t)evt.timer_group, (timer_idx_t)evt.timer_idx, &task_counter_value);
        printf("Allarme passato - tempo passato = %d Secondi\n", TIMER_INTERVAL1_SEC);


        for(SensorData s : ProbeVector)
        	s.printData();

        cout << "Send data...\n";

        SendData();

    }
}

static void wifi_client(){
	struct sockaddr_in servaddr;
	short port;
	int result;

	//Socket creation
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s < 0) {
        ESP_LOGE(CLIENT, "Failed to allocate socket.\n");
        return;
    }

	port = atoi (SERVER_PORT);

	memset(&servaddr, 0, sizeof(servaddr));
	//Specify information about server
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if(inet_aton(SERVER_ADDR, &servaddr.sin_addr)==0){
		ESP_LOGE(CLIENT, "Failed to convert the address.\n");
        return;
	}
	ESP_LOGI(CLIENT, "SOCKET CREATED");

	//Connection with the server
	if(connect(s, (struct sockaddr *) &servaddr, sizeof(servaddr))!=0){
		ESP_LOGE(CLIENT, "Socket connect failed!\n");
		close(s);
		return;
	};

	ESP_LOGI(CLIENT, "CONNECTION ESTABLISHED");

	if (SendAuthentication() == 0) {
		ESP_LOGI(CLIENT, "WAITING RESPONSE ...");
	}
	else{
		cout << "Socket will be closed!\n";
		close(s);
	}

	result=CheckResponse();
	if(result==0){
		ESP_LOGI(CLIENT,"Authentication closed with OK response from server");
	}
	else{
		ESP_LOGE(CLIENT,"Socket will be closed!");
		close(s);
	}
	return;
}

static int SendAuthentication(){
	string buf(ID_BOARD);

    if( write(s , buf.c_str() , buf.size()) < 0)
    {
        ESP_LOGE(CLIENT, "SEND FAILED\n");
        close(s);
        return -1;
    }
    else
    	ESP_LOGI(CLIENT, "AUTHENTICATION SENT CORRECTLY");

   return 0;
}

static int CheckResponse(){
	char response[DIM_SR+1];

    if( read(s , response , DIM_SR) < 0)
    {
        ESP_LOGE(CLIENT, "READ RESPONSE FAILED\n");
        close(s);
        return -1;
    }
    else{
    	if(response[0]=='O' && response[1]=='K'){
    		cout << "Reply from server is an OK message\n";
    		return 0;
    	}
    	else{
    		cout << "Reply from server is an ERR message\n";
    		return -1;
    	}
    }

}

static void SendData(){
	ostringstream data, temp;
	string buffer, dim;
	int result, nbytes;

	while (!ProbeVector.empty()){
	    data << ProbeVector.back().serialize();
	    ProbeVector.pop_back();
	}

	dim = data.str();

	nbytes = dim.size();
	//Set the flag
	if(nbytes != 0){
		//There are probe requests are catch in this 60 seconds

		temp << nbytes;

		dim = temp.str();

		cout << "Size(Byte): " << dim << endl;
		//Send buffer dimension
		if( write(s , dim.c_str() , dim.size()) < 0)
		{
			ESP_LOGE(CLIENT, "SEND DIMENSION FAILED\n");
			close(s);
			return;
		}
		else
			ESP_LOGI(CLIENT, "DIMENSION SENT CORRECTLY");

		result = CheckResponse();
		if(result == 0){
			buffer = data.str();

			if( write(s , buffer.c_str() , buffer.size()) < 0){

				ESP_LOGE(CLIENT, "SEND BUFFER FAILED\n");
				close(s);
				return;
			}
			else
				ESP_LOGI(CLIENT, "BUFFER SENT CORRECTLY");
			return;
		}
		else
			return;
	}
	return;
}

static int GetStarted() {
	char response[DIM_SR + 1];

	if (recv(s, response, DIM_SR, 0) < 0)
	{
		ESP_LOGE(CLIENT, "READ RESPONSE FAILED TIME\n");
		close(s);
		return -1;
	}
	else {
		ESP_LOGI(CLIENT, "RESPONSE RECEIVED TIME");
		printf("%s", response);
		TimeParser(response);
		return 0;
	}
}

void TimeParser(char *buffer) {
	vector<string> time_fields;
	string buf(buffer), sep_fields("\t");

	time_fields = split(buf, sep_fields);

	int sec = atoi(time_fields[0].c_str());
	int usec = atoi(time_fields[1].c_str());

	// struct timeval time_st;
	time_st.tv_sec = sec;
	time_st.tv_usec = usec;

	// settimeofday(&time_st, NULL);
}

vector<string> split(string str, string sep) {
	char* cstr = const_cast<char*>(str.c_str());
	char* current;
	char *nextToken = NULL;
	vector<string> arr;
	current = strtok(cstr, sep.c_str());
	while (current != NULL) {
		arr.push_back(current);
		current = strtok(NULL, sep.c_str());
	}
	return arr;
}

#ifdef __cplusplus
}
#endif
