/****************************************************************************
 * nxp_bms/BMS_v1/src/main.c
 *
 * BSD 3-Clause License
 * 
 * Copyright 2020 NXP
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <arch/board/board.h>
#include <sched.h>
#include <semaphore.h>
#include "data.h"
#include "cli.h"
#include "ledState.h"
#include "gpio.h"
#include "batManagement.h"
#include "uavcan.h"
#include "sbc.h"
#include "nfc.h"
#include "a1007.h"

//#warning each semaphore used to signal a task needs to call sem_setprotocol(&sem, SEM_PRIO_NONE); after sem_init!!!

/****************************************************************************
 * Defines
 ****************************************************************************/
#define BMS_VERSION_STRING "bms3.4-9.1"
//#define DONT_DO_UAVCAN

//! this macro is used to print a byte value to binary, use as variable to BYTE_TO_BINARY_PATTERN
#ifndef BYTE_TO_BINARY
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
#endif

//! this define is used with macro BYTE_TO_BINARYto print a byte value to binary
#ifndef BYTE_TO_BINARY_PATTERN
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#endif

#define DEFAULT_PRIORITY 	100
#define DEFAULT_STACK_SIZE 	2048
#define DATA_HANDLER_STACK_SIZE 1536 //2048 //1024

#define CHANGED_DATA_ARRAY_ELEMENTS 20

#define CHARGE_DETECTED_MARGIN 0.010 // [A]

//! this define is used to make sure it doesn't keep going in the charge with CB (from relaxation) when done
#define AMOUNT_CB_CHARGE_CYCLES_MAX 5

#define BUTTON_TIME_FOR_DEEP_SLEEP 	5 	// [s]
#define SELF_DISCHARGE_WAIT_TIME 	10	// [s]

/****************************************************************************
 * Types
 ****************************************************************************/
typedef enum{
	SLEEP_VAR,
	CHAR_VAR,
	DISCHAR_VAR
}transitionVars_t;

// the string array for the states
static const char *gStatesArray[] = 
{
	//"INIT", "NORMAL", "CHARGE", "SLEEP", "OCV", "FAULT", "SELF_DISCHARGE", "DEEP_SLEEP"
	FOR_EACH_STATE(GENERATE_STRING)
};

/****************************************************************************
 * private data
 ****************************************************************************/
// make a mutex lock for the main state 
pthread_mutex_t gStateLock;
bool gStateLockInitialized = false;

// make a mutex lock for the charge state
pthread_mutex_t gChargeStateLock;
bool gChargeStateLockInitialized = false;

pthread_mutex_t gTransVarLock;
bool gTransVarInitialized = false;

//! mutex for the state commands
pthread_mutex_t gStateCommandLock;
bool gStateCommandInitialized = false;

sem_t gDataChangedSem;
bool gChangedParameterTaskStarted = false;

parameterKind_t gChangedParametersArr[CHANGED_DATA_ARRAY_ELEMENTS];// = {NONE, NONE, NONE, NONE};
uint32_t gChangedDataArr[CHANGED_DATA_ARRAY_ELEMENTS];// = {NULL, NULL, NULL, NULL};

states_t gCurrentState = INIT;
charge_states_t gCurrentChargeState = CHARGE_START;

static bool gMainLoopStarted = false; 		// to indicate the main loop is started

static bool gDischargeDetected = true;
static bool gChargeDetected = false;
static bool gSleepDetected = false;

static bool gBCCRisingFlank = false; 
static bool gButtonRisingFlank = false; 
static bool gButtonPressFlank = false;

/****************************************************************************
 * private Functions
 ****************************************************************************/
/*!
 * @brief this function will implement the main state machine
 * 
 * @param argc the amount of arguments there are in argv (if the last argument is NULL!)
 * @param argv a character pointer array with the arguments, first is the taskname than the arguments
 */
static int mainTaskFunc(int argc, char *argv[]);

/*!
 * @brief function for a task to handle a parameter change if semaphore is available
 * 
 * @param argc the amount of arguments there are in argv (if the last argument is NULL!)
 * @param argv a character pointer array with the arguments, first is the taskname than the arguments
 */
int handleParamChangeFunc(int argc, char *argv[]);

/*!
 * @brief function to handle a parameter change, will set semaphore available
 * 
 * @param changedParameter the parameter that changed
 * @param newValue the new value that is set
 */
int parameterChangeFunction(parameterKind_t changedParameter, void *newValue);

/*!
 * @brief function that will will be called if there is an overcurrent fault function 
 * 
 * @param none
 */
void overCurrentFaultFunction(void);

/*!
 * @brief function that will will be called to change the LED
 * 
 * @param newColor the new LED color
 * @param BlinkOn true if led should blink
 */
void changeLedColor(LEDColor_t newColor, bool BlinkOn);

/*!
 * @brief function to be called when new data is set
 * 
 */
void newMeasurementsFunction(void);

/*!
 * @brief function that will be called with an interrupt occurs on the BCC pin
 * 
 * @param number the pin number that triggers this ISR
 */
void BCCFaultPinISR(int number);

/*!
 * @brief function that will be called with an interrupt occurs on the button (SBC_WAKE) pin
 * 
 * @param number the pin number that triggers this ISR
 */
void ButtonPinISR(int number);

/*!
 * @brief function that will be called when it needs to process a cli command when the CLI can't do this
 * 
 * @param command 		the command that needs to be processed 
 * @param showCommand 	if the command is CLI_SHOW this will be the show command to set
 * @param value 		if 1 that CLI_SHOW command is enabled, if 0 it is disabled
 */
void processCLICommand(commands_t command, showCommands_t showCommand, uint8_t value);

/*!
 * @brief function that will return the main state, but it will use the mutex 
 * 
 * @return the state
 */
states_t getMainState(void);

/*!
 * @brief function that will set the main state, but it will use the mutex 
 * 
 * @param newState the new state
 */
int setMainState(states_t newState);

/*!
 * @brief function that will return the charge state, but it will use the mutex 
 * 
 * @return the state
 */
charge_states_t getChargeState(void);

/*!
 * @brief function that will set the charge state, but it will use the mutex 
 * 
 * @param newState the new state
 */
int setChargeState(charge_states_t newState);

/*!
 * @brief function that will return one of the transition variables
 * 
 * @param variable the variable to get
 *
 * @return the value of the variable
 */
bool getTransitionVariable(transitionVars_t variable);

/*!
 * @brief function that will set one of the transition variables
 * 
 * @param variable the variable to set
 * @param newValue the new value to set
 *
 * @return 0 if ok
 */
int setTransitionVariable(transitionVars_t variable, bool newValue);

/*!
 * @brief 	function that will set one of the transition variables
 *
 * @param 	setNotGet if true it is used to set the variable, false to get the variable
 * @param 	newValue if setNotGet is true, this is the new value
 *
 * @return 	the state command variable, 0 if none is set, CMD_ERROR if error
 */
stateCommands_t getNSetStateCommandVariable(bool setNotGet, stateCommands_t newValue);
/****************************************************************************
 * main
 ****************************************************************************/
