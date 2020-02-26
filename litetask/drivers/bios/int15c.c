#include <stdio.h>
#include <process.h>
#include <dos.h>

extern void far installInt15(void);
extern void far removeInt15(void);

static volatile int signal = 0;
static char far *screen = (char far *)0xB0000000L;
static long suspends = 0L;
static long resumes = 0L;
static long idles = 0L;
static long deviceIds[128] = { 0 };

long far getTaskHandle(void)
{
   return 0L;
}

int far suspendTask(int deviceId)
{
static int i = 0, j = 0;

   signal = 1;
   suspends++;
   deviceIds[deviceId]++;
   _enable();
   while(signal)
   {
      idles++;
      screen[i] = (j) ? (char)' ' : (char)'*';
      if(i + 2 >= 4000)
         j = 1 - j;
      i = (i + 2) % 4000;
   }
   return 0;
}

int far resumeTask(long handle, int state)
{
   resumes++;
   return signal = 0;
}

long far setVector(int intr, void (interrupt far *newVector)())
{
long ret;

   ret = (long)_dos_getvect(intr);
   _dos_setvect(intr, newVector);
   return ret;
}

void main(int argc, char **argv)
{
int i;

/* see if screen segment provided */
   if(argc > 1)
   {
      sscanf(argv[1], "%X", &i);
      FP_SEG(screen) = i;
      printf("Using screen @ %p\n", screen);
   }

/* set vector trap */
   installInt15();

/* now shell to DOS and see what happens */
   spawnvp(P_WAIT, "COMMAND.COM", NULL);

/* reset vector and show stats */
   removeInt15();
   printf("%ld suspends, %ld resumes, %ld idle loops\n",
      suspends, resumes, idles);
   for(i=0; i<128; i++)
   {
      if(deviceIds[i])
         printf("Device: %d, Waits: %ld\n", i, deviceIds[i]);
   }
}

