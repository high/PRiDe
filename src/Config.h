/*
 * Copyright 2009 Johan Grahn
 *
 * This file is part of the PRiDe project 
 *
 * PRiDe is free software: you can distribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the 
 * Free Software Foundation, version 1. 
 *
 * PRiDe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warrenty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details. 
 *
 * You should have received a copy of the GNU General Public License along with
 * PRiDe. If not, see http://www.gnu.org/licenses/
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdio.h>
#include <glib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>

// Defines the maximum number of package that will be send each time 
#ifndef PROP_PACK_SIZE 
#define PROP_PACK_SIZE 10 
#endif

#ifndef PRIDE_MAX_PACK_SIZE
#define PRIDE_MAX_PACK_SIZE 2048
#endif

#include "ConflictSet.h"
#include "ObjectStore.h"

typedef struct _Config {
	
	int 		id; 		/* Id of the replica */
	int 		writer;		/* Sets to 1 if replica is writer */
	GSList 		*replicas;	/* List of replica definitions */
	FILE		*log; 		/* The logfile for the software */
	int 		lport;		/* The port to listen for packages */
	int			lsocket;	/* The socket that the server listens on */
	pthread_t 	receiver;	/* Thread ID for the receiver */
	pthread_t 	conflictResolutionThreadId;
	fd_set 		master;		/* Structure for all TCP sockets that are connected */
	
	pthread_cond_t listenDoneCondition; /* Condition that happens when a listen socket has been established */
	pthread_mutex_t listenMutex;
	
	/* This is depricated and should not be used */
	ConflictSet conflictSet;
	
	/* This stores all conflict sets with the dboid as key and the conflict set as value */
	GHashTable *conflictSets;
	
	/* The object store for all stable objects */
	ObjectStore *objectStore;
	
	/* 
	 This is a hash table where the key is the method name and the value is the 
	 function pointer to the method 
	*/
	GHashTable *methodList;
	
	/* 
	A list of object IDs and their lock status
	The key is the DBoid that can identify a conflict set 
	The value is a pthread mutex that looks access to conflict set 
	if a transaction is active 
	*/
	GHashTable *transactionLocks;

} Config;

/* Global variable for the configuration */
Config __conf;

#endif 

