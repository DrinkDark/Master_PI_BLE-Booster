#include "display.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display);


//uart device declaration
#define UART_DEVICE_NODE DT_ALIAS(displayuart)

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

//device reset value
bool device_1_reset = false;
bool device_2_reset = false;
bool device_3_reset = false;

//mutex to protect print
static struct k_mutex display_mutex;

void display_init()
{
        k_mutex_init(&display_mutex);
}


void nextion_command(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
        uart_poll_out(uart_dev,0xFF);
        uart_poll_out(uart_dev,0xFF);
        uart_poll_out(uart_dev,0xFF);
}

void display_main_page()
{
        k_mutex_lock(&display_mutex, K_FOREVER);
        nextion_command("page 1");
        k_mutex_unlock(&display_mutex);

        //reset display_device functions static variables
        display_device_1(-1,-1,-1,ST_INIT,false);
        display_device_1(-1,-1,-1,ST_INIT,false);
        display_device_1(-1,-1,-1,ST_INIT,false);
        k_msleep(100);
        hide_device_1();
        hide_device_2();
        hide_device_3();
}

void display_device_1(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected)
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        static int prev_deviceNbr = -1;
        static int prev_rssi = -1;
        static int prev_nbrDays = -1;
        static enum main_state prev_state = ST_INIT;
        static bool prev_selected = false;

        if (deviceNbr == prev_deviceNbr && rssi == prev_rssi && nbrDays == prev_nbrDays &&
                state == prev_state && selected == prev_selected && !device_1_reset) {
                // No change, return without sending Nextion commands
                k_mutex_unlock(&display_mutex);
                return;
        }

        // Update previous values
        prev_deviceNbr = deviceNbr;
        prev_rssi = rssi;
        prev_nbrDays = nbrDays;
        prev_state = state;
        prev_selected = selected;
        device_1_reset = false;

        if(selected)
        {
                nextion_command("m1_pic.pic=3");
                
                nextion_command("m1_title.bco=54938");
                nextion_command("m1_rssi.bco=54938");
                nextion_command("m1_rssi_val.bco=54938");
                nextion_command("m1_DS.bco=54938");
                nextion_command("m1_DS_val.bco=54938");
                nextion_command("m1_DR.bco=54938");
                nextion_command("m1_DR_val.bco=54938");
        }
        else
        {
                nextion_command("m1_pic.pic=2");

                nextion_command("m1_title.bco=65535");
                nextion_command("m1_rssi.bco=65535");
                nextion_command("m1_rssi_val.bco=65535");
                nextion_command("m1_DS.bco=65535");
                nextion_command("m1_DS_val.bco=65535");
                nextion_command("m1_DR.bco=65535");
                nextion_command("m1_DR_val.bco=65535");
        }

        char buf[50];

        sprintf(buf,"m1_title.txt=\"Device %d\"",deviceNbr);
        nextion_command(buf);

        sprintf(buf,"m1_rssi_val.txt=\"%d\"",rssi);
        nextion_command(buf);

        sprintf(buf,"m1_DR_val.txt=\"%d\"",nbrDays);
        nextion_command(buf);

        if(state==ST_INIT)
                nextion_command("m1_DS_val.txt=\"Initialization\"");
        else if(state==ST_WAIT_SD_CARD)
                nextion_command("m1_DS_val.txt=\"No SD Card\"");
        else if(state==ST_IDLE)
                nextion_command("m1_DS_val.txt=\"Idle\"");
        else if(state==ST_RECORDING)
                nextion_command("m1_DS_val.txt=\"Recording\"");
        else if(state==ST_DISK_FULL)
                nextion_command("m1_DS_val.txt=\"Disk Full\"");
        else if(state==ST_LOW_BATT)
                nextion_command("m1_DS_val.txt=\"Low Battery\"");
        else if(state==ST_POWER_SAVING)
                nextion_command("m1_DS_val.txt=\"Power Saving\"");
        else if(state==ST_ERROR)
                nextion_command("m1_DS_val.txt=\"Error\"");

        nextion_command("vis m1_pic,1");
        nextion_command("vis m1_title,1");
        nextion_command("vis m1_rssi,1");
        nextion_command("vis m1_rssi_val,1");
        nextion_command("vis m1_DS,1");
        nextion_command("vis m1_DS_val,1");
        nextion_command("vis m1_DR,1");
        nextion_command("vis m1_DR_val,1");

        k_msleep(20);

        k_mutex_unlock(&display_mutex);
}

