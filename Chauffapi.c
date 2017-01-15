/*
 * Chauffapi Gestion chauffage basé sur le Raspberry Pi
 * Copyright Christian Lucuix September 2015
 * Version 0.1
 */

#include <syslog.h>
#include "general.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define VERSION_STR TOSTRING(VERSION)
#define SUBVERSION_STR TOSTRING(SUBVERSION)
// General variable

int mode = 0; // 0 = Auto, 1 = Nuit, 2 = Jour, 3 = HG, 4 = STOP
int oldMode =0;
int automode =1;  // 1 == Nuit, 2 == jour --> dépends du scrip execute par crontab
int oldAutomode =1;
int changed =1;
int backlight = 100;
int off = 0;  // switch-key off status
char ext_mode[6]; // from socket external mode
char ext_submode[6]; // from socket external sub mode
char ext_subTemp[6]; // from socket external temperature sub mode



// Path for the DS18S20 1wire reading temperature sensor
char devicepath[100];


float temp_jour = 20.0;
float oldTemp_jour=20.0;
float temp_nuit = 15.0;
float oldTemp_nuit=15.0;
float real_temp = 20.5;
float hg_temp = 10.0;
float ctrl_temp = 0.0;
float old_ctrl_temp = 0.0;
float selection = 0.0;
float ext_temp = 0.0;

int g,rep;
int foo=10;
int foo1=10;

FILE *fd_gpio;

// sockets configuration

int sockfd, newsockfd, portno = 51717, clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int n;
int data;
int debug =0;  // no debug by default

/*************************************
 * Prepare and read the Time and Date
 *************************************/




