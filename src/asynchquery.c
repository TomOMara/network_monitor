/*
*	asynchquery.c
*	
*	script performs SNMPWALK of a target device, which feeds the results into a
*	ZMQ Worker script ( query_processors.cpp ) who places results in both
*	local database ( tcp://127.0.0.1:3306 )and 
*	remote database( tcp://10.0.13.47:3306 ). 
*
*	%> gcc -o ../bin/asynchquery asynchquery.c -l netsnmp -I/usr/include/mysql -lmysqlclient
*
*/



#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "mysql/mysql.h"
#include "mysql/my_config.h"
#include "mysql/my_global.h"

#include <stdio.h>
#include <string.h>

/* close connection */
void finish_with_error(MYSQL *con)
{
	fprintf(stderr, "%s\n", mysql_error(con));
     mysql_close(con);
     exit(1);
}

/* HOST LIST - Should get this from database */
struct host { 
	char *name;
	char *community;
	} hosts[] = { 
		{"10.0.2.2",  "public" },
//		{ }, 
		{ NULL }
	};

/* List of Variables to query for */ 
struct oid {
	char *Name;
	oid Oid[MAX_OID_LEN];
	int OidLen;
	} oids[] = {
		/* SNMPv2 MIB */
		{ "1.3.6.1.2.1.1.1.0" },		// sysDescr.0
		{ "1.3.6.1.2.1.1.2.0" }, 		// sysObjectId.0
		{ "1.3.6.1.2.1.1.3.0" }, 		// sysUpTime.0
		{ "1.3.6.1.2.1.1.4.0" }, 		// sysContact.0
		{ "1.3.6.1.2.1.1.5.0" }, 		// sysName.0
		{ "1.3.6.1.2.1.1.6.0" }, 		// sysLocation.0
		{ "1.3.6.1.2.1.1.7.0" }, 		// sysServices.0
		{ "1.3.6.1.2.1.2.1.0" },		// ifNumber.0 (levels of OSI added)
		
		/* HOST RESOURCES MIB */ 
		{ "1.3.6.1.2.1.25.1.1.0" },		// sysuptime
		{ "1.3.6.1.2.1.25.1.5.0" },		// no users
		{ "1.3.6.1.2.1.25.1.6.0" },		// sysprocesses
		{ "1.3.6.1.2.1.2.2.1.10.2" },		// hr device
		{ "1.3.6.1.4.1.59595.3.10.1.2.1.1.3.9" },		// admin info
	

		/* TCP MIB */ 
		{ "1.3.6.1.2.1.6.5.0" },		// tcpActiveOpens.0
		{ "1.3.6.1.2.1.6.6.0" },		// tcpPassiveOpens.0
		{ "1.3.6.1.2.1.6.7.0" },		// tcpAttemptFails.0
		{ "1.3.6.1.2.1.6.8.0" },		// tcpEstabResets.0
		{ "1.3.6.1.2.1.6.9.0" },		// tcpCurrEstab.0
		{ "1.3.6.1.2.1.6.10.0" },		// tcpInSegs.0
		{ "1.3.6.1.2.1.6.11.0" },		// tcpOutSegs.0
		{ "1.3.6.1.2.1.6.12.0" },		// tcpRetransSegs.0
//		{ "1.3.6.1.2.1.6.13.0" },		// tcpConnTable.0
		{ "1.3.6.1.2.1.6.14.0" },		// tcpInErrs.0
		{ "1.3.6.1.2.1.6.15.0" },		// tcpOutRsts.0



		/* UDP MIB */ 
		{ "1.3.6.1.2.1.7.1.0" },		// udpInDatagrams.0
		{ "1.3.6.1.2.1.7.3.0" },		// udpInErrors.0
		{ "1.3.6.1.2.1.7.4.0" },		// udpOutDatagrams.0
		

		{ NULL } 
	}; 

/* Initilize */ 
void initialize (void) 
{
	struct oid *op = oids;

	init_snmp("asynchapp");

	while (op->Name) {
		op->OidLen = sizeof(op->Oid)/sizeof(op->Oid[0]);
		if (!read_objid(op->Name, op->Oid, &op->OidLen)){
			snmp_perror("read_objid");
			exit(1);
		}
		op++;
	}
}

