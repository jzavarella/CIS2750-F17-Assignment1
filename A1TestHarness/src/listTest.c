#include "LinkedListAPI.h"

#include "testcases.h"
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#include "testharness.h"

#define TESTS 7
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
/*
static testRec* getDataFromChild(int* pipefd){
    
    testRec* tmpRec;
    int testNum;
    int numSubs;
    int subsPassed;
    int feedbackLen;
    char feedback[300];
    
    close(pipefd[1]); // close the write-end of the pipe, I'm not going to use it
    read(pipefd[0], &testNum, sizeof(int));
    read(pipefd[0], &numSubs, sizeof(int));
    read(pipefd[0], &subsPassed, sizeof(int));
    read(pipefd[0], &feedbackLen, sizeof(int));
    
    if (testNum < 1 || testNum > TESTS ||
        numSubs < 1 || numSubs > 20 ||
        subsPassed < 0 || subsPassed > numSubs ||
        feedbackLen <= 0 || feedbackLen > 2000){
        return NULL;
    }
    tmpRec = initRec(testNum, numSubs, feedback);
    tmpRec->subsPassed = subsPassed;
    tmpRec->feedbackLen = feedbackLen;
    
    for (int i = 0; i < feedbackLen; i++){
        int len;
        read(pipefd[0],&len, sizeof(int));
        char* tmp = malloc(len);
        read(pipefd[0],tmp, len);
        tmpRec->feedback[i] = tmp;
    }
   
    return tmpRec;
}

void sendDataToParent(int* pipefd, testRec* tmpRec){
    close(pipefd[0]); // close the read-end of the pipe, I'm not going to use it
    write(pipefd[1], &tmpRec->testNum, sizeof(int));
    write(pipefd[1], &tmpRec->numSubs, sizeof(int));
    write(pipefd[1], &tmpRec->subsPassed, sizeof(int));
    write(pipefd[1], &tmpRec->feedbackLen, sizeof(int));
    
    for (int i = 0; i < tmpRec->feedbackLen; i++){
        int tmpLen = strlen(tmpRec->feedback[i])+1;
        write(pipefd[1], (const char*)&tmpLen, sizeof(int));
        write(pipefd[1], tmpRec->feedback[i], tmpLen);
    }
    close(pipefd[1]);
}

void runTest(testRec* (*testFunc)(int) ){
    int pipefd[2];
    pid_t childPID;
    char feedback[300];
    
    pipe(pipefd); // create the pipe
    childPID = fork();
    if(childPID >= 0){ // fork was successful
        if (childPID == 0){
            signal(SIGSEGV, SIG_DFL);
            //0 based arrays make us use testNo-1
            testRec* tmpRec = testFunc(testNo);
            
            sendDataToParent(pipefd, tmpRec);
            exit(EXIT_SUCCESS);
        }else{
            
            testRec* tmpRec = getDataFromChild(pipefd);
            
            int status;
            wait(&status);
            
            if (WIFSIGNALED(status)){
                tmpRec = initRec(testNo-1, 1, feedback);
                tmpRec->subsPassed = 0;
                tmpRec->feedback[0] = malloc(200);
                char tmpBuf[1000];
                switch (WTERMSIG(status)){
                case SIGSEGV:
                    sprintf(tmpBuf, "Test %d encountered a segmentation fault and crashed", testNo);
                    strcpy(tmpRec->feedback[0], tmpBuf);
                    break;
                case SIGBUS:
                    sprintf(tmpBuf, "Test %d encountered a bus error and crashed", testNo);
                    strcpy(tmpRec->feedback[0], tmpBuf);
                    break;
                default:
                    sprintf(tmpBuf, "Test %d crashed - killed by a signal %d", testNo, WTERMSIG(status));
                    strcpy(tmpRec->feedback[0], tmpBuf);
                    break;
                }
            }
            
            testRecords[testNo-1] = tmpRec;
            studentScore = studentScore + getScore(testRecords[testNo-1]);
            testNo++;
        }
    }
}
*/
void addTestResult(testRec* tmpRec){
    testRecords[testNo-1] = tmpRec;
    studentScore = studentScore + getScore(testRecords[testNo-1]);
    testNo++;
}

int main()
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
    
    //Create list
    if (DEBUG) fprintf(OUT,"Testing list creation...\n");
    testRec* tmpRec = initListTest(testNo);
    addTestResult(tmpRec);
    
    
    //Node Creation
    if (DEBUG) fprintf(OUT, "Testing Node Creation\n");
    tmpRec = initNodeTest(testNo);
    addTestResult(tmpRec);
    
    //Insert Front and Back
    if (DEBUG) fprintf(OUT, "Testing insertion front and back\n");
    tmpRec = insertTest(testNo);
    addTestResult(tmpRec);
    
    //Getter functions test
    if (DEBUG) fprintf(OUT, "Testing getter functions\n");
    tmpRec = getTest(testNo);
    addTestResult(tmpRec);
    
    
    //Insert Sorted
    if (DEBUG) fprintf(OUT, "Testing sorted insertion\n");
    tmpRec = insertSortedTest(testNo);
    addTestResult(tmpRec);
    
    //delete List
    if (DEBUG) fprintf(OUT, "Testing deletion functionality");
    //runTest(&deleteTest);
    tmpRec = deleteTest(testNo);
    addTestResult(tmpRec);
    
    
    //Printing
    if (DEBUG) fprintf(OUT, "Testing list printing");
    tmpRec = printTest(testNo);
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
    
    printf("Score: %d\n", studentScore);
    
    return 0;
    
}