int main (int argc, char **argv)
{
    openlog("Chauffapi", LOG_CONS, LOG_USER);
    syslog(LOG_NOTICE, "Chauffapi version= " VERSION_STR "." SUBVERSION_STR  " starting");

	int time2wait = 10; //10 seconds to wait before starting by default

	if (argc > 4) //too much arguments we're supplied
			{
			fprintf(stderr, "You have't supplied too many arguments\n");
			exit(EXIT_FAILURE);
		}

		//Check the arguments
		int i;
		for(i = 1; i <argc; i++)
		{
			if (strncmp(argv[i], "SILENT", 5) == 0)
			{
				debug =0;
			}
			if (strncmp(argv[i], "DEBUG", 9) == 0)
			{
				debug =1;
			}

			if (strncmp(argv[i], "DELAY", 9) == 0)
			{
				// get delay from passed argument
				sscanf(argv[i+1],"%i",  &time2wait);

			}


		}


	/*********************************
	 * Definition of the threads
	 *********************************/
	   int s1_hand = 0;
	   int s2_hand = 0;
	   int s3_hand = 0;
	   int s4_hand = 0;
	   int s5_hand = 0;

	    pthread_t routines[6];


	    /**************************************
	     * Sleep 10 seconds to let the time
	     * to NTP to synchronize
	     **************************************/
	    if(debug ==1)
	    {
	    	printf("we sleep= %i \n", time2wait);
	    }

	    	sleep(time2wait);

	    if(debug ==1)
	    {
			printf("wake-up now \n");
	    }
/**********************************************
 * Guess the temperature senor path
 **********************************************/
	
	createSensorPath(devicepath);

/**************************************
 * Open and initialize the LCD display
 **************************************/
    pifacecad_open();

    lcd_init_Display();
    pifacecad_lcd_cursor_off();
    pifacecad_lcd_blink_off();
    iniSocks();

    //----------------------------
	//----- CREATE SEMAPHORE -----
	//----------------------------
    if(debug ==1)
    {
    	printf("Creating semaphore...\n");
    }
	semaphore1_id = semget((key_t)12345, 1, 0666 | IPC_CREAT);		//<<<<< SET THE SEMPAHORE KEY   (Semaphore key, number of semaphores required, flags)
	//	Semaphore key
	//		Unique non zero integer (usually 32 bit).  Needs to avoid clashing with another other processes semaphores (you just have to pick a random value and hope - ftok() can help with this but it still doesn't guarantee to avoid colision)

	//Initialize the semaphore using the SETVAL command in a semctl call (required before it can be used)
	union semun sem_union_init;
	sem_union_init.val = 1;
	if (semctl(semaphore1_id, 0, SETVAL, sem_union_init) == -1)
	{
	    if(debug ==1)
	    {
			fprintf(stderr, "Creating semaphore failed to initialize\n");
	    }
		exit(EXIT_FAILURE);
	}

    sleep(2);

		/*******************************************
		 * initialize the GPIO27 pin to control
		 * the Heater relay
		 *******************************************/
	if(debug ==1)
	{
		printf("Init ctrl relay ports\n");
	}
		// export the GPIO pin
		reset_gpio();
		//create pin27 and set direction to out
		setup_gpio();


		// Pin 13 on the header is pin 2 for WiringPi
		// (independent from the Raspberry type)
		if(debug ==1)
		{
			printf("Init control ports\n");
		}
	//	pinMode (2, OUTPUT) ;



				/********************************
				 * Creating the display Thread
				 * Cycle time 1s
				 ********************************/
				if(debug ==1)
				{
					printf("Creating all Threads\n");
				}

				s1_hand = pthread_create(&(routines[1]), NULL, display, (void *)&(routines[1]));
				s2_hand = pthread_create(&(routines[2]), NULL, control, (void *)&(routines[2]));
				s3_hand = pthread_create(&(routines[3]), NULL, keyboard, (void *)&(routines[3]));
				s4_hand = pthread_create(&(routines[4]), NULL, mode_ext, (void *)&(routines[4]));
				s5_hand = pthread_create(&(routines[5]), NULL, sock_Listen, (void *)&(routines[5]));

			   if (s5_hand != 0)
			   {
						perror("Chauffapi: Not possible to create sock_Listen threads:\n");
						exit(EXIT_FAILURE);
			   }
				   if (s4_hand != 0)
				   {
							perror("Chauffapi: Not possible to create mode_ext threads:\n");
							exit(EXIT_FAILURE);
				   }
					   if (s3_hand != 0)
					   {
								perror("Chauffapi: Not possible to create keyboard threads:\n");
								exit(EXIT_FAILURE);
					   }

						   if (s2_hand != 0)
						   {
									perror("Chauffapi: Not possible to create control threads:\n");
									exit(EXIT_FAILURE);
						   }

				   if (s1_hand != 0)
				   {
							perror("Chauffapi: Not possible to create threads:\n");
							exit(EXIT_FAILURE);
				   }


				 pthread_join(routines[1], NULL);
				 pthread_join(routines[2], NULL);
				 pthread_join(routines[3], NULL);
				 pthread_join(routines[4], NULL);
				 pthread_join(routines[5], NULL);

				 void* result1;
				 if ((pthread_join(routines[1], &result1)) == -1) {
					 perror("Chauffapi: Cannot join thread display");
						exit(EXIT_FAILURE);
				 }


				void* result2;
				if ((pthread_join(routines[2], &result2)) == -1) {
				   perror("Chauffapi: Cannot join thread control");
					exit(EXIT_FAILURE);
				}

				void* result3;
				if ((pthread_join(routines[3], &result3)) == -1) {
				   perror("Chauffapi: Cannot join thread keyboard");
					exit(EXIT_FAILURE);
				}

				void* result4;
				if ((pthread_join(routines[4], &result4)) == -1) {
				   perror("Chauffapi: Cannot join thread mode_ext");
					exit(EXIT_FAILURE);
				}

				void* result5;
				if ((pthread_join(routines[5], &result5)) == -1) {
				   perror("Chauffapi: Cannot join thread sock_Listen");
					exit(EXIT_FAILURE);
				}

	/********************************
	 * We close properly all opened
	 * libraries, ports, peripherals
	 ********************************/

		pthread_exit(NULL);
		pifacecad_close();

		//----------------------------
		//----- DELETE SEMAPHORE -----
		//----------------------------
		//It's important not to unintentionally leave semaphores existing after program execution. It also may cause problems next time you run the program.
		union semun sem_union_delete;
		if (semctl(semaphore1_id, 0, IPC_RMID, sem_union_delete) == -1)
			perror("Chauffapi: Failed to delete semaphore\n");


		return 0;

}