// keep in mind that this program can be called more than once to enter parameters
int bms_main(int argc, char *argv[])
{
	// clear the screen from cursor down
	cli_printf("\e[0J");
	
	// the messages for the tasks
	int lvRetValue;
	int errcode;
	int i;

	if(!gStateLockInitialized)
	{
		cli_printf("BMS version: %s\n", BMS_VERSION_STRING);

		// initialze the mutex
		pthread_mutex_init(&gStateLock, NULL);
		gStateLockInitialized = true;
	}

	if(!gChargeStateLockInitialized)
	{
		// initialze the mutex
		pthread_mutex_init(&gChargeStateLock, NULL);
		gChargeStateLockInitialized = true;
	}

	if(!gTransVarInitialized)
	{
		// initialize the transition variable mutex
		pthread_mutex_init(&gTransVarLock, NULL);
		gTransVarInitialized = true;
	}

	// check if not initialized 
	if(!gStateCommandInitialized)
	{
		// initialize the state command lock
		pthread_mutex_init(&gStateCommandLock, NULL);
		gStateCommandInitialized = true;
	}

	// initialize the functions

	// initialize data structure
	// create the changed parameter task if not started
	if(!gChangedParameterTaskStarted)
	{
		// initialize the changed parameter semaphore
		sem_init(&gDataChangedSem, 0, 0);

		// initialize the arrays
		for(i = 0; i < CHANGED_DATA_ARRAY_ELEMENTS; i++)
		{
			gChangedParametersArr[i] = NONE;
			gChangedDataArr[i] = INT32_MAX;
		}		

		// create the main loop task
		lvRetValue = task_create("changed data handler", DEFAULT_PRIORITY, DATA_HANDLER_STACK_SIZE, handleParamChangeFunc, NULL);
		if (lvRetValue < 0)
		{
			errcode = errno;

			cli_printf("ERROR main: Failed to start changed data handler task: %d\n", errcode);
			return 0;
		}
	}

	// initialzie the data part
	lvRetValue = data_initialize(&parameterChangeFunction);
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize data! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  FLASH\n");
		return lvRetValue;
	}

	// initialize the cli
	cli_initialize(&getMainState, &getChargeState, &processCLICommand);

	// initialize the GPIO
	lvRetValue = gpio_init();
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize gpio! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  GPIO\n");
		return lvRetValue;
	}

	// initialze the SBC
	lvRetValue = sbc_initialize();
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize SBC! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  SBC\n");
		return lvRetValue;
	}

#ifndef DONT_DO_UAVCAN

	// initialize the UAVACAN
	lvRetValue = uavcan_initialize();
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize UAVCAN! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  UAVCAN\n");
		return lvRetValue;
	}

#endif

	// initialize the LED
	lvRetValue = ledState_initialize();
	if(lvRetValue)
	{
		cli_printf("SELF-TEST FAIL: LEDs\n");
		// output to the user
		cli_printf("ERROR main: failed to initialize leds! code %d\n", lvRetValue);
		return lvRetValue;
	}

	// initialize the battery management
	lvRetValue = batManagement_initialize(&overCurrentFaultFunction, 
										  &changeLedColor, &newMeasurementsFunction);
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize batManagement! code %d\n", lvRetValue);
		return lvRetValue;
	}

	// initialze the NFC
	lvRetValue = nfc_initialize();
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize nfc! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  NFC\n");
		return lvRetValue;
	}

	// initialze the A1007
	lvRetValue = a1007_initialize();
	if(lvRetValue)
	{
		// output to the user
		cli_printf("ERROR main: failed to initialize A1007! code %d\n", lvRetValue);
		cli_printf("SELF-TEST FAIL:  A1007\n");
		return lvRetValue;
	}

	// check if a command was given
	if(argc > 1)
	{
		// TODO make parameters only changeable in normal and sleep, (self_discharge, fault)
		
		// start the cli task
		cli_processCommands(argc, argv);

	    // return 
	    return 0;
	}

	// check if the LED needs to blink
	if(!gMainLoopStarted)
	{
		// create the main loop task
		lvRetValue = task_create("mainLoop", DEFAULT_PRIORITY, DEFAULT_STACK_SIZE, mainTaskFunc, NULL);
		if (lvRetValue < 0)
		{
			// get the error
			errcode = errno;

			// error
			cli_printf("BMS_v1_main: ERROR: Failed to start main loop task: %d\n", errcode);
			return 0;
		}
	}

	// return
  	return 0;
}


