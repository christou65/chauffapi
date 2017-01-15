/*
 * Chauffapi Gestion chauffage basé sur le Raspberry Pi
 * Copyright Christian Lucuix September 2015
 * Version 0.1
 */


#include "general.h"

// External variable declaration

extern int sockfd, newsockfd , clilen;
extern struct sockaddr_in  cli_addr;


extern char ext_mode[]; // from socket external mode
extern char ext_submode[]; // from socket external sub mode
extern char ext_subTemp[]; // from socket external temperature sub mode
extern float ext_temp;
extern FILE *fd_gpio;
extern int mode; // 0 = Auto, 1 = Nuit, 2 = Jour, 3 = HG, 4 = STOP
extern int changed;
extern int automode;  // 1 == Nuit, 2 == jour --> dépends du scrip execute par crontab
extern int oldMode;
extern int oldAutomode;
extern int backlight;
extern char devicepath[];
extern int off;  // switch-key off status


extern float real_temp;
extern float temp_nuit;
extern float selection;
extern float temp_jour;
extern float oldTemp_jour;
extern float oldTemp_nuit;
extern float hg_temp;
extern float ctrl_temp;
extern float old_ctrl_temp;
extern float ext_temp;

extern int debug;

/*
 * ------------------------- Threads -----------------------------
 */

/**************************
 * Display control Thread
 **************************/

