/*******************************************************************
    Copyright (C) 2015 Christian Ege - STM32 modifications
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.

*******************************************************************/
/*!
    \file Cmd.c

    This implements a simple command line interface for the Arduino so that
    its possible to execute individual functions within the sketch.
*/
/**************************************************************************/

#include "usart.h"
#include "cmd.h"
#include "tfp_printf.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


// command line message buffer and pointer
static uint8_t msg[CMD_MAX_MSG_SIZE];
static uint8_t *msg_ptr;

// linked list for command table
static cmd_t *cmd_tbl_list, *cmd_tbl;

// text strings for command prompt (stored in flash)
const char cmd_banner[]  = "*************** CMD *******************";
const char cmd_prompt[]  = "CMD >> ";
const char cmd_unrecog[] = "CMD: Command not recognized.";

/**************************************************************************/
/*!
    Generate the main command banner
*/
/**************************************************************************/
void cmd_display_banner()
{
    tfp_printf("\r\n");
    tfp_printf("%s","***************************************\r\n");
    tfp_printf("BUILD DATE        : %s\r\n",__DATE__);
    tfp_printf("BUILD TIME        : %s\r\n",__TIME__);
    tfp_printf("STM32 HAL version : 0x%08X\r\n",HAL_GetHalVersion());
    tfp_printf("STM32 Rev. ID     : 0x%08X\r\n",HAL_GetREVID());
    tfp_printf("STM32 Dev. ID     : 0x%08X\r\n",HAL_GetDEVID());
    tfp_printf("%s\r\n",cmd_banner);
}

/**************************************************************************/
/*!
    Generate the main command prompt
*/
/**************************************************************************/
void cmd_display_prompt()
{
    tfp_printf("%s",cmd_prompt);
}

/**************************************************************************/
/*!
    Generate the main command prompt
*/
/**************************************************************************/
void cmd_display_unknown()
{
    tfp_printf("%s\r\n",cmd_unrecog);
}


/**************************************************************************/
/*!
    Parse the command line. This function tokenizes the command input, then
    searches for the command table entry associated with the commmand. Once found,
    it will jump to the corresponding function.
*/
/**************************************************************************/
void cmd_parse(char *cmd)
{
    uint8_t argc, i = 0;
    char *argv[30];
    cmd_t *cmd_entry;

    fflush(stdout);

    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[i] = strtok(cmd, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));

    // save off the number of arguments for the particular command.
    argc = i;

    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt
    for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {
        if (!strcmp(argv[0], cmd_entry->cmd))
        {
            cmd_entry->func(argc, argv);
            cmd_display_prompt();
            return;
        }
    }

    // command not recognized. print message and re-generate prompt.
    cmd_display_unknown();
    cmd_display_prompt();
}

/**************************************************************************/
/*!
    This function processes the individual characters typed into the command
    prompt. It saves them off into the message buffer unless its a "backspace"
    or "enter" key.
*/
/**************************************************************************/
void cmd_handler()
{
    unsigned char c = 0x00;
    if(HAL_OK ==  HAL_UART_Receive(&huart1, &c, 1, 100))
    {
        switch (c)
        {
        case '\r':
            // terminate the msg and reset the msg ptr. then send
            // it to the handler for processing.
            *msg_ptr = '\0';
            tfp_printf("\r\n");
            cmd_parse((char *)msg);
            msg_ptr = msg;
            break;

        case '\b':
            // backspace
            tfp_printf("%c",c);
            if (msg_ptr > msg)
            {
                msg_ptr--;
            }
            break;

        default:
            // normal character entered. add it to the buffer
            tfp_printf("%c",c);
            *msg_ptr++ = c;
            break;
        }
    }
    else
    {
        tfp_printf("%s","timeout occurred");
        cmd_display_prompt();
    }
}

/**************************************************************************/
/*!
    This function should be set inside the main loop. It needs to be called
    constantly to check if there is any available input at the command prompt.
*/
/**************************************************************************/
void cmd_poll()
{
    while (HAL_OK == UART_WaitOnFlagUntilTimeout(&huart1, UART_FLAG_RXNE, RESET, 0))
    {
        cmd_handler();
    }
}

/**************************************************************************/
/*!
    Initialize the command line interface. This sets the terminal speed and
    and initializes things.
*/
/**************************************************************************/
void cmd_init()
{
    // init the msg ptr
    msg_ptr = msg;

    // init the command table
    cmd_tbl_list = NULL;

    // set the serial speed
    cmd_display_banner();
    cmd_display_prompt();
}

/**************************************************************************/
/*!
    Add a command to the command table. The commands should be added in
    at the setup() portion of the sketch.
*/
/**************************************************************************/
void cmd_add(char *name, void (*func)(int argc, char **argv))
{
    // alloc memory for command struct
    cmd_tbl = (cmd_t *)malloc(sizeof(cmd_t));

    // alloc memory for command name
    char *cmd_name = (char *)malloc(strlen(name)+1);

    // copy command name
    strcpy(cmd_name, name);

    // terminate the command name
    cmd_name[strlen(name)] = '\0';

    // fill out structure
    cmd_tbl->cmd = cmd_name;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
}

/**************************************************************************/
/*!
    Convert a string to a number. The base must be specified, ie: "32" is a
    different value in base 10 (decimal) and base 16 (hexadecimal).
*/
/**************************************************************************/
uint32_t cmd_str2Num(char *str, uint8_t base)
{
    return strtol(str, NULL, base);
}