static int mainTaskFunc(int argc, char *argv[])
{	
	gMainLoopStarted = true;
	BMSFault_t BMSFault;
	int lvRetValue;
	//charge_states_t chargeState = CHARGE_START;
	charge_states_t oldChargeState = CHARGE_COMPLETE;
	struct timespec sampleTime;
	struct timespec buttonPressedTime;
	struct timespec selfDischargeTime;
	struct timespec currentTime;
	int32_t int32tVal;
	float floatVal;
	uint8_t amountOfCBChargeCycles = 0;
	bool onlyOnce = false;
	bool chargeToStorage = false;
	bool boolValue;
	bool deepsleepTimingOn = false;

	// get the variables if the fault happend
	gBCCRisingFlank = gpio_readPin(BCC_FAULT);

	// connect the ISR of the pins
	lvRetValue = gpio_registerISR(BCC_FAULT, BCCFaultPinISR, 1);
	
	// check for errors
	if(lvRetValue)
	{
		cli_printf("main ERROR: GPIO reg 1 went wrong ! %d\n", lvRetValue);

	}

	// make sure that if there was an interrupt it is read
	if(!gBCCRisingFlank && gpio_readPin(BCC_FAULT))
	{
		// set it
		gBCCRisingFlank = true;
	}

	// get the variables if the button press happend
	gButtonRisingFlank = gpio_readPin(SBC_WAKE);
	gButtonPressFlank = !gButtonRisingFlank;

	lvRetValue = gpio_registerISR(SBC_WAKE, ButtonPinISR, 0);

	// make sure that if there was an interrupt it is read
	if(!gButtonRisingFlank && gpio_readPin(SBC_WAKE))
	{
		// set it
		gButtonRisingFlank = true;
		gButtonPressFlank = false;
	}
	
	// check for errors
	if(lvRetValue)
	{
		cli_printf("main ERROR: GPIO reg 0 went wrong ! %d\n", lvRetValue);

	}

	// make the oldState different
	states_t lvOldState = !getMainState();
	// start the task
	cli_printf("BMS main loop!\n");

	// loop in the state machine
	while(1)
	{
		// check if the pin is high and the variable is not high
		if((getMainState() != FAULT) && (getMainState() != INIT) && (getMainState() != CHARGE) &&
		  (!gBCCRisingFlank) && (gpio_readPin(BCC_FAULT)))
		{
			// output to user why
			cli_printf("Rising edge BCC_FAULT not noticed, setting fault now!\n");

			// set the variable high to go through the error handler
			gBCCRisingFlank = true;
		}
		
		// check the button ISR value
		if(gBCCRisingFlank)
		{
			cli_printf("Rising edge BCC_FAULT!\n");
			// check the BCC fault
			batManagement_checkFault(&BMSFault, 0);
			batManagement_checkFault(&BMSFault, 0);

			// // check if a fault occured
			if(BMSFault)
			{
				// change the status flags if needed 
				// check for temperature errors
				if(BMSFault & (BMS_UT | BMS_OT))
				{
					// set the temperature bit
     				data_statusFlagBit(STATUS_TEMP_ERROR_BIT, 1);
				}

				// check for overload bit
				if(BMSFault & STATUS_OVERLOAD_BIT)
				{
					// set the temperature bit
     				data_statusFlagBit(STATUS_TEMP_ERROR_BIT, 1);
				}
				
				// TODO act on every fault, like sleep_oc, cc_overflow, otherfaults

				// check if need to react 
				if(BMSFault & (BMS_CELL_UV + BMS_CELL_OV + BMS_PCB_UV + BMS_PCB_OV + BMS_UT + BMS_OT + BMS_OVER_CURRENT))
				{
					// check for output to the user
					if(BMSFault & BMS_OVER_CURRENT)
					{
						cli_printf("overcurrent detected!\n");
					}
					else if(BMSFault & BMS_CELL_UV)
					{
						cli_printf("cell undervoltage detected!\n");
					}
					else if(BMSFault & BMS_CELL_OV)
					{
						cli_printf("cell overvoltage detected!\n");
					}
					else if(BMSFault & BMS_UT)
					{
						cli_printf("BMS undertemperature detected!\n");
					}
					else if(BMSFault & BMS_OT)
					{
						cli_printf("BMS overtemperature detected!\n");
					}

					// check for an cell overvoltage in the charge with CB state
					if(((BMSFault & (BMS_CELL_OV)) == BMS_CELL_OV) &&
						(getMainState() == CHARGE) && (getChargeState() == CHARGE_CB))
					{
						// set the relaxation state
						setChargeState(RELAXATION);
						cli_printf("going to relaxation state!\n");
					}
					else
					{
						// go to the FAULT state
						setMainState(FAULT);
					}
					// TODO with cell_UV go to deepsleep somehow
				}
				// check if there is a CC overflow
				if(BMSFault & BMS_CC_OVERFLOW)
				{
					// read and reset the CC registers by calculating a new remaining charge
					lvRetValue = batManagement_calcRemaningCharge();

					// check for errors
					if(lvRetValue)
					{
						// output to user
						cli_printf("main ERROR: could not reset CC register when overflow: %d\n", lvRetValue);
					}
				}
				#warning when an other fault happens it will trigger this function because the fault pin is high, when ignored, it wont trigger again when a real fault occurs
			}

			// to make sure you only enter this once
			gBCCRisingFlank = false;
		}

		// check for a buttonpress
		if(gButtonPressFlank)
		{
			if(getMainState() == SLEEP || getMainState() == NORMAL)
			{
				// get the time 
				if(clock_gettime(CLOCK_REALTIME, &buttonPressedTime) == -1)
				{
					cli_printf("main ERROR: failed to get buttonPressedTime!\n");
				}

				// set the variable true
				deepsleepTimingOn = true;
			}
			
			// make sure it will only do this once
			gButtonPressFlank = false;
		}

		// check the ISR value
		if(gButtonRisingFlank || getNSetStateCommandVariable(false, CMD_ERROR) == CMD_RESET)
		{
			// check if it is in the FAULT or SLEEP state
			if(getMainState() == FAULT || getMainState() == SLEEP || getMainState() == SELF_DISCHARGE)
			{
				// if self discharge check if the time passed 
				if(getMainState() == SELF_DISCHARGE)
				{
					// get the current time 
					if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
					{
						cli_printf("main ERROR: failed to get currentTime in gButtonRisingFlank!\n");
					}

					// check if the right amount of time has passed 
					if(((selfDischargeTime.tv_sec + SELF_DISCHARGE_WAIT_TIME) < currentTime.tv_sec) || 
					  (((selfDischargeTime.tv_sec + SELF_DISCHARGE_WAIT_TIME) == currentTime.tv_sec) && selfDischargeTime.tv_nsec <= currentTime.tv_nsec))
					{
						// go to the INIT state
						setMainState(INIT);
					}
				}
				else
				{
					// go the the init state with a button press
					setMainState(INIT);
				}
			}

			//cli_printf("INT!!!\n");
			gButtonRisingFlank = false;

			// reset the command variable
			getNSetStateCommandVariable(true, CMD_NONE);

			// set the variable false
			deepsleepTimingOn = false;
		}
		

		// check the state variable
		switch(getMainState())
		{
			case INIT:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// set the LED to green 
					ledState_setLedColor(GREEN, LED_BLINK_OFF);

					// set the AFE mode to normal
					batManagement_setAFEMode(AFE_NORMAL);

					// check configuration and reset faults
					batManagement_checkAFE(&BMSFault, true);

					// check the fault variable
					if(BMSFault)
					{
						// get the time
						if(clock_gettime(CLOCK_REALTIME, &sampleTime) == -1)
						{
							cli_printf("main ERROR: failed to get sampleTime!\n");
						}

						// wait until the pin is low again or 1sec timeout
						do
						{
							// get the time to check for an overflow
							if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
							{
								cli_printf("main ERROR: failed to get currentTime!\n");
							}

							// sleep a little bit
							usleep(1);

						} while (gpio_readPin(BCC_FAULT) && (((currentTime.tv_sec - sampleTime.tv_sec) >= 1) 
								&& (currentTime.tv_nsec >= sampleTime.tv_nsec)));

						// check if timeout happend
						if(gpio_readPin(BCC_FAULT))
						{
							// set the fault pin flank true to go to fault state
							gBCCRisingFlank = true;
						}

						// start a manual measurement and wait until the measurement is done
						lvRetValue = batManagement_doMeasurement();

						// check for error
						if(lvRetValue)
						{
							// wait 50ms to make sure the pin should be high again
							usleep(50*1000UL);
						}

						// check if the interrupt happend
						if(gBCCRisingFlank)
						{
							// break to skip the rest of the init (don't close the switch)
							break;
						}
					}

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// start a manual measurement and wait until the measurement is done
					lvRetValue = batManagement_doMeasurement();

					// check for error
					if(lvRetValue)
					{
						// set the fault pin flank true to go to fault state
						gBCCRisingFlank = true;
						break;
					}

					// check if the inserted n_cells is ok
				   	lvRetValue = batManagement_checkNCells(&boolValue);

				   	// check for errors
					if(lvRetValue != 0)
				    {
				    	// inform user
				      	cli_printf("batManagement: ERROR: Failed get n_cells ok: %d\n", lvRetValue);
				      	
				      	// set the fault pin flank true to go to fault state
						gBCCRisingFlank = true;
						break;
				    }

				    // check if the output is low
				    if(boolValue != 1)
				    {
				    	cli_printf("batManagement: ERROR: wrong n_cells!\n");
				    	cli_printf("Please set the correct cells! using \"bms set n_cells x\"\n");

				      	// go to the FAULT state
				      	setMainState(FAULT);
						break;
				    }

					// close the gate
					if(batManagement_setGatePower(GATE_CLOSE) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
					}

					// turn on the measurements 
					batManagement_updateMeasurementsOn(true);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					cli_printf("init mode\n");
				}

				// check for discharge
				if(!getTransitionVariable(CHAR_VAR))
				{
					// go to the normal state
					setMainState(NORMAL);
				}
				else
				{
					// go to the charge state
					setMainState(CHARGE);
				}

			break;
			case NORMAL:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{

					// save the old value
					lvOldState = getMainState();

					// set SBC V1_ON_V2_ON (V1 = 3.3V reg and V2 is CAN tranceiver supply)

					// BCC normal mode

					// turn on the gate 
					if(batManagement_setGatePower(GATE_CLOSE) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
					}

					// turn on the measurements if not on
					batManagement_updateMeasurementsOn(true);

					// reset consumed power 
					floatVal = 0;

					// set the consumed power to 0
					if(data_setParameter(E_USED, &floatVal))
					{
						cli_printf("main ERROR: couldn't reset consumed power!\n");
					}

					// get the state of charge 
				    if(data_getParameter(S_CHARGE, &int32tVal, NULL) == NULL)
				    {
				       cli_printf("main ERROR: getting state of charge went wrong!\n");
				       int32tVal = S_CHARGE_DEFAULT;
				       //return lvRetValue;
				    } 

					// set the value to calculate the state indication
					ledState_calcStateIndication((uint8_t)int32tVal);

					// set the LED to green blinking
					ledState_setLedColor(GREEN, LED_BLINK_ON);

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					cli_printf("normal mode\n");

					// TODO CLI and NFC is allowed

					// TODO enable diagnostics
				}

				// check if the button is pressed
				if(deepsleepTimingOn)
				{
					// check if the current stays low 
					if(!getTransitionVariable(DISCHAR_VAR))
					{
						// get the current time 
						if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
						{
							cli_printf("main ERROR: failed to get currentTime in sleep!\n");
						}

						// check if the right amount of time has passed 
						if(((buttonPressedTime.tv_sec + BUTTON_TIME_FOR_DEEP_SLEEP) < currentTime.tv_sec) || 
						  (((buttonPressedTime.tv_sec + BUTTON_TIME_FOR_DEEP_SLEEP) == currentTime.tv_sec) && buttonPressedTime.tv_nsec <= currentTime.tv_nsec))
						{
							// go to the deep sleep state
							setMainState(SELF_DISCHARGE);
						}
					}
					else
					{
						// set the variable false
						deepsleepTimingOn = 0;
					}
				}

				//check for charge 
				if(getTransitionVariable(CHAR_VAR))
				{
					// go to the charge state 
					setMainState(CHARGE);
				}

				// check for sleep
				if(getTransitionVariable(SLEEP_VAR) || getNSetStateCommandVariable(false, CMD_ERROR) == CMD_GO_2_SLEEP)
				{
					setMainState(SLEEP);
				}


			break;
			case CHARGE:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// set SBC V1_ON_V2_ON

					// BCC normal mode

					// turn on the gate 
					if(batManagement_setGatePower(GATE_CLOSE) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
					}

					// turn on the measurements if not on
					batManagement_updateMeasurementsOn(true);

					// set the charge mode
					batManagement_startCharging(true);

					// set the LED to blue
					ledState_setLedColor(BLUE, LED_BLINK_OFF);

					cli_printf("charge mode\n");

					// set the charge state to the first state
					setChargeState(CHARGE_START);

					// set the old charge state to the last state
					oldChargeState = CHARGE_COMPLETE;

					// reset the variable for the CB charge cycles
					amountOfCBChargeCycles = 0;

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					// reset the charge to storage variable
					batManagement_SetNReadChargeToStorage(true, 0);

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the variable to false
					chargeToStorage = false;

					// TODO CLI and NFC is allowed

					// TODO enable diagnostics
				}

				// check if the charge state changed
				if(getChargeState() != oldChargeState)
				{
					switch(getChargeState())
					{
						// in case of the beginning
						case CHARGE_START:

							// set the end of charge variable to 1 to make sure it will only check for voltage
							batManagement_SetNReadEndOfCBCharge(true, 1);

							// start the charging timer
							// check the time the charging begins
							// save the time
							if(clock_gettime(CLOCK_REALTIME, &sampleTime) == -1)
							{
								cli_printf("main ERROR: failed to get sampleTime!\n");
							}

							cli_printf("charge start %ds %dms\n", sampleTime.tv_sec, sampleTime.tv_nsec/1000000);

						break;

						// in case of charging with balancing
						case CHARGE_CB:

							// turn off the gate 
							if(batManagement_setGatePower(GATE_CLOSE) != 0)
							{
								cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
							}

							// enable cell balancing
							batManagement_setBalancing(true);

							// set the end of charge variable to 0 to check for current and voltage
							batManagement_SetNReadEndOfCBCharge(true, 0);

							// save the time
							if(clock_gettime(CLOCK_REALTIME, &sampleTime) == -1)
							{
								cli_printf("main ERROR: failed to get sampleTime!\n");
							}

							cli_printf("charge with CB %ds %dms\n", sampleTime.tv_sec, sampleTime.tv_nsec/1000000);

							// increase the counter
							amountOfCBChargeCycles++;

							// make sure it will only output CB done once
							boolValue = true;

							// get the n-charges-full value and increment it once 
						    if(data_getParameter(N_CHARGES, &int32tVal, NULL) == NULL)
						    {
						       cli_printf("main ERROR: getting n-charges went wrong!\n");
						       int32tVal = N_CHARGES_DEFAULT;
						       //return lvRetValue;
						    } 

						    // increament and limit it 
						    int32tVal =  ++int32tVal & UINT16_MAX;

						    if(data_setParameter(N_CHARGES, &int32tVal))
							{
								cli_printf("main ERROR: couldn't set n-charges!\n");
							}

							//cli_printf("charge wth CB %d\n", amountOfCBChargeCycles);

						break;

						// in case of relaxing
						case RELAXATION:

							// set the power switches open
							// turn off the gate 
							if(batManagement_setGatePower(GATE_OPEN) != 0)
							{
								cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
							}

							// start the relax time
							// save the time
							if(clock_gettime(CLOCK_REALTIME, &sampleTime) == -1)
							{
								cli_printf("main ERROR: failed to get sampleTime!\n");
							}

							cli_printf("charge RELAXATION %ds %dms\n", sampleTime.tv_sec, sampleTime.tv_nsec/1000000);

							// make sure it will only output CB done once
							boolValue = true;

							// make sure it will output the end time once
							onlyOnce = true;

							//cli_printf("charge relaxing %ds %dms\n", sampleTime.tv_sec, sampleTime.tv_nsec/1000000);

						break;

						// in case the charging is complete
						case CHARGE_COMPLETE:

							// save the time
							if(clock_gettime(CLOCK_REALTIME, &sampleTime) == -1)
							{
								cli_printf("main ERROR: failed to get sampleTime!\n");
							}

							// set the LED to green 
							ledState_setLedColor(GREEN, LED_BLINK_OFF);

							// turn off cell balancing function
							batManagement_setBalancing(false);

							// save the remaining capacity in full charge capacity
							if(batManagement_saveFullChargeCap())
							{
								cli_printf("main ERROR: failed to set FCC!\n");
							}

							cli_printf("charge complete %ds %dms\n", sampleTime.tv_sec, sampleTime.tv_nsec/1000000);

							// get the n-charges-full value and increment it once 
						    if(data_getParameter(N_CHARGES_FULL, &int32tVal, NULL) == NULL)
						    {
						       cli_printf("main ERROR: getting n-charges-full went wrong!\n");
						       int32tVal = N_CHARGES_FULL_DEFAULT;
						    } 

						    // increament and limit it 
						    int32tVal =  ++int32tVal & UINT16_MAX;

						    // set the incremented one
						    if(data_setParameter(N_CHARGES_FULL, &int32tVal))
							{
								cli_printf("main ERROR: couldn't set n-charges-full!\n");
							}						  

						break;
					}

					// save the state, so it wont be entered again
					oldChargeState = getChargeState();
				}

				// check for charge state transistions				
				switch(getChargeState())
				{
					// in case of the beginning
					case CHARGE_START:

						// if the cell is full the state needs to change 
						if((batManagement_SetNReadEndOfCBCharge(false, 0) & 2) == 2)
						{
							// make sure it doens't keep checking 
							batManagement_SetNReadEndOfCBCharge(true, 3);

							// set the next charge state
							setChargeState(CHARGE_CB);
						}

						// get the CB begin time
					    if(data_getParameter(T_CB_DELAY, &int32tVal, NULL) == NULL)
					    {
					       cli_printf("main ERROR: getting CB delay went wrong!\n");
					       int32tVal = T_CB_DELAY_DEFAULT;
					    } 

					    // make sure it is uint8
					    int32tVal &= UINT8_MAX;

						// start the charging timer
						// check the current time
						if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
						{
							cli_printf("main ERROR: failed to get currentTime!\n");
						}

						// check if the charge time ended and the charge is begon
						if((((sampleTime.tv_sec + int32tVal) == currentTime.tv_sec) && 
						  (sampleTime.tv_nsec <= currentTime.tv_nsec)) ||
						  ((sampleTime.tv_sec + int32tVal) < currentTime.tv_sec))
						{
							//cli_printf("ended charge start %ds %dms\n", currentTime.tv_sec, currentTime.tv_nsec/1000000);
							// set the next charge state
							setChargeState(CHARGE_CB);
						}

					break;

					// in case of charging with balancing
					case CHARGE_CB:

						// check current and cell voltages
						if(batManagement_SetNReadEndOfCBCharge(false, 0))
						{
							// make sure it doens't keep checking 
							batManagement_SetNReadEndOfCBCharge(true, 3);

							// go to the relaxation state
							setChargeState(RELAXATION);
						}

						// check if cell balancing is done to change the led color
						if(!batManagement_checkBalancing())
						{
							// set the LED to blue
							ledState_setLedColor(BLUE, LED_BLINK_OFF);

							// check if the output has been done
							if(boolValue)
							{

								// output to the user
								cli_printf("CB is done\n");

								// make sure it will only do this once 
								boolValue = false;
							}
						}

					break;

					// in case of relaxing
					case RELAXATION:

						// check if the charger is removed
						if(!batManagement_checkOutputVoltageActive())
						{
							// go to the sleep state by setting the transion variable true
							setTransitionVariable(SLEEP_VAR, true);
				
							// escape the transition switch
							break;
						}

						// check for CB is done
						if(!batManagement_checkBalancing())
						{
							// set the LED to blue
							ledState_setLedColor(BLUE, LED_BLINK_OFF);

							// check if the output has been done
							if(boolValue)
							{
								// output to the user
								cli_printf("CB is done\n");

								// make sure it will only do this once 
								boolValue = false;
							}

							// check for timeout 
							// get the relax time
						    if(data_getParameter(T_CHARGE_RELAX, &int32tVal, NULL) == NULL)
						    {
						       cli_printf("main ERROR: getting relax time went wrong!\n");
						       int32tVal = T_CHARGE_RELAX_DEFAULT;
						    } 

						    // make sure it is uint16
						    int32tVal &= UINT16_MAX;

							// check the current time
							if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
							{
								cli_printf("main ERROR: failed to get currentTime!\n");
							}

							// check if the charge time ended and the charge is begon
							if((((sampleTime.tv_sec + int32tVal) == currentTime.tv_sec) && 
							  (sampleTime.tv_nsec <= currentTime.tv_nsec)) ||
							  ((sampleTime.tv_sec + int32tVal) < currentTime.tv_sec))
							{
								if(onlyOnce)
								{
									onlyOnce = false;
									cli_printf("ended relaxing! %ds %dms\n", currentTime.tv_sec, currentTime.tv_nsec/1000000);
								}

								// get the cell margin in mv
								if(data_getParameter(V_CELL_MARGIN, &int32tVal, NULL) == NULL)
							    {
							       cli_printf("main ERROR: getting cell margin went wrong!\n");
							       int32tVal = V_CELL_MARGIN_DEFAULT;
							    } 

							    // make sure it is uint8
					    		int32tVal &= UINT8_MAX;

							    // get the cell over voltage
							    if(data_getParameter(V_CELL_OV, &floatVal, NULL) == NULL)
							    {
							       cli_printf("main ERROR: getting cell over voltage went wrong!\n");
							       floatVal = V_CELL_OV_DEFAULT;
							    } 

								// check check if highest cell voltage is smaller than the cell_ov - margin
								if((amountOfCBChargeCycles < AMOUNT_CB_CHARGE_CYCLES_MAX) && 
								  (batManagement_getHighestCellV() < 
								  (floatVal - ((float)(int32tVal)/1000))))
								{
									// go back to charge with CB
									setChargeState(CHARGE_CB);
								} 
								else
								{
									// check if 5 times has passed and that is why it transitioned
									if(amountOfCBChargeCycles >= AMOUNT_CB_CHARGE_CYCLES_MAX)
									{
										// send the message to the user 
										cli_printf("%d charge cycles done, skipping voltage requirement! highest cell: %.3fV\n", amountOfCBChargeCycles, batManagement_getHighestCellV());
									}
									// go to charging complete
									setChargeState(CHARGE_COMPLETE);
								}
							}
						}

					break;

					// in case the charging is complete
					case CHARGE_COMPLETE:

						// check if the charger is removed
						if(!batManagement_checkOutputVoltageActive())
						{
							// go to the sleep state by setting the transion variable true
							setTransitionVariable(SLEEP_VAR, true);
						}

						// check if charging to storage is on
						if(chargeToStorage)
						{
							// go to the self discharge state
							setMainState(SELF_DISCHARGE);
						}

					break;
				}

				// check for discharge
				if(getTransitionVariable(DISCHAR_VAR))
				{
					// go to the normal state
					setMainState(NORMAL);
				}
				// check for sleep 
				else if(getTransitionVariable(SLEEP_VAR))
				{
					// go to the sleep state
					setMainState(SLEEP);
				}

				// check for deep_sleep
				if((!chargeToStorage) && (getNSetStateCommandVariable(false, CMD_ERROR) == CMD_GO_2_DEEPSLEEP))
				{
					// get the storage voltage
				    if(data_getParameter(V_STORAGE, &floatVal, NULL) == NULL)
				    {
				       cli_printf("main ERROR: getting storage voltage went wrong!\n");
				       floatVal = V_STORAGE_DEFAULT;
				    } 

					// check if the highest voltage is higher than the storage voltage
					if(batManagement_getLowestCellV() >= floatVal)
					{
						// go to the self_discharge state
						setMainState(SELF_DISCHARGE);
					}
					else
					{
						// set the charge to storage variable
						batManagement_SetNReadChargeToStorage(true, 1);

						// set the variable to charge to the storage voltage
						chargeToStorage = true;

						cli_printf("charging until storage voltage\n");
					}
				}
				else if(chargeToStorage && (getNSetStateCommandVariable(false, CMD_ERROR) != CMD_GO_2_DEEPSLEEP))
				{
					// reset the charge to storage variable
					batManagement_SetNReadChargeToStorage(true, 0);

					// set the variable to false
					chargeToStorage = false;
				}

				// check if the state changes
				if(getMainState() != CHARGE)
				{
					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// make sure it doens't keep checking 
					batManagement_SetNReadEndOfCBCharge(true, 3);
				}

			break;
			case SLEEP:

				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// TODO set SBC V1_ON_V2_OFF (CAN tranceiver off)

					// TODO don't send data over UAVCAN

					// BCC sleep meas mode

					// turn on the gate 
					if(batManagement_setGatePower(GATE_CLOSE) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
					}

					// turn the LED off
					ledState_setLedColor(OFF, LED_BLINK_OFF);

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					cli_printf("sleep mode\n");

					// TODO enable wake for NFC

					// MCU sleep mode
				}

				// check for current
				if(!getTransitionVariable(SLEEP_VAR))
				{
					// go to the init state
					setMainState(INIT);
				}

				// check if the go to sleep command has been given
				if(getNSetStateCommandVariable(false, CMD_ERROR) == CMD_GO_2_DEEPSLEEP)
				{
					// go to the deep sleep state
					setMainState(SELF_DISCHARGE);
				}
				else if(getNSetStateCommandVariable(false, CMD_ERROR) == CMD_WAKE)
				{
					// go to the init state
					setMainState(INIT);
				}

				// check if the user is pressing the button
				if(deepsleepTimingOn)
				{
					// get the current time 
					if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
					{
						cli_printf("main ERROR: failed to get currentTime in sleep!\n");
					}

					// check if the right amount of time has passed 
					if(((buttonPressedTime.tv_sec + BUTTON_TIME_FOR_DEEP_SLEEP) < currentTime.tv_sec) || 
					  (((buttonPressedTime.tv_sec + BUTTON_TIME_FOR_DEEP_SLEEP) == currentTime.tv_sec) && buttonPressedTime.tv_nsec <= currentTime.tv_nsec))
					{
						// go to the deep sleep state
						setMainState(SELF_DISCHARGE);
					}

				}
				// TODO add other transitions (to the OCV state)

			break;
			case OCV:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();
					// set the leds to be green blinking, wake up 
					ledState_setLedColor(GREEN, LED_BLINK_OFF);

					cli_printf("OCV mode\n");

					// TODO look at look up table to set new remaining charge

					// TODO add state
				}

				// TODO add transitions

			break;
			case FAULT:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// turn off the gate 
					if(batManagement_setGatePower(GATE_OPEN) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to close gate\n");

						// set the LED to red
						ledState_setLedColor(RED, LED_BLINK_OFF);
					}
					else
					{
						
						// set SBC V1_ON_V2_ON

						// BCC normal mode

						// set the LED to red blinking
						ledState_setLedColor(RED, LED_BLINK_ON);
					}

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					cli_printf("FAULT mode\n");
				
					// TODO add state things to start a timer on certain faults

				}

				// transitions are done from the button press check part
				
			break;
			case SELF_DISCHARGE:
				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// check if the self discharge should be done

					// get the self discharge enable parameter
					if(data_getParameter(SELF_DISCHARGE_ENABLE, &int32tVal, NULL) == NULL)
				    {
				       cli_printf("main ERROR: getting self discharge var went wrong!\n");
				       int32tVal = SELF_DISCHARGE_ENABLE_DEFAULT;
				    } 

				    // make sure it is a bool
				    int32tVal &= 1;

				    // check if it should be done
				    if(!int32tVal)
				    {
				    	// go to the deepsleep state
						setMainState(DEEP_SLEEP);

						break;
				    }

				    // get the self discharge start time
				    if(clock_gettime(CLOCK_REALTIME, &selfDischargeTime) == -1)
					{
						cli_printf("main ERROR: failed to get selfDischargeTime!\n");
					} 

			    	// set SBC V1_ON_V2_ON

					// BCC normal mode
				
					// turn off the gate
					if(batManagement_setGatePower(GATE_OPEN) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to open gate\n");
					}

					// set the LED to magenta(purple) blinking
					ledState_setLedColor(BLUE, LED_BLINK_ON);

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// wait until the balancing is done
					while(batManagement_checkBalancing())
					{
						usleep(1);
					}

					// set self discharge on
					batManagement_selfDischarge(true);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					cli_printf("SELF_DISCHARGE mode\n");
				}

				// get the self discharge enable parameter
				if(data_getParameter(SELF_DISCHARGE_ENABLE, &int32tVal, NULL) == NULL)
			    {
			       cli_printf("main ERROR: getting self discharge var went wrong!\n");
			       int32tVal = SELF_DISCHARGE_ENABLE_DEFAULT;
			    } 

			    // make sure it is a bool
			    int32tVal &= 1;

			    // check if it should be done
			    if(!int32tVal)
			    {
			    	// go to the deepsleep state
					setMainState(DEEP_SLEEP);
			    }

				// check if cell balancing is done 
				if(!batManagement_checkBalancing())
				{
					// go to the deepsleep state
					setMainState(DEEP_SLEEP);
				}

				// check if the go to sleep command has been given
				if(getNSetStateCommandVariable(false, CMD_ERROR) == CMD_GO_2_SLEEP)
				{
					// go to the sleep state
					setMainState(SLEEP);
				}
				
			break;
			case DEEP_SLEEP:

				// check if the state has changed to not do this everytime
				if(getMainState() != lvOldState)
				{
					// save the old value
					lvOldState = getMainState();

					// set the LED to white
					ledState_setLedColor(WHITE, LED_BLINK_OFF);

					// turn off the gate 
					if(batManagement_setGatePower(GATE_OPEN) != 0)
					{
						cli_printf("BMS_v1_main: ERROR: Failed to close gate\n");
					}

					// disable cell balancing
					batManagement_setBalancing(false);

					// set the charge mode
					batManagement_startCharging(false);

					// turn off the measurements 
					batManagement_updateMeasurementsOn(false);

					// reset the command variable
					getNSetStateCommandVariable(true, CMD_NONE);

					// output to the user
					cli_printf("DEEP_SLEEP mode\n");
				}

				// break until the button is released
				if(deepsleepTimingOn)
				{
					break;
				}

				usleep(1000*1000);

				// BCC sleep 
				// set the AFE mode to normal
				cli_printf("Setting the BCC to sleep!\n");
				batManagement_setAFEMode(AFE_SLEEP);
				// wakeup with CSB from low to high

				cli_printf("Saving parameters before sleep\n");

				// save the variables
				lvRetValue = data_saveParameters();

				// turn the LED off
				ledState_setLedColor(OFF, LED_BLINK_OFF);

				//cli_printf("setting SBC to standby!\n");
				cli_printf("setting SBC to sleep!\n");

				// MCU off mode
				//lvRetValue = sbc_setSbcMode(SBC_STANDBY);
				lvRetValue = sbc_setSbcMode(SBC_SLEEP);

				// go to the init state
				setMainState(INIT);

			break;
		}

		// sleep for a little time
		usleep(1);
	}

	return 0;
}

