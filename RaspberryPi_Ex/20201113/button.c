#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define BUTTON_PIN 20
#define BUTTON_POUT 21
#define LED_POUT 17
#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30 


static int gpioexport(int pin)
{
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/export", O_WRONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open export for writing!\n");
      return(-1);
   }

   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);
   return 0;
}

static int gpiounexport(int pin)
{
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/unexport", O_WRONLY);;
   if ( -1 == fd) 
   {
      fprintf(stderr, "Failed to open unexport for writing!\n");
      return(-1);
   }
   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);

   return(0);
}

static int gpiodirection(int pin, int dir)
{
   static const char s_directions_str[] ="in\0out";
   char path[DIRECTION_MAX] = "/sys/class/gpio/gpio%d/direction";
   int fd;

   snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction",pin);

   fd = open(path, O_WRONLY);
   if( -1 == fd )
   {
      fprintf(stderr, "Failed to open gpio direction for writing!\n");
      return(-1);
   }

   if(-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2:3 ))
   {
      fprintf(stderr, "Failed to set direction!\n");
      return(-1);
   }

   close(fd);
   return(0);
}


static int gpioread(int pin)
{
   char path[VALUE_MAX];
   char value_str[3];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_RDONLY);

   if(-1 == fd)
   {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
      return(-1);
   }

   if(-1 == read(fd, value_str, 3))
   {
      fprintf(stderr, "Failed to read value!\n");
      return(-1);
   }

   close(fd);

   return(atoi(value_str));
}

static int gpiowrite(int pin, int value)
{
   static const char s_values_str[] = "01";
   char path[VALUE_MAX];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_WRONLY);

   if(-1 == fd)
   {
      fprintf(stderr, "Failed to open gpio value for writing!\n");
   }
   if(1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1))
   {
      fprintf(stderr, "Failed to write value!\n");
      close(fd);
      return(0);
   }
   printf("write value nice~\n");
}

int main(int argc, char *argv[])
{
   if(-1 == gpioexport(BUTTON_POUT) || -1 == gpioexport(BUTTON_PIN))
      return(1);
   if(-1 == gpioexport(LED_POUT))
      return(1);


   if(-1 == gpiodirection(BUTTON_POUT, OUT) || -1 == gpiodirection(BUTTON_PIN, IN))
      return(2);
   if(-1 == gpiodirection(LED_POUT,OUT))
      return(2);
    
   int LED_state = 0;

   while(1)
   {
      if(-1 == gpiowrite(BUTTON_POUT, 1 ))
         return(3);

      printf("i'm reading %d in gpio %d\n", gpioread(BUTTON_PIN), BUTTON_PIN);

      if(gpioread(BUTTON_PIN) == 0)
      {
         if(LED_state == 0)
             LED_state = 1;
         else
             LED_state = 0;
      }

      if(-1 == gpiowrite(LED_POUT, LED_state))
            return(3);

      usleep(100000);
   }

   if(-1 == gpiounexport(BUTTON_POUT) || -1 == gpiounexport(BUTTON_PIN))
      return(4);

   return(0);
}
