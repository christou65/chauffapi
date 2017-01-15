/*
 * Chauffapi Gestion chauffage basé sur le Raspberry Pi
 * Copyright Christian Lucuix September 2015
 * Version 0.1
 */

#include <syslog.h>
#include "general.h"



// External variable declaration

extern int sockfd, portno , clilen;
extern struct sockaddr_in serv_addr, cli_addr;


extern char ext_mode[]; // from socket external mode
extern char ext_submode[]; // from socket external sub mode
extern char ext_subTemp[]; // from socket external temperature sub mode

extern int mode; // 0 = Auto, 1 = Nuit, 2 = Jour, 3 = HG, 4 = STOP
extern float ext_temp;

//extern struct tm *tm ;

extern int foo;
extern int foo1;

extern int debug;
extern char * devicepath;
//***********************************************************
//***********************************************************
//********** WAIT IF NECESSARY THEN LOCK SEMAPHORE **********
//***********************************************************
//***********************************************************
//Wait if necessary and then change the semaphore by –1. This is the "wait" operation
int semaphore1_get_access(void)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(semaphore1_id, &sem_b, 1) == -1)		//Wait until free
	{
		perror("Chauffapi: semaphore1_get_access failed\n");
		exit(EXIT_FAILURE);
	}
	return(1);
}

//***************************************
//***************************************
//********** RELEASE SEMAPHORE **********
//***************************************
//***************************************
//Setting the semaphore back to available.  This is the "release" operation.

int semaphore1_release_access(void)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(semaphore1_id, &sem_b, 1) == -1)
	{
		perror("semaphore1_release_access failed\n");
		exit(EXIT_FAILURE);
	}
	return(1);
}

/****************************************
 *  Check the w1 path to reqd the sensor
 ****************************************/
 int createSensorPath(char *chemin)
 {
FILE *fd;
char sensorname[20],dot[10], twodot[10] ;

  if(debug ==1)
        {
          printf("createSensorPath\n");
        }       

	// execute the comand and the result back
	// in background popen fork and create pipe 
	// to redirect stdout to a file descriptor
	//  recuperated here into fd

       fd = popen("ls -a /sys/bus/w1/devices/", "r");

  	if ( fd != NULL)
    	{

	// scan the file descriptor and get the output
	// As i know the format, it is easy to recuperate

    	      fscanf(fd,"%s %s %s",dot, twodot, sensorname);	
   		 sprintf(chemin, "/sys/bus/w1/devices/%s/w1_slave", sensorname) ;

 		if(debug ==1)
        	{
         		printf("finished CreateSensorPath Function \n");
        	}       

    	    return 0;

    	} else {
		printf("Error opening file /home/pi/workspace/test/path.txt\n");
	}
  return -1;
}


/****************************************
 *  Initialize communication sockets
 ****************************************/
int iniSocks(void)
{

	// display if in debug mode
	if(debug ==1)
	{
		  printf( "using port #%d\n", portno );
	}

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
	    perror("ERROR opening socket") ;
		exit(EXIT_FAILURE);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons( portno );
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0)
		  {
			  perror( "ERROR on binding" ) ;
			  exit(EXIT_FAILURE);
			}
  listen(sockfd,5);
  clilen = sizeof(cli_addr);

	return(0);

}
/****************************************
 *  sendData communication sockets
 ****************************************/
void sendData( int sockfd, char *data ) {
  int n;

  char buffer[32];
  sprintf( buffer, "%s\n", data );
  if ( (n = write( sockfd, buffer, strlen(buffer) ) ) < 0 )
  {
	  perror( "ERROR writing to socket");
	  exit(EXIT_FAILURE);
	}
  buffer[n] = '\0';
}
/****************************************
 *  sendData communication sockets
 ****************************************/