/*!
 * @brief function to handle a parameter change
 * 
 * @param argc the amount of arguments there are in argv (if the last argument is NULL!)
 * @param argv a character pointer array with the arguments, first is the taskname than the arguments
 */
int handleParamChangeFunc(int argc, char *argv[])
{
	gChangedParameterTaskStarted = true;
	int lvRetValue = 0;
	static uint8_t handleChangedDataArrayIndex = 0;
	static bool timeOutTimeStarted = false;
	static bool chargeTimeStarted = false;
	static struct timespec timeOutBeginTime;
	static struct timespec chargeBeginTime;

	struct timespec currentTime;

	static uint8_t sleepCurrent = 0;
	static uint16_t timeoutTime = 0;
	static uint8_t chargeDetectTime = 0;

	void* voidPointer;
	float currentmA;//, lvFloatValue1, lvFloatValue2;
	uint32_t intValue = 0;

	// get the sleepcurrent, timeout time and charge detect time
	if(data_getParameter(I_SLEEP_OC, &sleepCurrent, NULL) == NULL)
    {
    	sleepCurrent = I_SLEEP_OC_DEFAULT;
       	cli_printf("handleParamChangeFunc ERROR: getting sleep current went wrong!\n");
    }
    if(data_getParameter(T_BMS_TIMEOUT, &timeoutTime, NULL) == NULL)
    {
    	timeoutTime = T_BMS_TIMEOUT_DEFAULT;
		cli_printf("handleParamChangeFunc ERROR: getting timeoutTime went wrong!\n");
    } 
    if(data_getParameter(T_CHARGE_DETECT, &chargeDetectTime, NULL) == NULL)
    {
    	chargeDetectTime = T_CHARGE_DETECT_DEFAULT;
		cli_printf("handleParamChangeFunc ERROR: getting chargeDetectTime went wrong!\n");
    } 

    //cli_printf("sleepcur: %dmA, timeoutT: %ds, chargeDetectTime %ds\n", sleepCurrent, timeoutTime, chargeDetectTime);

    // loop, use the semaphore to trigger a new cycle
	while(1)
	{
		// wait for the semaphore
		// semaphore is free if a data is changed
		lvRetValue = sem_wait(&gDataChangedSem);
		if (lvRetValue != 0)
		{
			cli_printf("handleParamChangeFunc: ERROR sem_wait failed\n");
		}

		// check which parameter changed
		switch(gChangedParametersArr[handleChangedDataArrayIndex])
		{
			// check if the current changed
			case I_BATT:

				voidPointer = &gChangedDataArr[handleChangedDataArrayIndex];

				// get the current
				currentmA = *(float*)voidPointer;
				currentmA *= 1000;
				
				// check for a discharge
				if((currentmA) <= -sleepCurrent)
				{
					//cli_printf("discharge! %fmA\n", (currentmA));
					// reset the charge variables
					setTransitionVariable(CHAR_VAR, false);
					chargeTimeStarted = false;

					// reset the sleep variable
					setTransitionVariable(SLEEP_VAR, false);

					// reset the timeoutstarted variable 
					timeOutTimeStarted = false;

					// set the discharge variable
					setTransitionVariable(DISCHAR_VAR, true);

				}
				// check for a charge
				else if((currentmA) >= sleepCurrent)
				{
					//cli_printf("charge! %fmA\n", (currentmA));
					// reset the timeoutstarted variable 
					timeOutTimeStarted = false;

					// reset the sleep variable
					setTransitionVariable(SLEEP_VAR, false);

					// reset the discharge variable
					setTransitionVariable(DISCHAR_VAR, false);

					// check if the battery is in the normal state
					if(getMainState() == NORMAL)
					{
						// check if the charge time has started
						if(!chargeTimeStarted)
						{
							// save the time
							if(clock_gettime(CLOCK_REALTIME, &chargeBeginTime) == -1)
							{
								cli_printf("main ERROR: failed to get chargeBeginTime!\n");
							}

							//cli_printf("chargebegintime: %ds %dus\n", chargeBeginTime.tv_sec, chargeBeginTime.tv_nsec/1000);

							// set the variable 
							chargeTimeStarted = true;
						}
						else if(chargeTimeStarted && !getTransitionVariable(CHAR_VAR))
						{
							// check if the time passed
							// check the current time
							if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
							{
								cli_printf("main ERROR: failed to get chargeBeginTime!\n");
							}

							// check if the time passed
							if(((chargeBeginTime.tv_sec + chargeDetectTime) < currentTime.tv_sec) || 
							  (((chargeBeginTime.tv_sec + chargeDetectTime) == currentTime.tv_sec) && chargeBeginTime.tv_nsec <= currentTime.tv_nsec))
							{
								//cli_printf("chargeEndTime: %ds %dus\n", currentTime.tv_sec, currentTime.tv_nsec/1000);
								// set the variable 
								setTransitionVariable(CHAR_VAR, true);
							}
						}
					}

					// if not in normal state
					else
					{
						// reset the variable 
						chargeTimeStarted = false;
						setTransitionVariable(CHAR_VAR, false);
					}

				}
				// check for a sleep current if abs(i-batt) < i-sleep-oc  
				else
				{
					//cli_printf("sleep! %fmA\n", (currentmA));
					// reset the charge variables
					//gChargeDetected = false;
					chargeTimeStarted = false;

					// reset the discharge variable
					setTransitionVariable(DISCHAR_VAR, false);

					// check if the battery is in the normal state
					if(getMainState() == NORMAL)
					{
						// check if the timeout time has started
						if(!timeOutTimeStarted)
						{
							// save the time
							if(clock_gettime(CLOCK_REALTIME, &timeOutBeginTime) == -1)
							{
								cli_printf("main ERROR: failed to get timeoutBeginTime!\n");
							}

							//cli_printf("sleepbegintime: %ds %dus\n", timeOutBeginTime.tv_sec, timeOutBeginTime.tv_nsec/1000);

							// set the variable 
							timeOutTimeStarted = true;
						}
						else if(timeOutTimeStarted && !getTransitionVariable(SLEEP_VAR))
						{
							// check if the time passed
							// check the current time
							if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1)
							{
								cli_printf("main ERROR: failed to get timeoutBeginTime!\n");
							}

							// check if the time passed
							if(((timeOutBeginTime.tv_sec + timeoutTime) < currentTime.tv_sec) || 
							  (((timeOutBeginTime.tv_sec + timeoutTime) == currentTime.tv_sec) && timeOutBeginTime.tv_nsec <= currentTime.tv_nsec))
							{

								//cli_printf("sleepEndtime: %ds %dus\n", currentTime.tv_sec, currentTime.tv_nsec/1000);
								// set the variable 
								setTransitionVariable(SLEEP_VAR, true);
							}
						}
					}
					// // when in the charge mode, make sure to transition to sleep mode if the charge current is 0 or less
					else if(getMainState() == CHARGE)
					{
						// check if the current is 0 or less (no charging / charger disconnected) 
						// but only in the charge states where the gate is closed
						if((currentmA <= 0) && 
						((getChargeState() == CHARGE_START) || (getChargeState() == CHARGE_CB)))
						{
							// check if the gate is already closed (comming from relaxation)
							// this variable is set to 0 when the charge_CB state has entered and 
							// after the gate is closed, if it isn't 0 is should sleep anyway.
							if((batManagement_SetNReadEndOfCBCharge(false, 0) == 0) || (getChargeState() != CHARGE_CB))
							{
								// set the sleep variable to go to the sleep state if the charger 
								/// is disconnected 
								setTransitionVariable(SLEEP_VAR, true);
								timeOutTimeStarted = true;
								cli_printf("No charge current: %.3fA <= 0A\n", currentmA/1000);
							}
						}
						else
						{
							// reset the sleep variable 
							timeOutTimeStarted = false;
							setTransitionVariable(SLEEP_VAR, false);
						}
					}
					// make sure to reset the sleep variable if in any other state 
					else if(getMainState() != SLEEP)
					{
						// set the variable 
						timeOutTimeStarted = false;
						setTransitionVariable(SLEEP_VAR, false);
					}
				}
				// I_BATT
			break;
			case T_BMS_TIMEOUT:
				// save the value 
				timeoutTime = (uint16_t)gChangedDataArr[handleChangedDataArrayIndex] & UINT16_MAX;

			break;
			case I_SLEEP_OC:
				// save the value 
				sleepCurrent = (uint8_t)gChangedDataArr[handleChangedDataArrayIndex] & UINT8_MAX;
			break;

			case T_CHARGE_DETECT:
				chargeDetectTime = (uint8_t)gChangedDataArr[handleChangedDataArrayIndex] & UINT8_MAX;
			break;

			// in case of the state of charge
			case S_CHARGE:
				// get the state of charge
				voidPointer = &gChangedDataArr[handleChangedDataArrayIndex];
				intValue = *(uint32_t*)voidPointer; 
				intValue &= UINT8_MAX;

				// call the funtion set the led blink pattern
				ledState_calcStateIndication((uint8_t)intValue);

			break;

			default:
			break;
		}

		// handle the change in the batManagmenet part
		if(batManagement_changedParameter(gChangedParametersArr[handleChangedDataArrayIndex], (void*)&gChangedDataArr[handleChangedDataArrayIndex]))
		{
			cli_printf("main ERROR: batManagement_changedParameter went wrong! with %d\n", gChangedParametersArr[handleChangedDataArrayIndex]);
		}

		// set the array to none to indicate it was handled
		gChangedParametersArr[handleChangedDataArrayIndex] = NONE;

		// increase the arrayIndex
		handleChangedDataArrayIndex = (handleChangedDataArrayIndex + 1) % CHANGED_DATA_ARRAY_ELEMENTS;
	}

	// never come here
	return lvRetValue;
}

