#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <signal.h>

#define CHECK_STATUS(status) {			\
		if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) \
		{								    \
			syslog(LOG_INFO,"ksq: system shell failed!");               \
			return -1;						    \
		}}

int set_affinity(char *ifname, int queues)
{
	FILE * fp;
        int i=0;
	char cmd[512] = {0};
	char buffer[80] = {0};
	int irq;
	int mask;
	int status;

	for(i = 0; i < queues; i++)
	{
		mask = (2 << i );
		
		sprintf(cmd, "cat /proc/interrupts | grep -i %s-input.%d | cut  -d:  -f1 | sed \"s/ //g\"", ifname, i);
		
		fp = popen(cmd,"r"); 
		fgets(buffer,sizeof(buffer),fp);
		
		printf("%s",buffer); 
		irq = atoi(buffer);
		
		pclose(fp); 

		memset(cmd, 0, 512);
		sprintf(cmd, "echo %d > /proc/irq/%d/smp_affinity", mask, irq);
		status = system(cmd);
		CHECK_STATUS(status);

		memset(cmd, 0, 512);
		memset(buffer, 0, 80);
		sprintf(cmd, "cat /proc/interrupts | grep -i %s-output.%d | cut  -d:  -f1 | sed \"s/ //g\"",ifname,i);
		
		fp = popen(cmd,"r"); 
		fgets(buffer,sizeof(buffer),fp);
		
		printf("%s",buffer); 
		irq = atoi(buffer);
		
		pclose(fp); 

		memset(cmd, 0, 512);
		sprintf(cmd, "echo %d > /proc/irq/%d/smp_affinity", mask, irq);
		status = system(cmd);
		CHECK_STATUS(status);
	}

	return 0;
}

int get_check_if()
{
	FILE * fp;
        int i=0;
        int sockfd;
        struct ifconf ifconf;
        unsigned char buf[512] = {0};
	char cmd[512] = {0};
        struct ifreq *ifreq;
	char buffer[80] = {0};
	int queue;
	int ret; 

        ifconf.ifc_len = 512;
        ifconf.ifc_buf = buf;

        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
        {
                perror("socket");
                exit(1);
        }

        ioctl(sockfd, SIOCGIFCONF, &ifconf);

        ifreq = (struct ifreq*)buf;
        for(i = (ifconf.ifc_len/sizeof(struct ifreq)); i > 0; i--)
        {
		memset(cmd, 0, 512);
		memset(buffer, 0, 80);
		
		if(strncmp("eth0", ifreq->ifr_name, 4) != 0)
		{
			ifreq++;
			continue;
		}

		sprintf(cmd, "ethtool -l %s | grep Combined | awk '{print $2}'", ifreq->ifr_name);
		
		fp=popen(cmd,"r"); 
		fgets(buffer,sizeof(buffer),fp);
		
		printf("%s",buffer); 
		queue = atoi(buffer); 
	
		if(queue == 1)
		{
			ifreq++;
			pclose(fp); 
			continue;
		}

		memset(cmd, 0, 512);

		sprintf(cmd, "ethtool -L %s combined %d", ifreq->ifr_name, queue);
                printf("cmd is %s\n", cmd);

		ret = system(cmd);
		CHECK_STATUS(ret);

		memset(cmd, 0, 512);
		sprintf(cmd, "service irqbalance stop");

		system(cmd);
		CHECK_STATUS(ret);

		set_affinity("virtio0", queue);
		pclose(fp); 
                ifreq++;
        }

        return 0;
}

int daemon_init(void)
{
        pid_t pid;

        if((pid = fork()) < 0)
                return(-1);
        else if(pid != 0)
                exit(0); /* parent exit */

        /* child continues */
        setsid(); /* become session leader */
        chdir("/"); /* change working directory */

        umask(0); /* clear file mode creation mask */

        close(0); /* close stdin */
        close(1); /* close stdout */
        close(2); /* close stderr */
        return(0);
}

void sig_term(int signo)
{
        if(signo == SIGTERM)
        /* catched signal sent by kill(1) command */
        {
                syslog(LOG_INFO, "program terminated.");
                closelog();
                exit(0);
        }
}

int main()
{
	FILE * fp;
	int ret;
	char cmd[512] = {0};
	char buffer[80] = {0};
        
	if(daemon_init() == -1)
        {
                printf("can't fork self\n");
                exit(0);

        }

        openlog("ksq", LOG_PID, LOG_USER);

        syslog(LOG_INFO, "program started.");
        
	signal(SIGTERM, sig_term); /* arrange to catch the signal */

	while(1)
        {
		memset(cmd, 0, 512);
                memset(buffer, 0, 80);
                sprintf(cmd, "ls /sys/class/net | grep eth0");
	
                fp = popen(cmd,"r");
                fgets(buffer,sizeof(buffer),fp);

                printf("%s",buffer);
		
		if(strcmp("eth0", buffer) == 0){
			memset(cmd, 0, 512);
                	sprintf(cmd, "ifup eth0");
			system(cmd);
			sleep(1);
			break;
		}
		else
			sleep(1);
        }

	ret = get_check_if();
	if (ret != 0)
	{
		syslog(LOG_INFO,"NIC queue cpu affinity set failed!");		
	}

	return(0);
}
