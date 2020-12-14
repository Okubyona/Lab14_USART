/*	Author: Andrew Bazua [abazu001]
 *  Partner(s) Name:
 *	Lab Section: 024
 *	Assignment: Lab #14  Exercise #1
 *	Exercise Description: [Two microcontrollers each have an LED connected to
    PA0. Both LEDs are synchronously toggled on/off in one second intervals
    using USART.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo link:
 *  https://drive.google.com/file/d/1tx4va8G74PvczWJKTNmMUCFdfq_ylQRE/view?usp=sharing
 *
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"
#include "usart_ATMega1284.h"

typedef enum Leader_States { send } L_states;
typedef enum Follower_States { receive } R_states;

int LeaderTick(int state);
int FollowerTick(int state);

int main(void) {
    DDRA = 0xFF; PORTA = 0x00;

    initUSART(0);
    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = receive;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &FollowerTick;

    unsigned long GCD = tasks[0]->period;
    for (unsigned char i = 0; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    while (1) {
        for (unsigned char i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }

    return 1;
}

// Shared variables

// -----------------------------------------

int LeaderTick(int state) {
    static unsigned char sendData = 0x00;

    switch (state) {
        case send:
            if (USART_IsSendReady(0)) {
                sendData = !sendData;
                PORTA = sendData & 0x01;
                USART_Send(sendData, 0);
            }
            break;

        default: state = send; break;
    }

    return state;
}

int FollowerTick(int state) {
    static unsigned char receiveData;

    switch (state) {
        case receive:
            if (USART_HasReceived(0)) {
                receiveData = USART_Receive(0);
                PORTA = receiveData & 0x01;
            }
    }

    return state;
}