/*!
 * @brief function to handle a parameter change, will set semaphore available
 * 
 * @param changedParameter the parameter that changed
 * @param newValue the new value that is set
 */
int parameterChangeFunction(parameterKind_t changedParameter, void *newValue)
{
	int lvRetValue = 0;
	uint16_t i = 0;
	static uint8_t changedDataArrayIndex = 0;
	static parameterKind_t timeoutParameter = NONE;
	static void *timoutValue = NULL;
	bool savingParameter = false;
	bool currentParameterSet = false;
	static bool parameterSaved = false;
	bool outputToUserDone = false;

	// loop until currentParameterSet or savingParameter 
	do
	{
		// wait until the array is writable
		while(gChangedParametersArr[changedDataArrayIndex] != NONE && i < 1000)
		{
			if(!outputToUserDone)
			{
				outputToUserDone = true;
				cli_printf("please increase CHANGED_DATA_ARRAY_ELEMENTS with 1!\n");
			}
			// sleep for 1us
			usleep(1);

			// increase the index for the time out
			i++;
		}

		// check for timeout
		if(i >= 1000)
		{
			cli_printf("Timeout happend!\n");
			timeoutParameter = changedParameter;
			timoutValue = newValue;
			savingParameter = true;
		}
		else
		{
			// check if the saved parameter needs to be set
			if(parameterSaved)
			{
				
				// set the new changed data parameter
				gChangedParametersArr[changedDataArrayIndex] = timeoutParameter;
				gChangedDataArr[changedDataArrayIndex] = *(uint32_t*)timoutValue;

				// remember that the parameter is set
				parameterSaved = false;
			}
			else
			{
				// set the new changed data parameter
				gChangedParametersArr[changedDataArrayIndex] = changedParameter;
				gChangedDataArr[changedDataArrayIndex] = *(uint32_t*)newValue;

				// to go out of the while loop
				currentParameterSet = true;
			}

			// increase the semaphore so the task will handle the change
			lvRetValue = sem_post(&gDataChangedSem);
			if (lvRetValue != 0)
			{
				cli_printf("parameterChangeFunction: ERROR sem_post failed\n");
			}

			// increase the arrayIndex
			changedDataArrayIndex = (changedDataArrayIndex + 1) % CHANGED_DATA_ARRAY_ELEMENTS;
		}
		//cli_printf("in array index: %d\n", changedDataArrayIndex);
	}while(!currentParameterSet && !savingParameter);

	// set the saved parameter to true if needed
	if(savingParameter)
		parameterSaved = true;

	// return
	return lvRetValue;
}