int getData( int sockfd ) {
  char buffer[32];
  int n;


  if ( (n = read(sockfd,buffer,31) ) < 0 )
  {  // error during reading
		// display if in debug mode
		if(debug ==1)
		{
		    printf( "ERROR reading from socket");
		}
    buffer[n] = '\0';
    return -1;
  }
  buffer[n] = '\0';

  // Check for outside command

  /**************************************
   * ECONOMY Command no submode submode
   **************************************/
	  if (strcmp(buffer, "ECO\n") ==0)
	  {
			// display if in debug mode
			if(debug ==1)
			{
				printf("ECO command received\n");
			}

		strcpy(ext_mode, "ECO");

	  }

	  /**************************************
	   * CONFORT Command no submode submode
	   **************************************/
		  if (strcmp(buffer, "CONF\n") ==0)
		  {
				// display if in debug mode
				if(debug ==1)
				{
					printf("CONF command received\n");
				}
				strcpy(ext_mode, "CONF");
		  }

		  /**************************************
		   * HGEL Command no submode submode
		   **************************************/
			  if (strcmp(buffer, "HGEL\n") ==0)
			  {
					// display if in debug mode
					if(debug ==1)
					{
						printf("HGEL command received\n");
					}
					strcpy(ext_mode, "HGEL");
			  }

			  /**************************************
			   * Automatic Command no submode submode
			   **************************************/
				  if (strcmp(buffer, "AUTO\n") ==0)
				  {
						// display if in debug mode
						if(debug ==1)
						{
							printf("AUTO command received\n");
						}
						strcpy(ext_mode, "AUTO");
				  }

		  /**************************************
		   * Automatic Command + Economy submode
		   **************************************/
		  if (strcmp(buffer, "AUTOE\n") ==0)
		  {
				// display if in debug mode
				if(debug ==1)
				{
					printf("AUTOE command received\n");
				}
				strcpy(ext_mode, "AUTO");
				strcpy(ext_submode, "ECO");

		  }

	  /**************************************
	   * Automatic Command + Comfort submode
	   **************************************/

	  if (strcmp(buffer, "AUTOC\n") ==0)
	  {
			// display if in debug mode
				if(debug ==1)
				{
					printf("AUTOC command received\n");
				}
				strcpy(ext_mode, "AUTO");
				strcpy(ext_submode, "CONF");

	  }
			  /**************************************
			   * ECONOMY Temperature setting
			   **************************************/

	  	  	  if (memcmp(buffer, "TECO\n",3) ==0)
			  {
					// display if in debug mode
					if(debug ==1)
					{
						printf("Economy Temperature received\n");
					}
					strcpy(ext_subTemp, "TECO");
				// scan the string and get the mode ant temperature
				sscanf(buffer,"%s %f", ext_subTemp, &ext_temp);

				// display if in debug mode
				if(debug ==1)
				{
					printf("ext_temp value %f\n", ext_temp);
				}

			  }

	  /**************************************
	   * COMFORT Temperature setting
	   **************************************/
	  if (memcmp(buffer, "TCONF\n",4) ==0)
	  {
			// display if in debug mode
			if(debug ==1)
			{
				printf("Comfort Temperature received\n");
			}
			strcpy(ext_subTemp, "TCONF");
		// scan the string and get the mode ant temperature
		sscanf(buffer,"%s %f", ext_subTemp, &ext_temp);
		// display if in debug mode
		if(debug ==1)
		{
			printf("ext_temp value %f\n", ext_temp);
		}

	  }
	  /**************************************
	   * Automatic Command no submode submode
	   **************************************/
		  if (strcmp(buffer, "STATUS\n") ==0)
		  {
				// display if in debug mode
				if(debug ==1)
				{
					printf("STATUS command received\n");
				}
			return 2;
		  }
		  /**************************************
		   * Debug Mode control
		   **************************************/
			  if (strcmp(buffer, "DEBUG\n") ==0)
			  {
				  debug=1;
					// display if in debug mode
					if(debug ==1)
					{
						printf("DEBUG command received\n");
					}
			  }
			  /**************************************
			   * SILENT Mode control
			   **************************************/
				  if (strcmp(buffer, "SILENT\n") ==0)
				  {
					  debug=0;
				  }

  return 0;
}
/****************************************
 *  Initialize the display mask pattern
 ****************************************/
int lcd_init_Display(void)
{
	char message[1];
	sprintf(message, "%c",0xdf);
	   /*************************************
	    * Display the Time and Date template
	    *************************************/
				pifacecad_lcd_set_cursor(0,0);
				pifacecad_lcd_write("Lun 00:00 20.0 C");
				pifacecad_lcd_set_cursor(14,0);
				pifacecad_lcd_write(message); // degree Symbol
				pifacecad_lcd_set_cursor(0,1);
				pifacecad_lcd_write("Conf Auto 20.0 C");
				pifacecad_lcd_set_cursor(14,1);
				pifacecad_lcd_write(message); // degree Symbol

	return 0;
}

/****************************
 * Print current Time value
 ****************************/