char * get_values(int status, struct snmp_session *sp, struct snmp_pdu *pdu )
{
    /* declare array to hold results */
	char *values[20];	//<--- needs to be dynamic
	char buf[1024];
	struct variable_list *vp;
	int ix;
	switch (status) {
	case STAT_SUCCESS:
		vp = pdu->variables;
		if (pdu->errstat == SNMP_ERR_NOERROR) {


	  			
	  		int max_length = 256;
	 		int counter = 0;
	 		int i = 0; 	  
	 		 for(i; i < 20; ++i)
			{
				values[i] = malloc( max_length * sizeof(char) ); 
			}
		
		    /* look through results */
      		while (vp) {
        		snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);

				/* get values only */
				snmp_set_quick_print(1);// < --- changed line
				char * token = strtok(buf, " " );
				token = strtok(NULL, " ");
				//printf("~~~~~~>%s\n<~~~~~~", token);
		
				/* update results array with values */
			 	values[counter] =  token; 
				printf("%s: ~~> %s\n", sp->peername, values[counter]);
				counter++; 		


        	//	fprintf(stdout, "%s: **%s\n", sp->peername, buf);
				vp = vp->next_variable;
     		 }

   		}
    	else {
      		for (ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++);
      		if (vp) snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
      		else strcpy(buf, "(none)");
      		fprintf(stdout, "%s: %s: %s\n",
      		sp->peername, buf, snmp_errstring(pdu->errstat));
   		}
    return *values;
  	/* Error: Timeout */ 
 	case STAT_TIMEOUT:
    	fprintf(stdout, "%s: Timeout\n", sp->peername);
    return 0;

  	/* Error with stat */
 	case STAT_ERROR:
    	snmp_perror(sp->peername);
    return 0;

  	}
  	return 0;
} 




/* print returned data */ 
int print_result( int status, struct snmp_session *sp, struct snmp_pdu *pdu )
{
  char buf[1024];
  struct variable_list *vp;
  int ix;

  /* Times */
  struct timeval now;
  struct timezone tz;
  struct tm *tm;

  gettimeofday(&now, &tz);
  tm = localtime(&now.tv_sec);
  fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec,
          now.tv_usec); 


  switch (status){ 
  case STAT_SUCCESS:
    vp = pdu->variables;
    if (pdu->errstat == SNMP_ERR_NOERROR) {

	  /* declare array to hold results */
	  char *values[20];	//<--- needs to be dynamic
	  int max_length = 256;
	  int counter = 0;
	  int i = 0; 	  
	  for(i; i < 20; ++i)
		{
			values[i] = malloc( max_length * sizeof(char) ); 
		}
		
	  /* look through results */
      while (vp) {
        snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);

		/* get values only */
		snmp_set_quick_print(1);// < --- changed line
		char * token = strtok(buf, " " );
		token = strtok(NULL, " ");
	//	printf("~~~~~~>%s\n<~~~~~~", token);
		
		/* update results array with values */
	 	values[counter] =  token; 
		printf("value: ~~> %s\n", values[counter]);
		counter++; 		

       // fprintf(stdout, "%s: **%s\n", sp->peername, buf);
	vp = vp->next_variable;
      }
/*******
start connect and insert 
******/
	    MYSQL *con = mysql_init(NULL);

	    if (con == NULL) 
  	    {
	  	    fprintf(stderr, "%s\n", mysql_error(con));
		    exit(1);
	    }

	    if (mysql_real_connect(con, "localhost", "root", "password", 
		      "network_database", 0, NULL, 0) == NULL) 
	    {
		    fprintf(stderr, "%s\n", mysql_error(con));
	    }  
	 
	    /* Insert statements 
	    if (mysql_query(con, "INSERT INTO device VALUES(,'Audi',52642)")) {
		    finish_with_error(con);
	    }
			*/ 
		mysql_close(con);	
		
/*******
end insert and clean up 
******/
    }
    else {
      for (ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++);
      if (vp) snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
      else strcpy(buf, "(none)");
      fprintf(stdout, "%s: %s: %s\n",
      	sp->peername, buf, snmp_errstring(pdu->errstat));
    }
    return 1;
  /* Error: Timeout */ 
    case STAT_TIMEOUT:
   	 fprintf( stdout, "%s: Timeout\n", sp->peername );
   	 return 0;

  /* Error with stat */
 	case STAT_ERROR:
    	snmp_perror( sp->peername );
   	 return 0;

  }
  return 0;
} 

