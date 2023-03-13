/*
 * Copyright (c) 2013-2014, Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Altera Corporation nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <getopt.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MSG(args...) printf(args)

#define FPGA_BIN_PATH "/image/"

char FPGA_BIN_FILE[256]={'\0'};

int gpio_export(int pin);

int gpio_unexport(int pin);

int gpio_direction(int pin, int dir);

int gpio_write(int pin, int value);

int gpio_read(int pin);

int gpio_export(int pin)  
{  
    char buffer[64];  
    int len;  
    int fd;  

    fd = open("/sys/class/gpio/export", O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open export %d for writing!\n",pin);  
        return(-1);  
    }  
    len = snprintf(buffer, sizeof(buffer), "%d", pin);  
    if (write(fd, buffer, len) < 0) {  
        MSG("error: export gpio %d!",pin);  
        return -1;  
    }
    close(fd);  
    return 0;  
}  

int gpio_unexport(int pin)  
{  
    char buffer[64];  
    int len;  
    int fd;  

    fd = open("/sys/class/gpio/unexport", O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open unexport %d for writing!\n",pin);  
        return -1;  
    }  
    len = snprintf(buffer, sizeof(buffer), "%d", pin);  
    if (write(fd, buffer, len) < 0) {  
        MSG("error: unexport gpio %d!",pin);  
        return -1;  
    }  
    close(fd);  
    return 0;  

} 

//dir: 0-->IN, 1-->OUT
int gpio_direction(int pin, int dir)  
{  

    static const char dir_str[] = "in\0out";  
    char path[64];  
    int fd;  
	
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);  
    fd = open(path, O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open gpio %d direction for writing!\n",pin);  
        return -1;  
    }  

    if (write(fd, &dir_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0) {  
        MSG("error: set direction gpio %d!\n",pin);  
        return -1;  
    }  
    close(fd);  
    return 0;  
}  

int gpio_open_directly(int pin){
	char path[64];  
	int fd;  

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);  
    fd = open(path, O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open gpio %d value for writing!\n",pin);  
        return -1;  
    }
	return fd;
}

int gpio_write_directly(int pin, int fd,int value){
    static const char values_str[] = "01";  

    if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0) {  
        MSG("error: write value  %d!\n",pin);  
        return -1;  
    }

    return 0;  
}

void gpio_close_directly(int pin, int fd){
    close(fd);  
}

//value: 0-->LOW, 1-->HIGH
int gpio_write(int pin, int value)  
{  
    static const char values_str[] = "01";  
    char path[64];  
    int fd;  

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);  
    fd = open(path, O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open gpio %d value for writing!\n",pin);  
        return -1;  
    }  

    if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0) {  
        MSG("error: write value gpio %d!\n",pin);  
        return -1;  
    }
 	//MSG("write %d succeed!\n",pin);
    close(fd);  
    return 0;  

}

int gpio_read_nonblock(int pin)  
{  
    char path[64];  
    char value_str[3];  
    int fd;  

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);  
    fd = open(path, O_RDONLY|O_NONBLOCK);  
    if (fd < 0) {  
        MSG("error: open gpio %d value for reading!\n",pin);  
        return -1;  
    }  

    if (read(fd, value_str, 3) < 0) {  
        MSG("error: read value for gpio %d!\n",pin);  
        return -1;  
    }  
	
    close(fd);  
    return (atoi(value_str));

}  

int gpio_read(int pin)  
{  
    char path[64];  
    char value_str[3];  
    int fd;  

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);  
    fd = open(path, O_RDONLY);  
    if (fd < 0) {  
        MSG("error: open gpio %d value for reading!\n",pin);  
        return -1;  
    }  

    if (read(fd, value_str, 3) < 0) {  
        MSG("error: read value for gpio %d!\n",pin);  
        return -1;  
    }  
    close(fd);  
    return (atoi(value_str));

}  

// 0-->none, 1-->rising, 2-->falling, 3-->both

int gpio_edge(int pin, int edge)
{
	const char dir_str[] = "none\0rising\0falling\0both"; 
	int ptr;
	char path[64];  
    int fd; 

	switch(edge){
		case 0:
			ptr = 0;
			break;
		case 1:
			ptr = 5;
			break;
		case 2:
			ptr = 12;
			break;
		case 3:
			ptr = 20;
			break;
		default:
			ptr = 0;
	} 

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/edge", pin);  
    fd = open(path, O_WRONLY);  
    if (fd < 0) {  
        MSG("error: open gpio %d edge for writing!\n",pin);  
        return -1;  
    }  

    if (write(fd, &dir_str[ptr], strlen(&dir_str[ptr])) < 0) {  
        MSG("error: set edge for gpio %d!\n",pin);  
        return -1;  
    }  

    close(fd);  
    return 0;  

}

#define BEEP 50
#define GREEN_LED 45
#define RED_LED 23

#define PROGRAM_B0 113
#define INIT_B0 86
#define DONE0 7

#define RESET0 117
#define PLUG0 22

#define RESET1 44
#define PLUG1 27

#define RESET2 46
#define PLUG2 47

#define RESET3 88
#define PLUG3 115

#define MAXLEN 1024*512

static const char *device = "/dev/spidev2-1.0";
static uint8_t mode = 0|SPI_CPHA|SPI_CPOL;
static uint8_t bits = 8;
static uint32_t speed = 24000000;
static uint16_t delay;


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int transfer(int fd,char* buf,int len)
{
	int ret;
	struct spi_ioc_transfer tr[1] = {
		{
		.tx_buf = (unsigned long)buf,
		.rx_buf = 0,
		.len = len,//ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		}
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), tr);
	if (ret < 1){
		return -1;
	}
	
	return 0;
}


int spiInit(){
	int fd;
	int ret = 0;
	
	fd = open(device, O_RDWR);
	if(fd < 0){
		MSG("SPI: open device %s failed\n",device);
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret < 0){
		goto ERROR;
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret < 0){
		goto ERROR;
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret < 0){
		goto ERROR;
	}

	MSG("mode: %d\n", mode);
	MSG("bits: %d\n", bits);
	MSG("speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return fd;

ERROR:
	close(fd);
	return -1;
}

int downloadFgpa0(char *buf,int spiFd){
	int i=0;
	int retry=0;
	int rc=0;
	int totalLength=0;
	FILE * infile = NULL;
	
	retry = 0;
	while(1){
		if(retry ++ >100){
			goto ERROR;
		}
		i = gpio_read_nonblock(INIT_B0);
		if(i == 1){
			break;
		}else{
			usleep(10000);
		}
	}

	gpio_write(PROGRAM_B0,0);
	retry = 0;
	while(1){
		if(retry ++ >100){
			goto ERROR;
		}
		i = gpio_read_nonblock(INIT_B0);
		if(i==0){
			gpio_write(PROGRAM_B0, 1);
			break;
		}
	}

	retry = 0;
	while(1){
		if(retry ++ >100){
			goto ERROR;
		}
		i = gpio_read_nonblock(INIT_B0);
		if(i==1){
			break;
		}
	}

	infile = fopen(FPGA_BIN_FILE,"rb");
	if(infile == NULL){
		MSG("EROOR: open %s failed\n",FPGA_BIN_FILE);
		goto ERROR;
	}

	while((rc = fread(buf,sizeof(char),MAXLEN,infile))!=0){
		totalLength += rc;
		MSG("write fiel size: %d\n",totalLength);
		if(transfer(spiFd,buf,rc)<0){
			MSG("ERROR: spi transfer failed\n");
			goto ERROR;
		}
		//sleep(1);
	}

	retry = 0;
	while(1){
		if(retry ++ >100){
			goto ERROR;
		}
		i = gpio_read_nonblock(DONE0);
		if(i==1){
			break;
		}
	}

	MSG("downloadf image finish \n");
	
	fclose(infile);

	return 0;

ERROR:
	if(infile != NULL){
		fclose(infile);
	}
	return -1;
}



int main(int argc, char *argv[]){
	int retry=0;
	int blink = 0;
	int spiFd = -1;
	int chainNum=0;
	int restart = 0;
	char fname[256]={0};

	memset(fname,0,256);
	memset(FPGA_BIN_FILE,0,256);

	if(argc <= 1 || argv[1] == NULL){
		strncpy(FPGA_BIN_FILE,"/image/fpgaminer_top_btc.bit",strlen("/fpgabit/fpgaminer_top_btc.bit"));
	}else{
		strncpy(FPGA_BIN_FILE,FPGA_BIN_PATH,strlen(FPGA_BIN_PATH));
		strncpy(FPGA_BIN_FILE+strlen(FPGA_BIN_PATH),argv[1],strlen(argv[1]));
	}
	
	char *buf = (char*)malloc(MAXLEN);

	gpio_export(GREEN_LED);
	gpio_direction(GREEN_LED,1);
	gpio_write(GREEN_LED,1);

	gpio_export(RED_LED);
	gpio_direction(RED_LED,1);
	gpio_write(RED_LED,0);
	
	//BEEP:50
	gpio_export(BEEP);
	gpio_direction(BEEP, 1);
	gpio_write(BEEP, 0);

	//PLUG0:22
	gpio_export(PLUG0);
	gpio_direction(PLUG0, 0);
	
	//PROGRAM_B0:113
	gpio_export(PROGRAM_B0);
	gpio_direction(PROGRAM_B0, 1);
	gpio_write(PROGRAM_B0, 1);

	//INIT_B0:110
	gpio_export(INIT_B0);
	gpio_direction(INIT_B0, 0);

	//DONE0:86
	gpio_export(DONE0);
	gpio_direction(DONE0, 0);

	//RESET0:117
	gpio_export(RESET0);
	gpio_direction(RESET0, 1);
	gpio_write(RESET0, 1);
			
	//PLUG1:27
	gpio_export(PLUG1);
	gpio_direction(PLUG1, 0);

	//RESET1:44
	gpio_export(RESET1);
	gpio_direction(RESET1, 1);
	gpio_write(RESET1, 1);

	//RESET2:46
	gpio_export(RESET2);
	gpio_direction(RESET2, 1);
	gpio_write(RESET2, 1);

	//PLUG2:47
	gpio_export(PLUG2);
	gpio_direction(PLUG2, 0);

	//RESET3:88
	gpio_export(RESET3);
	gpio_direction(RESET3, 1);
	gpio_write(RESET3, 1);

	//PLUG3:115
	gpio_export(PLUG3);
	gpio_direction(PLUG3, 0);


RESTART:
	chainNum = 0;
	spiFd = -1;
	retry=0;
	blink = 0;
	
	if(gpio_read(PLUG0) !=1){
		MSG("Chain 0 is available\n");
	}else{
		chainNum++;
	}

	if(gpio_read(PLUG1) !=1){
		MSG("Chain 1 is available\n");
	}else{
		chainNum++;
	}

	if(gpio_read(PLUG2) !=1){
		MSG("Chain 2 is available\n");
	}else{
		chainNum++;
	}

	if(gpio_read(PLUG3) !=1){
		MSG("Chain 3 is available\n");
	}else{
		chainNum++;
	}

	gpio_write(RESET0, 0);
	gpio_write(RESET0, 1);

	gpio_write(RESET1, 0);
	gpio_write(RESET1, 1);

	gpio_write(RESET2, 0);
	gpio_write(RESET2, 1);

	gpio_write(RESET3, 0);
	gpio_write(RESET3, 1);
	
	if(chainNum == 0){
		goto ERROR;
	}

	spiFd = spiInit();
	if(spiFd == -1){
		goto ERROR;
	}

	memset(buf,0,MAXLEN);
	retry = 0;
	while(1){
		if(retry ++ >2){
			goto ERROR;
		}
		if(downloadFgpa0(buf,spiFd) == 0){
			break;
		}
		sleep(1);
	}
	gpio_write(RESET0, 0);
	gpio_write(RESET0, 1);

	gpio_write(RESET1, 0);
	gpio_write(RESET1, 1);

	gpio_write(RESET2, 0);
	gpio_write(RESET2, 1);

	gpio_write(RESET3, 0);
	gpio_write(RESET3, 1);
	
	MSG("init fpga done");

	close(spiFd);

	return 0;

ERROR:
	if(spiFd !=-1){
		close(spiFd);
	}

	if(restart++ < 3){
		sleep(1);
		goto RESTART;
	}
	
	gpio_write(BEEP, 0);
	gpio_write(GREEN_LED,0);
	while(1){
		gpio_write(RED_LED,blink?1:0);
		blink=!blink;
		sleep(2);
		MSG("dead loop for init fpga !!!!!!!!!!!!!");
	}
	return -1;
}  