int lcd_print_Time(void)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	//time_t t = time(NULL);
	char s[64];

	if (foo ==0)
	{
	  foo =10;

	  // format the day of the week with %a

		FILE *fp= popen("date +%a","r");
		if (fp !=NULL)
		{
			if (fgets(s,100,fp) !=NULL)
			{

				// we close pipe only if opened
				pclose(fp);

			/*************************************
			 * set cursor to 6,0
			 *  display the Day of the week
			 *  As name of day is all in lower case
			 *  I change display to make it upper case
			 *  Ok more clever ways exists...
			 *************************************/
					pifacecad_lcd_set_cursor(0,0);
					/*********************************************************
					 * we need to check against both English and French date
					 * because @ boot default language is English.
					 * Only when opening a terminal language is changed to French
					 *********************************************************/
					if ((strncmp(s, "lun", 3) == 0)|| (strncmp(s, "Mon", 3) == 0))
					{
					pifacecad_lcd_write("Lun");
					}
					if ((strncmp(s, "mar", 3) == 0)|| (strncmp(s, "Tue", 3) == 0))
						{
						pifacecad_lcd_write("Mar");
						}
						if ((strncmp(s, "mer", 3) == 0)|| (strncmp(s, "Wed", 3) == 0))
							{
							pifacecad_lcd_write("Mer");
							}
							if ((strncmp(s, "jeu", 3) == 0)|| (strncmp(s, "Thu", 3) == 0))
								{
								pifacecad_lcd_write("Jeu");
								}
							if ((strncmp(s, "ven", 3) == 0)|| (strncmp(s, "Fri", 3) == 0))
							{
							pifacecad_lcd_write("Ven");
							}
						if ((strncmp(s, "sam", 3) == 0)|| (strncmp(s, "Sat", 3) == 0))
						{
						pifacecad_lcd_write("Sam");
						}
					if ((strncmp(s, "dim", 3) == 0)|| (strncmp(s, "Sun", 3) == 0))
					{
					pifacecad_lcd_write("Dim");
					}


			} else {
				perror("unable to read date data pipe\n");

			}
		} else {
			perror("failed to open date pipe\n");
		}

				  // format the time in HH:MM
				  strftime(s, sizeof(s), "%R", tm);

		    /*************************************
			 * set cursor to 0,0
			 *  display the Time
			 *************************************/
		  	pifacecad_lcd_set_cursor(4,0);
		  	pifacecad_lcd_write((const char *) s);
	} else {
		foo--;
	}
	return 0;
}
/**********************
 * Display system Mode
 **********************/

int lcd_print_Mode(void)
{

			  switch(mode)
			  {
				  case 0:  // Mode Auto
				  {
						/*************************************
						 * set cursor to 5,1 5th column raw 1
						 *  display the Mode
						 *************************************/
							pifacecad_lcd_set_cursor(5,1);
							pifacecad_lcd_write( "Auto");
					  break;
				  }
				  case 1:  // Mode Nuit
				  {
						/*************************************
						 * set cursor to 5,1 5th column raw 1
						 *  display the Mode
						 *************************************/
							pifacecad_lcd_set_cursor(5,1);
							pifacecad_lcd_write( "Nuit");
							pifacecad_lcd_set_cursor(0,1);
							pifacecad_lcd_write("Eco ");
					  break;
				  }
				  case 2:  // Mode Jour
				  {
						/*************************************
						 * set cursor to 5,1 5th column raw 1
						 *  display the Mode
						 *************************************/
							pifacecad_lcd_set_cursor(5,1);
							pifacecad_lcd_write( "Jour");
							pifacecad_lcd_set_cursor(0,1);
							pifacecad_lcd_write("Conf");
					  break;
				  }
				  case 3:  // Mode Hors Gel
				  {
						/*************************************
						 * set cursor to 5,1 5th column raw 1
						 *  display the Mode
						 *************************************/
							pifacecad_lcd_set_cursor(5,1);
							pifacecad_lcd_write( "Hgel");
							pifacecad_lcd_set_cursor(0,1);
							pifacecad_lcd_write("    ");
					  break;
				  }
				  case 4:  // Mode STOP
				  {
						/*************************************
						 * set cursor to 5,1 5th column raw 1
						 *  display the Mode
						 *************************************/
							pifacecad_lcd_set_cursor(5,1);
							pifacecad_lcd_write( "Stop");
							pifacecad_lcd_set_cursor(0,1);
							pifacecad_lcd_write("    ");
					  break;
				  }
			  }


	return 0;

}
/********************************
 * Display Reference temperature
 ********************************/
int lcd_print_Ref_Temp (float reftemp){

	char message[4];
		/*************************************
		 * set cursor to 12,1 5th column raw 1
		 *  display the Reference temperature
		 *************************************/
				pifacecad_lcd_set_cursor(10,1);
			  sprintf(message,"%-.1f",reftemp );

			 // display the Time
			pifacecad_lcd_write (message);


		return 0;

	}


