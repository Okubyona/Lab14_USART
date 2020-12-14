/*	Author: Andrew Bazua [abazu001]
 *  Partner(s) Name:
 *	Lab Section: 024
 *	Assignment: Lab #14  Exercise #2
 *	Exercise Description: [Expand upon Exercise 1 by adding a switch that
    determines the mode (Leader or Follower) of the microcontroller.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo link:
 *  https://drive.google.com/file/d/1a-rwM30zuf8V1FOMYhwbXCOx06L-LWdG/view?usp=sharing
 *
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"
#include "usart_ATMega1284.h"

typedef enum Leader_States { wait_L, send } L_states;
typedef enum Follower_States { wait_F, receive } R_states;

int LeaderTick(int state);
int FollowerTick(int state);

int main(void) {
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0x00; PORTB = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;

    initUSART(0);
    initUSART(1);

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = send;
    task1.period = 20;
    task1.elapsedTime = task1.period;
    task1.TickFct = &LeaderTick;

    task2.state = receive;
    task2.period = 20;
    task2.elapsedTime = task2.period;
    task2.TickFct = FollowerTick;

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
#define switchMode (~PINB & 0x01)

// -----------------------------------------

int LeaderTick(int state) {
    static unsigned char sendData = 0x00;
    static unsigned char count;

    switch (state) {
        case wait_L:
            state = switchMode ? send : wait_L;
            break;
        case send:
            state = switchMode ? send : wait_L;
            break;

        default: state = wait_L; break;
    }

    switch (state) {
        case wait_L:
            PORTC = 0x00;
            break;

        case send:
            PORTC = 0x01;
            if (count == 50) {
                if (USART_IsSendReady(1)) {
                    sendData = !sendData;
                    PORTA = sendData & 0x01;
                    USART_Send(sendData, 1);
                }
                count = 0;
            }
            ++count;
            break;
    }

    return state;
}

int FollowerTick(int state) {
    static unsigned char receiveData;

    switch (state) {
        case wait_F:
            state = switchMode ? wait_F : receive;
            break;

        case receive:
            state = switchMode ? wait_F : receive;


        default: state = receive; break;
    }

    switch (state) {
        case wait_F:
            PORTC = 0x01;
            break;

        case receive:
            if (USART_HasReceived(0)) {
                receiveData = USART_Receive(0);
                PORTA = receiveData & 0x01;
            }
            break;
    }

    return state;
}