void display_device_2(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected)
{
        k_mutex_lock(&display_mutex, K_FOREVER);
        
        static int prev_deviceNbr = -1;
        static int prev_rssi = -1;
        static int prev_nbrDays = -1;
        static enum main_state prev_state = ST_INIT;
        static bool prev_selected = false;

        if (deviceNbr == prev_deviceNbr && rssi == prev_rssi && nbrDays == prev_nbrDays &&
                state == prev_state && selected == prev_selected && !device_2_reset) {
                // No change, return without sending Nextion commands
                k_mutex_unlock(&display_mutex);
                return;
        }

        // Update previous values
        prev_deviceNbr = deviceNbr;
        prev_rssi = rssi;
        prev_nbrDays = nbrDays;
        prev_state = state;
        prev_selected = selected;
        device_2_reset = false;

        if(selected)
        {
                nextion_command("m2_pic.pic=3");
                
                nextion_command("m2_title.bco=54938");
                nextion_command("m2_rssi.bco=54938");
                nextion_command("m2_rssi_val.bco=54938");
                nextion_command("m2_DS.bco=54938");
                nextion_command("m2_DS_val.bco=54938");
                nextion_command("m2_DR.bco=54938");
                nextion_command("m2_DR_val.bco=54938");
        }
        else
        {
                nextion_command("m2_pic.pic=2");

                nextion_command("m2_title.bco=65535");
                nextion_command("m2_rssi.bco=65535");
                nextion_command("m2_rssi_val.bco=65535");
                nextion_command("m2_DS.bco=65535");
                nextion_command("m2_DS_val.bco=65535");
                nextion_command("m2_DR.bco=65535");
                nextion_command("m2_DR_val.bco=65535");
        }

        char buf[50];

        sprintf(buf,"m2_title.txt=\"Device %d\"",deviceNbr);
        nextion_command(buf);

        sprintf(buf,"m2_rssi_val.txt=\"%d\"",rssi);
        nextion_command(buf);

        sprintf(buf,"m2_DR_val.txt=\"%d\"",nbrDays);
        nextion_command(buf);

        if(state==ST_INIT)
                nextion_command("m2_DS_val.txt=\"Initialization\"");
        else if(state==ST_WAIT_SD_CARD)
                nextion_command("m2_DS_val.txt=\"No SD Card\"");
        else if(state==ST_IDLE)
                nextion_command("m2_DS_val.txt=\"Idle\"");
        else if(state==ST_RECORDING)
                nextion_command("m2_DS_val.txt=\"Recording\"");
        else if(state==ST_DISK_FULL)
                nextion_command("m2_DS_val.txt=\"Disk Full\"");
        else if(state==ST_LOW_BATT)
                nextion_command("m2_DS_val.txt=\"Low Battery\"");
        else if(state==ST_POWER_SAVING)
                nextion_command("m2_DS_val.txt=\"Power Saving\"");
        else if(state==ST_ERROR)
                nextion_command("m2_DS_val.txt=\"Error\"");

        nextion_command("vis m2_pic,1");
        nextion_command("vis m2_title,1");
        nextion_command("vis m2_rssi,1");
        nextion_command("vis m2_rssi_val,1");
        nextion_command("vis m2_DS,1");
        nextion_command("vis m2_DS_val,1");
        nextion_command("vis m2_DR,1");
        nextion_command("vis m2_DR_val,1");

        k_msleep(20);

        k_mutex_unlock(&display_mutex);
}

