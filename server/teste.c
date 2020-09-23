#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

int end = 0;

void time_out_handler(int signal)
{
	end = 1;
}


void main(void)
{
	//estrutura que define um tratador de sinal (deve ser global ou static)
	struct sigaction action ;

	//estrutura de inicialização to timer
	struct itimerval default_timer ;

	struct itimerval stop_timer ;

	//act handler
		action.sa_handler = time_out_handler;
	  	sigemptyset (&action.sa_mask) ;
	  	action.sa_flags = 0 ;
	  	sigaction (SIGALRM, &action, 0);

		//tick set
		default_timer.it_value.tv_usec = 0;      // primeiro disparo, em micro-segundos
	  	default_timer.it_value.tv_sec  = 2;      	 	// primeiro disparo, em segundos
	  	default_timer.it_interval.tv_usec = 0;   // disparos subsequentes, em micro-segundos
	  	default_timer.it_interval.tv_sec  = 2;   	 	// disparos subsequentes, em segundos

	  	stop_timer.it_value.tv_usec = 0;      // primeiro disparo, em micro-segundos
	  	stop_timer.it_value.tv_sec  = 0;      	 	// primeiro disparo, em segundos
	  	stop_timer.it_interval.tv_usec = 0;   // disparos subsequentes, em micro-segundos
	  	stop_timer.it_interval.tv_sec  = 0;   	 	// disparos subsequentes, em segundos

	  	setitimer (ITIMER_REAL, &default_timer, 0);

	  	end = 0;
	  	while(!end)
	  	{
	  		printf("dormindo\n");
	  		usleep(3000000);
	  		printf("acordou\n");
	  	}
	  	printf("%d\n",end);
	  	setitimer (ITIMER_REAL, &stop_timer, 0);
	  	end = 0;
	  	while(!end)
	  	{
	  		printf("dormindo\n");
	  		usleep(3000000);
	  	}
}
	