void synchronous (void)
{
	struct host *hp;
	
	for ( hp = hosts; hp->name; hp++ ) {
		struct snmp_session ss, *sp;
		struct oid *op;

		/* initialise session */ 
		snmp_sess_init(&ss);
		ss.version = SNMP_VERSION_2c;
		ss.peername = hp->name;
		ss.community = hp->community;
		ss.community_len = strlen(ss.community);
		snmp_synch_setup(&ss);

		if ( ! ( sp = snmp_open(&ss))){
			snmp_perror("snmp_open");
			continue;
		}

		for ( op = oids; op->Name; op++ ) {
			struct snmp_pdu *req, *resp;
			int status;
			req = snmp_pdu_create(SNMP_MSG_GET);
			snmp_add_null_var(req, op->Oid, op->OidLen);
			status = snmp_synch_response(sp, req, &resp);
	/**
			char *vals[20]; = get_values(status, sp, resp);
	  		int i = 0; 	 
			int max_length = 256; 
	  		for(i; i < 20; ++i)
			{
				*vals[i] = malloc( max_length * sizeof(char) ); 
			}


				
			for(i=0; i<20; i++)
			{
				printf("hehehe: %s", vals[i]);
			}
			snmp_free_pdu(resp);
	/***/
			if (!print_result(status, sp, resp)) break;
			snmp_free_pdu(resp);
		}
		snmp_close(sp);
	}
}


/* define a struct session to hold our per host state */
struct session { 
	struct snmp_session *sess; 
	struct oid *current_oid;
	} sessions[sizeof(hosts)/sizeof(hosts[0])];
	
	int active_hosts; 

/* Callback function to fire whenever we get a response */
int asynch_response(int operation, struct snmp_session *sp, int reqid,
						struct snmp_pdu *pdu, void *magic)
{
	struct session* host = (struct session*)magic; 
	struct snmp_pdu *req; 

	/* upon recieving a response, we will print it out and send next req  */
	if ( operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE ) {
		if ( print_result( STAT_SUCCESS, host->sess, pdu)) {
			host->current_oid++;
			if ( host->current_oid->Name) { 

				/* build next request */ 
				req = snmp_pdu_create(SNMP_MSG_GET);
				snmp_add_null_var(req, host->current_oid->Oid, host->current_oid->OidLen);
				if (snmp_send( host->sess, req )) /* everything went well */
					return 1;
				/* Something went wrong.. */				
				else {
					snmp_perror("snmp_send");
					snmp_free_pdu(req);
					}
			}
		}
	}
	else 
		print_result(STAT_TIMEOUT, host->sess, pdu);
		active_hosts--;
		return 1; 
}

/* Code to fireup asynch_respose */ 
void asynchronous(void)
{
	struct session *hs;
	struct host *hp;

	/* for each host, open a session and send out first request */
	for ( hs = sessions, hp = hosts; hp->name; hs++, hp++ ) { 
		struct snmp_pdu *req;
		struct snmp_session sess;
		snmp_sess_init(&sess);
		sess.version = SNMP_VERSION_2c;
		sess.peername = hp->name; 
		sess.community = hp->community;
		sess.community_len = strlen(sess.community);
		
		sess.callback = asynch_response;
		sess.callback_magic = hs; 
		if ( ! (hs->sess = snmp_open(&sess))) {
			snmp_perror("snmp_open");
			continue;
		} 
		hs->current_oid = oids;
		req = snmp_pdu_create(SNMP_MSG_GET);
		snmp_add_null_var(req, hs->current_oid->Oid, hs->current_oid->OidLen);
		if (snmp_send(hs->sess, req))
			active_hosts++;
		else {
			snmp_perror("snmp_send");
			snmp_free_pdu(req);
		}
	}

	/* loop while any active hosts */ 
	while (active_hosts) {
		int fds = 0, block = 1;	
		fd_set fdset;
		struct timeval timeout;
		
		FD_ZERO(&fdset);
		snmp_select_info(&fds, &fdset, &timeout, &block);
		fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
		
		/* Read all sockets and process them */ 
		if (fds ) snmp_read(&fdset); 
		else snmp_timeout();
	}

	/* All active hosts processed, close sessions and finish up */ 
	for (hp = hosts, hs = sessions; hp->name; hs++, hp++){
		if (hs->sess) snmp_close(hs->sess);
	}
}

int main( int argc, char **argv)
{
	initialize();
		
	printf("-------synchronous--------\n");
	synchronous(); 
	
	printf("-------asynchronous--------\n");
	asynchronous();
 



	return 0;

}










		
	













				
	


