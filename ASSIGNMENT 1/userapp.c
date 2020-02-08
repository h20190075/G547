#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>

#define WR_VALUE _IOW('a','a',int32_t*)
#define WR_VALUE2 _IOW('a','b',int32_t*)

int main()
{
        int fd;
        char buffer[16];
        uint16_t value;
        int32_t number,number2;
        buffer[16]='\0';
       
        printf("\nOpening Driver\n");
        fd = open("/dev/adc8", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file\n");
                return 0;
        }

        printf("Enter the channel to send\n");
        scanf("%d",&number);
        printf("Writing Value to Driver\n");
      
         ioctl(fd, WR_VALUE, number); 
         

        printf("Enter the preference to send\n");
        scanf("%d",&number2);
        printf("Writing Value to Driver\n");
        ioctl(fd, WR_VALUE2, number2); 
 
    
       read(fd, buffer, sizeof(buffer));
       puts(buffer);
      
      

       /* printf("Reading Value from Driver\n");
        ioctl(fd, RD_VALUE, (uint16_t*) &value);
        printf("Value is %d\n", value);*/

        printf("Closing Driver\n");
        close(fd);
}