/*****************************
 * Display Real temperature
 *****************************/
int lcd_print_Real_Temp(float actualtemp){
	char	message[4];
    /*************************************
	 * set cursor to 12,1 5th column raw 1
	 *  display the Real temperature
	 *************************************/
	if (foo1==0)
	{
		foo1=10;
		pifacecad_lcd_set_cursor(10,0);
		  sprintf(message,"%-.1f",actualtemp );

		 // display the Time
				pifacecad_lcd_write(message);
	} else {
		foo1--;
	}
		return 0;

	}


/*************************
 * Read Real temperature
 *************************/
double readTemp(char *path)
{
   if(debug ==1)
    {
	printf("readTemp Function \n");
	printf("path = %s  \n", path);
    }
	FILE *device = fopen(path, "r");
	double temperature = -1;
	char crcVar[5];
	if (device == NULL)
	{
		// display if in debug mode
		if(debug ==1)
		{
			printf("Check connections %s\n", path);
		}
		perror("Failed to open temperature descriptor\n");
	}
			if (device != NULL)
			{
				if (!ferror(device))
				{
					fscanf(device, "%*x %*x %*x %*x %*x %*x %*x %*x %*x : crc=%*x %s", crcVar);
					if (strncmp(crcVar, "YES", 3) == 0)
					{
						fscanf(device, "%*x %*x %*x %*x %*x %*x %*x %*x %*x t=%lf", &temperature);

						temperature /= 1000.0;
						// display if in debug mode
					}

				fclose(device);

				} else {
					perror("error on temperature file pointer\n");

				}
			}
	return temperature;
}


int reset_gpio(void)
{
    int fd, len;
    int  port = 27;
    char str[0x100];

    if ((fd = open("/sys/class/gpio/unexport", O_WRONLY)) < 0)
    {
        char *errstr = strerror(errno);
        syslog(LOG_ERR, "Failed to open /sys/class/gpio/unexport: %s", errstr);
        return -1;
    }
    else
    {
            len = snprintf(str, sizeof(str), "%d", port);
            write(fd, str, len);
        close(fd);
    }
    return 0;
}

int setup_gpio(void)
{
    int fd, len;
    int port = 27 ;
    char str[0x100];

    if (reset_gpio() < 0)
    {
        return -1;
    }

	// export the gpio class

    if ((fd = open("/sys/class/gpio/export", O_WRONLY)) < 0)
    {
        char *errstr = strerror(errno);
        syslog(LOG_ERR, "Failed to open /sys/class/gpio/export: %s", errstr);
        return -1;
    }  else {
            len = snprintf(str, sizeof(str), "%d", port);
            if (write(fd, str, len) != len)
            {
                char *errstr = strerror(errno);
                syslog(LOG_ERR, "Failed to write to /sys/class/gpio/export: %s", errstr);
                close(fd);
                return -1;
            }
        close(fd);
    }

	// check that the gpio is created
        
	snprintf(str, sizeof(str), "/sys/class/gpio/gpio%d/value", port);
        while (access(str, F_OK) < 0)
        {
            if (errno != ENOENT)
            {
                return -1;
            }
        }

// Now set the port direction to OUT

        snprintf(str, sizeof(str), "/sys/class/gpio/gpio%d/direction", port);
        if ((fd = open(str, O_WRONLY)) < 0)
        {
            char *errstr = strerror(errno);
            syslog(LOG_ERR, "Failed to open /sys/class/gpio/gpio%d/direction: %s", port, errstr);
            return -1;
        }
        if (write(fd, "out", 4) != 4)
        {
            char *errstr = strerror(errno);
            syslog(LOG_ERR, "Failed to write to /sys/class/gpio/gpio%d/direction: %s",port , errstr);
            close(fd);
            return -1;
        }
        close(fd);

    return 0;

}

int  activeHeater(int onoff)
 {
  int fd;
  int port = 27;
  char str[0x100];


       snprintf(str, sizeof(str), "/sys/class/gpio/gpio%d/value", port);
        if ((fd = open(str, O_WRONLY)) < 0)
        {
            char *errstr = strerror(errno);
            syslog(LOG_ERR, "Failed to open /sys/class/gpio/gpio%d/value: %s", port, errstr);
            return -1;
        }
       if (write(fd, (onoff & 1) ? "1" : "0", 2) != 2)
         {
            char *errstr = strerror(errno);
            syslog(LOG_ERR, "Failed to write to /sys/class/gpio/gpio%d/value: %s", port , errstr);
            return -1;
        }
        close(fd);
  return 0;
 }