void display_device_3(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected)
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        static int prev_deviceNbr = -1;
        static int prev_rssi = -1;
        static int prev_nbrDays = -1;
        static enum main_state prev_state = ST_INIT;
        static bool prev_selected = false;

        if (deviceNbr == prev_deviceNbr && rssi == prev_rssi && nbrDays == prev_nbrDays &&
                state == prev_state && selected == prev_selected && !device_3_reset) {
                // No change, return without sending Nextion commands
                k_mutex_unlock(&display_mutex);
                return;
        }

        // Update previous values
        prev_deviceNbr = deviceNbr;
        prev_rssi = rssi;
        prev_nbrDays = nbrDays;
        prev_state = state;
        prev_selected = selected;
        device_3_reset = false;

        if(selected)
        {
                nextion_command("m3_pic.pic=3");
                
                nextion_command("m3_title.bco=54938");
                nextion_command("m3_rssi.bco=54938");
                nextion_command("m3_rssi_val.bco=54938");
                nextion_command("m3_DS.bco=54938");
                nextion_command("m3_DS_val.bco=54938");
                nextion_command("m3_DR.bco=54938");
                nextion_command("m3_DR_val.bco=54938");
        }
        else
        {
                nextion_command("m3_pic.pic=2");

                nextion_command("m3_title.bco=65535");
                nextion_command("m3_rssi.bco=65535");
                nextion_command("m3_rssi_val.bco=65535");
                nextion_command("m3_DS.bco=65535");
                nextion_command("m3_DS_val.bco=65535");
                nextion_command("m3_DR.bco=65535");
                nextion_command("m3_DR_val.bco=65535");
        }

        char buf[50];

        sprintf(buf,"m3_title.txt=\"Device %d\"",deviceNbr);
        nextion_command(buf);

        sprintf(buf,"m3_rssi_val.txt=\"%d\"",rssi);
        nextion_command(buf);

        sprintf(buf,"m3_DR_val.txt=\"%d\"",nbrDays);
        nextion_command(buf);

        if(state==ST_INIT)
                nextion_command("m3_DS_val.txt=\"Initialization\"");
        else if(state==ST_WAIT_SD_CARD)
                nextion_command("m3_DS_val.txt=\"No SD Card\"");
        else if(state==ST_IDLE)
                nextion_command("m3_DS_val.txt=\"Idle\"");
        else if(state==ST_RECORDING)
                nextion_command("m3_DS_val.txt=\"Recording\"");
        else if(state==ST_DISK_FULL)
                nextion_command("m3_DS_val.txt=\"Disk Full\"");
        else if(state==ST_LOW_BATT)
                nextion_command("m3_DS_val.txt=\"Low Battery\"");
        else if(state==ST_POWER_SAVING)
                nextion_command("m3_DS_val.txt=\"Power Saving\"");
        else if(state==ST_ERROR)
                nextion_command("m3_DS_val.txt=\"Error\"");

        nextion_command("vis m3_pic,1");
        nextion_command("vis m3_title,1");
        nextion_command("vis m3_rssi,1");
        nextion_command("vis m3_rssi_val,1");
        nextion_command("vis m3_DS,1");
        nextion_command("vis m3_DS_val,1");
        nextion_command("vis m3_DR,1");
        nextion_command("vis m3_DR_val,1");

        k_msleep(20);
        k_mutex_unlock(&display_mutex);
}

void display_more_devices()
{
        k_mutex_lock(&display_mutex, K_FOREVER);
        nextion_command("vis more,1");
        k_mutex_unlock(&display_mutex);
}


void hide_device_1()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis m1_pic,0");
        nextion_command("vis m1_title,0");
        nextion_command("vis m1_rssi,0");
        nextion_command("vis m1_rssi_val,0");
        nextion_command("vis m1_DS,0");
        nextion_command("vis m1_DS_val,0");
        nextion_command("vis m1_DR,0");
        nextion_command("vis m1_DR_val,0");   
        device_1_reset = true;

        k_mutex_unlock(&display_mutex);
}

void hide_device_2()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis m2_pic,0");
        nextion_command("vis m2_title,0");
        nextion_command("vis m2_rssi,0");
        nextion_command("vis m2_rssi_val,0");
        nextion_command("vis m2_DS,0");
        nextion_command("vis m2_DS_val,0");
        nextion_command("vis m2_DR,0");
        nextion_command("vis m2_DR_val,0");  
        device_2_reset = true; 

        k_mutex_unlock(&display_mutex);
}