void *sock_Listen(void *parameters) {
  int result =-1;
	char data[100];

	// display if in debug mode
	if(debug ==1)
	{
		printf("socks_Listen  starting...\n");
	}

    //--- infinite wait on a connection ---
    while ( 1 ) {
    	// display if in debug mode
    	if(debug ==1)
    	{
    		printf( "waiting for new client...\n" );
    	}
       if ( ( newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
           printf("ERROR on accept") ;
       // display if in debug mode
      	if(debug ==1)
       	{
      		printf( "opened new communication with client\n" );
       	}
					//---- wait for client command ---
					result = getData( newsockfd );

					if (result == 2){ // we ask for status


						sprintf(data,"%d %d temp = %f ECO=%f CONF=%f\n",mode, automode, real_temp, temp_nuit, temp_jour );
						sendData( newsockfd, data );

					}
					 //--- send new data back ---

       	   close( newsockfd );
           // display if in debug mode
          	if(debug ==1)
           	{
                printf( "closed  communication with client\n" );
          	}

    }
       perror( "Chauffapi: unforeseen exit from sock_Listen Thread\n" );
       exit(EXIT_FAILURE);

//  pthread_exit(NULL);
  //return 0;
}

/**************************
 * Display control Thread
 **************************/

void *display(void *parameters) {
	// display if in debug mode
	if(debug ==1)
	{
	    printf("display  starting...\n");
	}

 		while (1)
		{
			/*************************
			 * Display the local time
			 *************************/
			 lcd_print_Time();

				/******************************
				 * Display the real temperature
				 ******************************/

			 lcd_print_Real_Temp(real_temp);

				/*******************************
				 * If in Auto mode, depending
				 * on day/night value in automode
				 * Display day or night ref temp
				 *******************************/

				if (changed ==1)
				{

						if (mode ==0) // Auto
						{
							/*************************
							 * Display night ref temp
							 *************************/
							if (automode == 1) // eco
							{
								// we change the ref point according to t
								// user request
									temp_nuit +=selection;
									//we reset the increment
									selection = 0.0;
									// we select the right ref for the controller
									 // ctrl_temp = temp_nuit
									ctrl_temp = temp_nuit;

									lcd_print_Ref_Temp(temp_nuit);
									// display Auto sub Mode
									pifacecad_lcd_set_cursor(0,1);
								 // display the Time
								pifacecad_lcd_write("Eco ");

							}
								/*************************
								 * Display day ref temp
								 *************************/
								if (automode == 2) // conf
									{
										// we change the ref point according to t
										// user request
											temp_jour +=selection;
											//we reset the increment
											selection = 0.0;
											 // ctrl_temp = temp_nuit
											ctrl_temp = temp_jour;

											// display the ref value
										lcd_print_Ref_Temp(temp_jour);
											// display Auto sub Mode
											pifacecad_lcd_set_cursor(0,1);
										 // display the Time
										pifacecad_lcd_write("Conf" );
									}

						 }

						// Any mode now

						/*************************
						 * Display night ref temp
						 * if in manual night mode
						 *************************/

							if (mode ==1) // nuit
							{
								// we change the ref point according to t
								// user request
									temp_nuit +=selection;
									//we reset the increment
									selection = 0.0;
									 // ctrl_temp = temp_nuit
									ctrl_temp = temp_nuit;
								// display the ref value
								lcd_print_Ref_Temp(temp_nuit);

							}
						/*************************
						 * Display day ref temp
						 * if in manual day mode
						 *************************/
						if (mode ==2) // jour
						{
							// we change the ref point according to t
							// user request
								temp_jour +=selection;
									//we reset the increment
									selection = 0.0;
								 // ctrl_temp = temp_nuit
								ctrl_temp = temp_jour;
							// display the ref value
							lcd_print_Ref_Temp(temp_jour);

						}
						/*************************
						 * Display hg ref temp
						 * if in manual hg mode
						 *************************/
						if (mode ==3) // HG
						{
							// we change the ref point according to t
							// user request
								hg_temp +=selection;
									//we reset the increment
									selection = 0.0;
								 // ctrl_temp = temp_nuit
								ctrl_temp = hg_temp;
							// display the ref value
							lcd_print_Ref_Temp(hg_temp);


						}
						/*************************
						 * Display OFF mode
						 * if in manual STOP mode
						 *************************/
						if (mode ==4) // STOP
						{
							pifacecad_lcd_set_cursor(10,1);
							 // display the Time
							pifacecad_lcd_write ("XX.X");
							 // ctrl_temp = temp_nuit
							ctrl_temp = 0.0;

						}

						/*************************
						 * Display running mode
						 * Auto/NIGHT/DAY/HG/STOP
						 *************************/
						lcd_print_Mode();

				}


			/*************************
			 * All needed display done
			 * We reset the changed variable
			 * for next modification
			 *************************/
			if (semaphore1_get_access())
			{
				changed=0;
				semaphore1_release_access();

			}

			/*************************
			 * When a key is touched
			 * switch-on the display
			 * backlight for 10 seconds
			 *************************/

			//----- SEMAPHORE GET ACCESS -----
					if (backlight ==100)
					{
						  pifacecad_lcd_backlight_on();
					}

							if (backlight == 1)
							{
								  pifacecad_lcd_backlight_off();
							}


							if(backlight !=0)
							{
									backlight--;
							}
			// sleep 100 msecond
			usleep(100000);
		}
        perror( "Chauffapi: unforeseen exit from display Thread\n" );
        exit(EXIT_FAILURE);

 //  pthread_exit(NULL);
   //return 0;
}

/***************************************
 * mode and Temp external config Thread
 ***************************************/
void *mode_ext(void *parameters) {
	// display if in debug mode
	if(debug ==1)
	{
		   printf("mode_ext starting...\n");
	}

		while (1)
		{
		  if ((changed==0)&&(backlight==0))
		  {

						/***********************************
						 * Mode auto , we check for submode
						 ***********************************/
						if (strncmp(ext_mode, "AUTO", 4) == 0)
						{
							if (semaphore1_get_access())
							{

								if(oldMode != 0){
									mode = 0;
									oldMode =0;
									  backlight=100;
								  changed=1;
								}
								semaphore1_release_access();
							}
                                                                                /******************************
										 * Mode auto with submode ECO
										 ******************************/
										if (strncmp(ext_submode, "ECO", 4) == 0)
										{

											if (semaphore1_get_access())
											{
												if(oldAutomode != 1){
													// in all cases, AUTO takes precedence for heating if not in HGEL
													if ( mode != 3)
													{ 
														mode = 0;
														oldMode= 0;
													}
													automode = 1;
													oldAutomode =1;
													 backlight=100;
													  changed=1;
												}
												semaphore1_release_access();
											}

										}
								/*********************************
								 * Mode auto with submode CONFORT
								 *********************************/
									if (strncmp(ext_submode, "CONF", 4) == 0)
									{

										if (semaphore1_get_access())
										{
											if(oldAutomode != 2){
                                                                                          // in all cases, AUTO takes precedence for heating if not in HGEL
                                                                                            if ( mode != 3)
                                                                                             { 
                                                                                                 mode = 0;
                                                                                                 oldMode= 0;
                                                                                              }
											automode = 2;
											oldAutomode =2;
											 backlight=100;
											  changed=1;
											}
											semaphore1_release_access();
										}

									}

						     }
											/*********************************
											 * Mode CONFORT no submode
											 *********************************/
											if (strncmp(ext_mode, "CONF", 4) == 0)
											{
												if (semaphore1_get_access())
												{
														if(oldMode != 2){
															mode = 2;
															oldMode =2;
															backlight=100;
															changed=1;

															}
													semaphore1_release_access();
												}

											}

									/*********************************
									 * Mode ECONOMY no submode
									 *********************************/
									if (strncmp(ext_mode, "ECO", 4) == 0)
									{
										if (semaphore1_get_access())
										{
											if(oldMode != 1){
												mode = 1;
												oldMode =1;
												backlight=100;
												changed=1;
											}
											semaphore1_release_access();
										}

									}

							/*********************************
							 * Mode HGEL no submode
							 *********************************/
							if (strncmp(ext_mode, "HGEL", 4) == 0)
							{
								if (semaphore1_get_access())
								{
									if(oldMode != 3){
										mode = 3;
										oldMode =3;
										backlight=100;
										changed=1;
									}
									semaphore1_release_access();
								}

							}

					/*********************************
					 * Mode STOP no submode
					 *********************************/
					if (strncmp(ext_mode, "STOP", 4) == 0)
					{
						if (semaphore1_get_access())
						{
							if(oldMode != 4){
								mode = 4;
								oldMode =4;
								backlight=100;
								changed=1;
							}
							semaphore1_release_access();
						}

					}


					/**********************
					 * Temperature setting
					 **********************/


			// Mode Eco
			if (strncmp(ext_subTemp, "TECO", 4) == 0)
			{

				if (semaphore1_get_access())
				{
					if(oldTemp_nuit != ext_temp){
						temp_nuit = ext_temp;
						oldTemp_nuit =ext_temp;
						backlight=100;
						changed=1;
					}
					semaphore1_release_access();
				}

			}

					// Mode Confort
					if (strncmp(ext_subTemp, "TCONF", 4) == 0)
					{
						if (semaphore1_get_access())
						{
							if(oldTemp_jour != ext_temp){
								temp_jour = ext_temp;
								oldTemp_jour =ext_temp;
								backlight=100;
								changed=1;
							}
							semaphore1_release_access();
						}
				   }


		  }

			sleep(1);


		}
	        perror( "Chauffapi: unforeseen exit from mode_ext Thread\n" );
	        exit(EXIT_FAILURE);

	 //  pthread_exit(NULL);
	   //return 0;
}
/************************
 * Mode control  Thread
 ************************/
void *control(void *parameters) {

	// display if in debug mode
	if(debug ==1)
	{
		   printf("control starting...\n");
	}

		while (1)
		{
				real_temp = readTemp(devicepath);
			 if (real_temp < ctrl_temp )
			 {
				 // if delta(t) > 0.3 --> we switch-on the heater

				// if ((real_temp + 0.3)<= ctrl_temp )
					 // We switch-on the heater
					activeHeater(1);  //ON
			 }

					 if (real_temp > ctrl_temp )
					 {
						 // if delta(t) < 0.3 --> we switch-off the heater
				//		 if ((ctrl_temp + 0.2)<=real_temp  )
							 // We switch-off the heater
							activeHeater(0); ; //OFF
					 }
			   if(old_ctrl_temp != ctrl_temp)
			   {
					old_ctrl_temp = ctrl_temp;
			   }

			//lcd_print_Time();
			// sleep 1 second
			sleep(1);
		}
	        perror( "Chauffapi: unforeseen exit from mode_ext Thread\n" );
	        exit(EXIT_FAILURE);

	 //  pthread_exit(NULL);
	   //return 0;
}
/*
 * Keyboard control Thread
 */
void *keyboard(void *parameters) {

	// display if in debug mode
	if(debug ==1)
	{
		printf("keyboard  starting...\n");
	}
    uint8_t switch_bits =0;

    while (1)
		{

			  switch_bits = pifacecad_read_switches();

			//lcd_print_Time();
			// sleep 0.1 second
				usleep(100000);
			  switch(switch_bits)
			  {
				  case 254:  // Mode Auto
				  {
					  if ((changed==0)&& (off ==0))
					  {
							if (semaphore1_get_access())
							{
							  mode = 0;
							  backlight=100;
							  changed=1;
							  //we need the switch in off state
							  // for at least 0.2 seconds
							  off = 2;
								semaphore1_release_access();
							}

						  }
					  break;
				  }
				  case 253:  // Mode ECO, CONF ou HG
				  {
					  if ((changed==0)&& (off ==0))
					  {
							  if (mode==0)
							  {
									if (semaphore1_get_access())
									{
										mode = 1;
										semaphore1_release_access();
									}

							  } else {

									  if (mode <3)
									  {
											if (semaphore1_get_access())
											{
												  mode += 1;
												semaphore1_release_access();
											}

									  }  else {

											if (semaphore1_get_access())
											{
												mode = 1;
												semaphore1_release_access();
											}

									  }
							  }
							if (semaphore1_get_access())
							{
								// switch-on backlight for 10 seconds
									backlight=100;
									changed=1;
									//we need the switch in off state
									// for at least 0.2 seconds
									off = 2;
								semaphore1_release_access();
							}


					  }
					  break;
				  }
				  case 251:  // Temp+
				  {
					  if ((changed==0)&& (off ==0))
					  {
							if (semaphore1_get_access())
							{
								  selection = 0.5;
								  // switch-on backlight for 10 seconds
								  backlight=100;
								  changed=1;
								  //we need the switch in off state
								  // for at least 0.2 seconds
								  off = 2;
								semaphore1_release_access();
							}

					  }

					  break;
				  }
				  case 247: // Temp - HG
				  {
					  if ((changed==0)&& (off ==0))
					  {
							if (semaphore1_get_access())
							{
								  selection =-0.5;
								  changed=1;
								  // switch-on backlight for 10 seconds
								  backlight=100;
								  //we need the switch in off state
								  // for at least 0.2 seconds
								  off = 2;
								semaphore1_release_access();
							}
					  }

					  break;
				  }
				  case 239:  // Mode STOP
				  {
					  if ((changed==0)&& (off ==0))
					  {
							if (semaphore1_get_access())
							{
								  mode = 4;
								  backlight=100;
								  changed=1;
								  off = 2;
							  semaphore1_release_access();
							}

					  }

					  break;
				  }
				  default:
				  {
						if (semaphore1_get_access())
						{
							  if (off!=0)
							  {
								  off--;

							  } else {

								  off=0;
							  }
							semaphore1_release_access();
						}
					  break;

				  }
			  }
	   }
    perror( "Chauffapi: unforeseen exit from keyboard Thread\n" );
    exit(EXIT_FAILURE);

//  pthread_exit(NULL);
//return 0;
}