/*!
 * @brief function that will will be called if there is an overcurrent fault function 
 * 
 * @param none
 */
void overCurrentFaultFunction(void)
{
	// set the variable high
	gBCCRisingFlank = true;
	// cli_printf("TODO! overCurrentFaultFunction\n");
}

/*!
 * @brief function that will will be called to change the LED
 * 
 * @param newColor the new LED color
 * @param BlinkOn true if led should blink
 */
void changeLedColor(LEDColor_t newColor, bool BlinkOn)
{
	// set the new LED color
	ledState_setLedColor(newColor, BlinkOn);
}

/*!
 * @brief function to be called when new data is set
 * 
 */
void newMeasurementsFunction(void)
{
	int error = 0;

	//cli_printf("Sending over CAN!\n");
#ifndef DONT_DO_UAVCAN
	// send data over UAVCAN
	error = uavcan_sendBMSStatus();

	// check error
	if(error)
	{
		// output to user
		cli_printf("Coulnd't send over UAVCAN!\n");
	}
#endif
}

/*!
 * @brief function that will be called with an interrupt occurs on the BCC pin
 * 
 * @param number the pin number that triggers this ISR
 */
void BCCFaultPinISR(int number)
{
	// check the pin 
	if(gpio_readPin(BCC_FAULT))
	{
		// set the variable high
		gBCCRisingFlank = true;
	}
}

