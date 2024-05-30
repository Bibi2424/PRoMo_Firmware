#include <errno.h>
#include <stdio.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include "stm32f4xx_ll_usart.h"
#include "main.h"

#if DEBUG_UART == 1
   #define D_USART USART1
#elif DEBUG_UART == 6
   #define D_USART USART6
#endif



int _write (int fd, const void *buf, size_t count);
int _write (int fd, const void *buf, size_t count) {
   if ((fd != STDOUT_FILENO) && (fd != STDERR_FILENO)) {
      errno = EBADF;
      return -1;
   }
   uint8_t* ch = ((uint8_t*)buf);
   for(size_t i = 0; i < count; i++){
      // while(!LL_USART_IsActiveFlag_TXE(D_USART));   
      LL_USART_TransmitData8(D_USART, *ch++);
      while(!LL_USART_IsActiveFlag_TXE(D_USART));
   }
   return count;
}

