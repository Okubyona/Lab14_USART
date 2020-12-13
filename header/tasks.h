
#ifndef TASKS_H
#define TASKS_H

unsigned long int findGCD(unsigned long int a, unsigned long b) {
    unsigned long int c;
    while(1) {
        c = a % b;
        if (c == 0) { return b; }
        a = b;
        b = c;
    }

    return 0;
}

//Struct for tasks

typedef struct Task {
    signed char state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct) (int);
} task;

#endif