/*!
 * @brief function that will be called with an interrupt occurs on the button (SBC_WAKE) pin
 * 
 * @param number the pin number that triggers this ISR
 */
void ButtonPinISR(int number)
{
	//cli_printf("button: %d\n", gpio_readPin(number));
	// check which transaction
	if(gpio_readPin(SBC_WAKE))
	{
		//cli_printf("Button ISR!\n");
		// set the variable high (release)
		gButtonRisingFlank = true;
	}
	else
	{
		// set the variable high (press)
		gButtonPressFlank = true;
	}
}

/*!
 * @brief function that will be called when it needs to process a cli command when the CLI can't do this
 * 
 * @param command 		the command that needs to be processed 
 * @param showCommand 	if the command is CLI_SHOW this will be the show command to set
 * @param value 		if 1 that CLI_SHOW command is enabled, if 0 it is disabled
 */
void processCLICommand(commands_t command, showCommands_t showCommand, uint8_t value)
{
	int returnValue;
	states_t currentState = getMainState();

	// check for the show commnand
	if(command == CLI_SHOW)
	{
		// pass it through to the batmanagement part to process it
		batManagement_setShowMeas(showCommand, value);
	}
	else if(command == CLI_RESET)
	{
		// check for state
		if(currentState == FAULT)
		{
			// set the rest variable
			getNSetStateCommandVariable(true, CMD_RESET);
		}
		else
		{
			// output to the user
			cli_printf("Can not reset, not in FAULT mode! %s\n", gStatesArray[(int)currentState]);
		}
	}
	else if(command == CLI_SLEEP)
	{
		// check what the current state is to help the user understand why it won't change
		if(currentState == NORMAL || currentState == SELF_DISCHARGE)
		{
			// set the sleep variable
			setTransitionVariable(SLEEP_VAR, true);

			// it can transition
			getNSetStateCommandVariable(true, CMD_GO_2_SLEEP);
		}
		else if(currentState == SLEEP || currentState == OCV)
		{
			// inform user 
			cli_printf("Already in sleep state (or OCV)!\n");
		}
		else
		{
			// inform user
			cli_printf("Can not go to sleep state by command in this state: %s!\n", gStatesArray[(int)currentState]);
		}
	}
	else if(command == CLI_WAKE)
	{
		if(currentState == SLEEP)
		{
			// transition
			getNSetStateCommandVariable(true, CMD_WAKE);
		}
		else
		{
			// can't wake
			cli_printf("Can not wake in this state: %s\n", gStatesArray[(int)currentState]);
		}
	}
	else if(command == CLI_DEEP_SLEEP)
	{
		if(currentState == SLEEP || currentState == CHARGE)
		{
			// transition
			getNSetStateCommandVariable(true, CMD_GO_2_DEEPSLEEP);
		}
		else
		{
			cli_printf("Can not go to deep sleep in this state: %s\n", gStatesArray[(int)currentState]);
		}
	}
	else if(command == CLI_SAVE)
	{
		// save the parameters to the flash
		cli_printf("Saving parameters to flash!\n");

		// save them 
		returnValue = data_saveParameters();

		// check for error
		if(returnValue)
		{
			cli_printf("processCLICommand ERROR: saving par went wrong!%d\n", returnValue);
		}
	}
	else if(command == CLI_LOAD)
	{
		// load the parameters from the flash
		cli_printf("Loading parameters from flash!\n");

		// save them 
		returnValue = data_loadParameters();

		// check for error
		if(returnValue)
		{
			cli_printf("processCLICommand ERROR: saving par went wrong!%d\n", returnValue);

			// set the default values
			if(data_setDefaultParameters())
			{
				// output
				cli_printf("processCLICommand ERROR: could not set default values!\n");
			}
			else
			{
				// save the default values
				returnValue = data_saveParameters();

				// check for error
				if(returnValue)
				{
					cli_printf("processCLICommand ERROR: saving par went wrong!%d\n", returnValue);
				}
			}
		}
	}
	else if(command == CLI_DEFAULT)
	{
		// set the default parameters 
		cli_printf("Setting default parameters!\n");

		// set the default values and check for error
		if(data_setDefaultParameters())
		{
			// output
			cli_printf("processCLICommand ERROR: could not set default values!\n");
		}
	}
	else
	{
		// error
		cli_printf("processCLICommand ERROR: wrong command: %d!\n", command);
	}
}