void hide_device_3()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis m3_pic,0");
        nextion_command("vis m3_title,0");
        nextion_command("vis m3_rssi,0");
        nextion_command("vis m3_rssi_val,0");
        nextion_command("vis m3_DS,0");
        nextion_command("vis m3_DS_val,0");
        nextion_command("vis m3_DR,0");
        nextion_command("vis m3_DR_val,0");   
        device_3_reset = true;

        k_mutex_unlock(&display_mutex);
}

void reset_main_page()
{
        device_1_reset = true;
        device_2_reset = true;
        device_3_reset = true;  
}

void hide_more_devices()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis more,0");

        k_mutex_unlock(&display_mutex);
}

void display_loading_page(int deviceNbr)
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("page 3");

        char buf[50];

        sprintf(buf,"lbl_title.txt=\"Device %d\"",deviceNbr);
        nextion_command(buf);

        k_mutex_unlock(&display_mutex);
}

void display_device_page(int deviceNbr, int rssi, int nbrDays, enum main_state state)
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("page 2");

        char buf[50];

        sprintf(buf,"lbl_title.txt=\"Device %d\"",deviceNbr);
        nextion_command(buf);

        sprintf(buf,"lbl_rssi_val.txt=\"%d\"",rssi);
        nextion_command(buf);

        sprintf(buf,"lbl_DR_val.txt=\"%d\"",nbrDays);
        nextion_command(buf);

        if(state==ST_INIT)
                nextion_command("lbl_DS_val.txt=\"Initialization\"");
        else if(state==ST_WAIT_SD_CARD)
                nextion_command("lbl_DS_val.txt=\"No SD Card\"");
        else if(state==ST_IDLE)
                nextion_command("lbl_DS_val.txt=\"Idle\"");
        else if(state==ST_RECORDING)
                nextion_command("lbl_DS_val.txt=\"Recording\"");
        else if(state==ST_DISK_FULL)
                nextion_command("lbl_DS_val.txt=\"Disk Full\"");
        else if(state==ST_LOW_BATT)
                nextion_command("lbl_DS_val.txt=\"Low Battery\"");
        else if(state==ST_POWER_SAVING)
                nextion_command("lbl_DS_val.txt=\"Power Saving\"");
        else if(state==ST_ERROR)
                nextion_command("lbl_DS_val.txt=\"Error\"");

        k_mutex_unlock(&display_mutex);
}


void display_select_open()
{       
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("btn_open.pic=5");
        nextion_command("btn_reset.pic=8");
        nextion_command("btn_toggle.pic=10");
        nextion_command("btn_exit.pic=14");

        k_mutex_unlock(&display_mutex);
}
void display_select_reset()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("btn_open.pic=6");
        nextion_command("btn_reset.pic=7");
        nextion_command("btn_toggle.pic=10");
        nextion_command("btn_exit.pic=14");

        k_mutex_unlock(&display_mutex);
}
void display_select_toggle()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("btn_open.pic=6");
        nextion_command("btn_reset.pic=8");
        nextion_command("btn_toggle.pic=9");
        nextion_command("btn_exit.pic=14");

        k_mutex_unlock(&display_mutex);
}
void display_select_exit()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("btn_open.pic=6");
        nextion_command("btn_reset.pic=8");
        nextion_command("btn_toggle.pic=10");
        nextion_command("btn_exit.pic=13");

        k_mutex_unlock(&display_mutex);
}
void display_show_popup()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis popup,1");
        
        nextion_command("popup.pic=12");

        k_mutex_unlock(&display_mutex);
}

void display_show_popup_highlighted()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis popup,1");
        
        nextion_command("popup.pic=15");

        k_mutex_unlock(&display_mutex);
}

void display_hide_popup()
{
        k_mutex_lock(&display_mutex, K_FOREVER);

        nextion_command("vis popup,0");

        k_mutex_unlock(&display_mutex);
}


