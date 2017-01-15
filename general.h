/**
 * @file  general.h
 * @brief general rototypes for the Chauffapi application.
 *
 * Copyright (C) 2015 Christian Lucuix <christian.lucuix@wanadoo.fr>
  *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GENERAL_H
#define _GENERAL_H

#include <stdio.h>
#include <stdlib.h>
//#include <fstream>
//#include <iostream>
//#include <sstream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/sem.h>
#include "pifacecad.h"
//#include "../wiringPi/wiringPi.h"


#ifdef __cplusplus
extern "C" {
#endif


#define VERSION 2
#define SUBVERSION 0

/****************************************************************
 * 				Semaphore union declaration
 ****************************************************************/
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
/****************************************************************
 * 		Semaphore related prototype declaration
 ****************************************************************/

int semaphore1_get_access(void);
int semaphore1_release_access(void);

/****************************************************************
 * 		Semaphore variable declaration
 ****************************************************************/
int semaphore1_id;

/****************************************************************
 * Prototypes of the functions used in the Chauffapi application
 ****************************************************************/
// prototypes

int lcd_print_Time(void);
int lcd_print_Mode(void);
int lcd_init_Display(void);
int lcd_print_Ref_Temp(float realtemp);
int lcd_print_Real_Temp(float actualtemp);
double readTemp(char *path);

// Communication related functions
int iniSocks(void);
void sendData( int sockfd, char *data );
int getData( int sockfd );


// Threads prototypes
void *display(void *parameters);
void *control(void *parameters);
void *keyboard(void *parameters);
void *mode_ext(void *parameters);
void *sock_Listen(void *parameters);

// Temperature sensor related function
int createSensorPath(char *chemin);

// Handle GPIO

int reset_gpio(void);
int setup_gpio(void);
int  activeHeater(int onoff);
#ifdef __cplusplus
}
#endif

#endif