/*!
 * @brief function that will return the main state, but it will use the mutex 
 * 
 * @return the state
 */
states_t getMainState(void)
{
	states_t lvRetValue = INIT; 

	// lock the mutex
	pthread_mutex_lock(&gStateLock);

	// save the state
	lvRetValue = gCurrentState;

	// unlock the mutex
	pthread_mutex_unlock(&gStateLock);

	// return to the user
	return lvRetValue;
}

/*!
 * @brief function that will set the main state, but it will use the mutex 
 * 
 * @param newState the new state
 */
int setMainState(states_t newState)
{
	int lvRetValue = 0;

	// lock the mutex
	pthread_mutex_lock(&gStateLock);

	// set the state 
	gCurrentState = newState;

	// unlock the mutex
	pthread_mutex_unlock(&gStateLock);

	return lvRetValue;
}

/*!
 * @brief function that will return the charge state, but it will use the mutex 
 * 
 * @return the state
 */
charge_states_t getChargeState(void)
{
	charge_states_t lvRetValue = INIT; 

	// lock the mutex
	pthread_mutex_lock(&gChargeStateLock);

	// save the state
	lvRetValue = gCurrentChargeState;

	// unlock the mutex
	pthread_mutex_unlock(&gChargeStateLock);

	// return to the user
	return lvRetValue;
}

/*!
 * @brief function that will set the charge state, but it will use the mutex 
 * 
 * @param newState the new state
 */
int setChargeState(charge_states_t newState)
{
	int lvRetValue = 0;

	// lock the mutex
	pthread_mutex_lock(&gChargeStateLock);

	// set the state 
	gCurrentChargeState = newState;

	// unlock the mutex
	pthread_mutex_unlock(&gChargeStateLock);

	return lvRetValue;
}

/*!
 * @brief function that will return one of the transition variables
 * 
 * @param variable the variable to get
 *
 * @return the value of the variable
 */
bool getTransitionVariable(transitionVars_t variable)
{
	bool lvRetValue;

	// lock the mutex
	pthread_mutex_lock(&gTransVarLock);

	// check which variable to return
	switch(variable)
	{
		case DISCHAR_VAR:
			// save the state
			lvRetValue = gDischargeDetected;
		break;
		case CHAR_VAR:
			// save the variable 
			lvRetValue = gChargeDetected;
		break;
		case SLEEP_VAR:
			// save the variable 
			lvRetValue = gSleepDetected;
		break;
	}

	// unlock the mutex
	pthread_mutex_unlock(&gTransVarLock);

	// return to the user
	return lvRetValue;	
}

/*!
 * @brief function that will set one of the transition variables
 * 
 * @param variable the variable to set
 * @param newValue the new value to set
 *
 * @return 0 if ok
 */
int setTransitionVariable(transitionVars_t variable, bool newValue)
{
	int lvRetValue = 0;

	// lock the mutex
	pthread_mutex_lock(&gTransVarLock);

	// check which variable to return
	switch(variable)
	{
		case DISCHAR_VAR:
			// set the new value
			gDischargeDetected = newValue;
		break;
		case CHAR_VAR:
			// set the new value
			gChargeDetected = newValue;
		break;
		case SLEEP_VAR:
			// set the new value
			gSleepDetected = newValue;
		break;
	}

	// unlock the mutex
	pthread_mutex_unlock(&gTransVarLock);

	// return to the user
	return lvRetValue;	
}

/*!
 * @brief 	function that will set one of the transition variables
 *
 * @param 	setNotGet if true it is used to set the variable, false to get the variable
 * @param 	newValue if setNotGet is true, this is the new value
 *
 * @return 	the state command variable, 0 if none is set, CMD_ERROR if error
 */
stateCommands_t getNSetStateCommandVariable(bool setNotGet, stateCommands_t newValue)
{
	stateCommands_t lvRetValue = CMD_ERROR;
	static stateCommands_t stateCommand = CMD_NONE;

	// lock the mutex
	pthread_mutex_lock(&gStateCommandLock);

	// check if write or read
	// if set
	if(setNotGet)
	{
		// set the new variable
		stateCommand = newValue;
	}
	else
	// if get
	{
		// save the variable 
		lvRetValue = stateCommand;
	}

	// unlock the mutex
	pthread_mutex_unlock(&gStateCommandLock);

	// return the value
	return lvRetValue;
}
