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

#include "ConflictResolution.h"
#include "Debug.h"
#include "Generation.h"
#include "ObjectStore.h"
#include "Object.h"
#include "Watch.h"
#include "Timer.h"
void* conflictResolutionThread( void *data)
{
	EventQueue 			*completeGenerationsQueue;
	ConflictSet 		*conflictSet;
	dboid_t				conflictSetDBoid;
	Generation 			*generation;
	MethodCallObject 	*methodCallObject;
	ObjectStore 		*objectStore;
	timeval 			result;
	void 				*object;
	
	completeGenerationsQueue = (EventQueue*) data;
	objectStore = __conf.objectStore;
	
	__DEBUG( "Starting conflict resolution thread" );
	
	while( 1 ) 
	{
		/* Listen for new complete generation from all conflict sets */
		EventQueue_listen( completeGenerationsQueue );
		
		/* Fetch the newest event */
		conflictSetDBoid = EventQueue_pop( completeGenerationsQueue );
		conflictSet = g_hash_table_lookup( __conf.conflictSets, conflictSetDBoid );
		
		__DEBUG( "Got stabilization signal from conflict set with dboid: %s", conflictSet->dboid );
		
		if( !ConflictSet_isEmpty( conflictSet ) )
		{
			/* Fetch the generation that is complete */ 
			generation = ConflictSet_popGeneration( conflictSet );
		
			while( generation != NULL ) 
			{
			
				__DEBUG( "Performing conflict resolution on generation %d", generation->number );
		
				methodCallObject = firstPolicy( generation );
				if( methodCallObject == NULL )
				{
					__ERROR( "Failed to get the stable update from the generation %d", generation->number );
					Generation_free( generation );
					continue;
				}
				
				//__DEBUG( "Add update to object with dboid %s: <%s with param0: %d>", conflictSet->dboid, methodCallObject->methodName, methodCallObject->params[0].paramData.intData );
			
				/* Fetches the object that gets the update */
				ObjectStore_fetch( objectStore, conflictSet->dboid, &object, 0 );
			
				/* 
				 Fetches the pointer to the resolve method for the object 
				*/
				prideMethodPrototype = g_hash_table_lookup( __conf.methodList, methodCallObject->methodName );
			
				/* Perform the actual method */
				prideMethodPrototype( object, methodCallObject->params, methodCallObject->paramSize );
				
				/*
				 * Check the value of the update that have been performed to see if it is the last update
				 * so that the elapsed time can be calculated
				 */
				/*
				if(((Object*)object)->propertyA == watch_getValue() ) {	
					timer_mark( &__stable_end );
					timer_getDiff( &result, &__stable_start, &__stable_end );
					
					__TIME( "Elapsed time: %ld.%06d", result.tv_sec, result.tv_usec);
				}
				*/
				/* Stores the new object with the new conflict resolved values */
				ObjectStore_put( objectStore, conflictSet->dboid, object, ((Object*)object)->size );
			
				/* No need for the generation information */
				Generation_free( generation );
				
				generation = ConflictSet_popGeneration( conflictSet );
			}
		}
	}
	
	return NULL;
}

MethodCallObject* firstPolicy(Generation *generation)
{
	int 				it;
	
	/* Fetches the the update from the first replica that has an update */
	for (it = 0; it < PRIDE_NUM_REPLICAS; it++) {
		if( generation->generationType[it] == GEN_UPDATE )
			return generation->generationData[it].methodCallObject;
	}
	
	return NULL;
}
