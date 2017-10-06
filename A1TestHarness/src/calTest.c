#include "CalendarParser.h"

#include "calTestCases.h"
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#include "testharness.h"

#define TESTS 10
#define DEBUG 0
#define OUT stdout

int testNo;
testRec * * testRecords;
int studentScore;  //globals  required to handle segfaults gracefully
//------------ Something went wrong and student code segfaulted --------------


static void segfaultCatcher(int signal,  siginfo_t* si, void *arg)
{
    printf("\n\n************** Code Segfaulted: Partial Report ********************\n");
    int j;
    for(j=0; j<TESTS; j++){
        if(testRecords[j] != NULL){
            printRecord(testRecords[j]);
        }
    }
    
    printf("*******************\nPARTIAL RESULTS\nProgram caused segfault\n*******************\n");
    printf("Partial Score:%d\n",studentScore);
    
    
    exit(EXIT_FAILURE);
}

void addTestResult(testRec* tmpRec){
    testRecords[testNo-1] = tmpRec;
    studentScore = studentScore + getScore(testRecords[testNo-1]);
    testNo++;
}

float calcGrade(void){
    float weights[] = {10,10,15,5,5,5,5,15,10,10};
    float totalScore = 0;
    for (int i = 0; i < TESTS; i++){
        totalScore += weights[i]*(float)getScore(testRecords[i]);
    }
    return totalScore;
}

int main(void)
{
    
    studentScore = 0;
    testNo = 1;
    
    struct sigaction segfaultSignaler;
    // set up the segfault handler
    memset(&segfaultSignaler, 0, sizeof(struct sigaction));
    sigemptyset(&segfaultSignaler.sa_mask);
    segfaultSignaler.sa_sigaction = segfaultCatcher;
    segfaultSignaler.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &segfaultSignaler, NULL);
    
    
    //record keeping array
    testRecords =  malloc(sizeof(testRec *)* TESTS);
    
    if(DEBUG) fprintf(OUT, "************** Testing Details ********************\n\n");
    
    //Create calendar
    if (DEBUG) fprintf(OUT,"Testing calendar creation...\n");
    testRec* tmpRec = simpleCalTest(testNo);
    addTestResult(tmpRec);
    
    tmpRec = medCalTest(testNo);
    addTestResult(tmpRec);
    
    tmpRec = largeCalTest(testNo);
    addTestResult(tmpRec);
    
    //Print calendar
    if (DEBUG) fprintf(OUT,"Testing printCalendar...\n");
    tmpRec = printCalTest(testNo);
    addTestResult(tmpRec);
    
    //Delete calendar - test for crashes
    if (DEBUG) fprintf(OUT,"Testing deleteCalendar...\n");
    tmpRec = deleteCalTest(testNo);
    addTestResult(tmpRec);
    
    //Print error test
    if (DEBUG) fprintf(OUT,"Testing printError...\n");
    tmpRec = printErrTest(testNo);
    addTestResult(tmpRec);
    
    //Test file error handling
    if (DEBUG) fprintf(OUT,"Testing file error handling...\n");
    tmpRec = invFileTest(testNo);
    addTestResult(tmpRec);
    
    //Test invaid calendars
    if (DEBUG) fprintf(OUT,"Testing invalid caledar object...\n");
    tmpRec = invCalTest(testNo);
    addTestResult(tmpRec);

    //Test invaid events
    if (DEBUG) fprintf(OUT,"Testing caledar object with invalid events...\n");
    tmpRec = invEvtTest(testNo);
    addTestResult(tmpRec);
    
    //Test invaid alarms
    if (DEBUG) fprintf(OUT,"Testing caledar object with invalid alarm...\n");
    tmpRec = invAlmTest(testNo);
    addTestResult(tmpRec);
    
    printf("Last test completed");
    //Test Conclusion and report
    printf("\n\n************** Testing Report ********************\n");
    
    int j;
    for(j=0; j<TESTS; j++)
    {
        
        printRecord(testRecords[j]);
        //printf("\n");
    }
    //fclose(output);
    
    printf("Score: %f\n", calcGrade());
    
    return 0;
    
}



